/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>
#include <fcntl.h>
#if defined(HAVE_PATHS_H)
#include <paths.h>
#else
#define _PATH_TMP "/tmp"
#endif
#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <syslog.h>

#include "run.h"
#include "analyzer.h"
#include "byteset.h"
#include "cf.h"
#include "debug.h"
#include "exec.h"
#include "gethostname.h"
#include "ifparser.h"
#include "mmap.h"
#include "siglist.h"
#include "state.h"
#include "units.h"
#include "variable.h"
#include "version.h"

static char *mylock, *job, *id, *jid;
static time_t start;
static struct passwd *pw;

static void remove_lock(void)
{
    if (mylock != NULL) {
	if (unlink(mylock) != 0)
	    error("Failed to remove lock file (%s): %s", mylock, ERRSTR);
	free(mylock);
	mylock = NULL;
    }
}

static int copyslog, copyout;

static void log(int, char *, ...)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
    __attribute__ ((__format__(__printf__, 2, 3)))
#endif
    ;
static void log(int level, char *format, ...)
{
    va_list va;

    if (copyslog) {
	va_start(va, format);
	vsyslog(level, format, va);
	va_end(va);
    }
    if (copyout) {
	va_start(va, format);
	vfprintf(stdout, format, va);
	va_end(va);
    }
    if (copyout)
	fprintf(stdout, "\n");
}

static void sighandler(int sig)
{
    log(LOG_INFO, "%s: Caught SIG%s (%d)", jid, getsignamebynum(sig), sig);
}

static char *mystrftime(char *format, time_t when)
{
    struct tm *tm;
    static char timestr[256];

    tm = localtime(&when);
    if (strftime(timestr, 256, format, tm) == 0)
	timestr[255] = '\0';
    return timestr;
}

static void myputenv(char *var, char *val)
{
    char *tmp;

    tmp = (char *) malloc(strlen(var) + strlen(val) + 2);
    if (tmp == NULL) {
	error("malloc() failed: %s", ERRSTR);
	exit(1);
    }
    sprintf(tmp, "%s=%s", var, val);
    if (putenv(tmp) != 0) {
	error("putenv(%s) failed: %s", tmp, ERRSTR);
	exit(1);
    }
}

static int mailsubject(FILE * stream, int cf)
{
    char *format;
    int err = 1;

    assert(stream != NULL);
    assert(cf == CF_TIMEOUTSUBJECT || cf == CF_LOCKSUBJECT || cf < 0);

    if (cf < 0) {
	/* Report */
	format = cf_getrptstr(-cf, CF_RPTSUBJECT);
	if (format == NULL)
	    return err;
	if (strchr(format, '%') == NULL) {
	    /* userprefix & hostprefix apply */
	    err = fprintf(stream, "Subject: %s",
			  (cf_getrptnum(-cf, CF_RPTHOST) != 0
			   || cf_getrptnum(-cf,
					   CF_RPTUSER) != 0) ? "[" : "");
	    if (err > 0 && cf_getrptnum(-cf, CF_RPTUSER) != 0)
		err = fprintf(stream, "%s%s",
			      (pw != NULL && pw->pw_name != NULL) ?
			      pw->pw_name : "?",
			      (cf_getrptnum(-cf, CF_RPTHOST) !=
			       0) ? "@" : "");
	    if (err > 0 && cf_getrptnum(-cf, CF_RPTHOST) != 0)
		err = fprintf(stream, "%s",
			      get_hostname(cf_getrptnum(-cf, CF_RPTHOST)));
	    if (err > 0)
		err = fprintf(stream, "%s%s\n",
			      (cf_getrptnum(-cf, CF_RPTHOST) != 0
			       || cf_getrptnum(-cf,
					       CF_RPTUSER) !=
			       0) ? "] " : "", format);
	    return err;
	}
    } else
	format = cf_getstr(cf);

    assert(format != NULL);

    err = fprintf(stream, "Subject: ");
    while (err > 0 && *format != '\0') {
	if (*format != '%')
	    err = fprintf(stream, "%c", *format);
	else {
	    format += 1;
	    switch (*format) {
	    case '%':
		err = fprintf(stream, "%%");
		break;
	    case '\0':
		return err;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		err = fprintf(stream, "%s", get_hostname(*format - 48));
		break;
	    case 'h':
		err = fprintf(stream, "%s", get_hostname(0));
		break;
	    case 'i':
		if (*id != '\0')
		    err = fprintf(stream, "%s", id);
		break;
	    case 'n':
		err = fprintf(stream, "%s", job);
		break;
	    case 'N':
		err = fprintf(stream, "%s", jid);
		break;
	    case 'r':
		if (cf >= 0)
		    break;
		err = fprintf(stream, "%s", cf_getrptstr(-cf, CF_RPTNAME));
		break;
	    case 't':
		err = fprintf(stream, "%s",
			      unit_rtime((u_int) (time(NULL) - start)));
		break;
	    case 'u':
		err = fprintf(stream, "%s",
			      (pw != NULL && pw->pw_name != NULL)
			      ? pw->pw_name : "?");
		break;
	    case 'U':
		err = fprintf(stream, "%u", getuid());
		break;
	    case '-':
		if (*(format + 1) != '\0' && isdigit(*(format + 1))) {
		    err = fprintf(stream, "%s",
				  get_hostname(48 - *(format + 1)));
		    format += 1;
		    break;
		}
		/* FALLTHRU */
	    default:
		err = fprintf(stream, "%c", *format);
		break;
	    }
	}
	format += 1;
    }

    if (err > 0)
	fprintf(stream, "\n");
    return err;
}

static int tails[6];

#if defined(HAVE_PTHREAD)
static void *tail(void *fdarray)
{
    int *fd, nl, ptr, len;
    char buffer[16384];
    time_t stop;

    fd = (int *) fdarray;
    ptr = 0;
    stop = 0;
    while (stop == 0 || (ptr < fd[2] && time(NULL) < stop)) {
	len = pread(fd[0], buffer, 16384, ptr);
	if (len < 0) {
	    error("tail pread() failed: %s", ERRSTR);
	    pthread_exit(NULL);
	}
	nl = len;
	while (nl > 0) {
	    if (buffer[nl - 1] == '\n')
		break;
	    nl -= 1;
	}
	ptr += nl;
	if (nl >= 0 && write(fd[1], buffer, nl) != nl) {
	    error("tail write(%d) failed: %s", fd[1], ERRSTR);
	    pthread_exit(NULL);
	} else
	    sleep(1);
	if (fd[2] >= 0 && stop == 0)
	    stop = time(NULL) + 5;
    }
    if (ptr < fd[2])
	fprintf(stderr,
		"%s: Standard %s truncated (%d bytes out of %d).\n", jid,
		(fd[1] == fileno(stdout)) ? "output" : "error", ptr,
		fd[2]);
    pthread_exit(NULL);
}
#endif

#define SM_TIMEOUT 1
#define SM_LOCK    2
static void send_notification(int type, char *to, pid_t pid, int ptr)
{
    int mail[2], err, smstatus;
    FILE *sm;
    char *smv[4];
    pid_t sendmail;

    assert(type == SM_TIMEOUT || type == SM_LOCK);

    /* Create pipe to sendmail */
    if (pipe(mail) != 0) {
	error("pipe() failed for %s notification to %s: %s",
	      (type == SM_TIMEOUT) ? "timeout" : "lock", to, ERRSTR);
	return;
    }

    sm = fdopen(mail[1], "w");
    if (sm == NULL) {
	error("fdopen() failed for %s notification to %s: %s",
	      (type == SM_TIMEOUT) ? "timeout" : "lock", to, ERRSTR);
	close(mail[0]);
	close(mail[1]);
	return;
    }

    /* Spawn sendmail process */
    smv[0] = cf_getstr(CF_SENDMAIL);
    smv[1] = "-t";
    smv[2] = "-i";
    smv[3] = NULL;
    sendmail = exec(mail[0], 1, 2, 0, smv);
    if (sendmail < 0) {
	error("Failed to send %s notification to %s",
	      (type == SM_TIMEOUT) ? "timeout" : "lock", to);
	fclose(sm);
	return;
    }

    /* Write the mail */
    err = fprintf(sm, "To: %s\n", to);
    if (type == SM_TIMEOUT) {
	if (err > 0)
	    mailsubject(sm, CF_TIMEOUTSUBJECT);
	if (err > 0)
	    err =
		fprintf(sm,
			"\n\"%s\" has timed out while running on host \"%s\".\nProcess %d has been running for %s.\n\nDefined timeout actions are: %s\nPending timeout actions are: %*s\n",
			jid, get_hostname(0), pid,
			unit_rtime((u_int) (time(NULL) - start)),
			cf_getstr(CF_TIMEOUT),
			strlen(cf_getstr(CF_TIMEOUT)),
			cf_getstr(CF_TIMEOUT) + ptr);
    } else if (type == SM_LOCK) {
	if (err > 0)
	    mailsubject(sm, CF_LOCKSUBJECT);
	if (err > 0)
	    err =
		fprintf(sm,
			"\n\"%s\" has been trying to obtain its lock for %s while running on host \"%s\" (process id %d).\n\nDefined lock actions are: %s\nPending lock actions are: %*s\n",
			jid, unit_rtime((u_int) (time(NULL) - start)),
			get_hostname(0), pid, cf_getstr(CF_LOCK),
			strlen(cf_getstr(CF_LOCK)),
			cf_getstr(CF_LOCK) + ptr);
    }

    if (err > 0)
	err = fflush(sm);
    if (err == -1)
	error("Error while writing mail for %s notification to \"%s\": %s",
	      (type == SM_TIMEOUT) ? "timeout" : "lock", to, ERRSTR);
    if (fclose(sm) != 0 && err > 0)
	error("fclose([sendmail for %s notification]) failed: %s",
	      (type == SM_TIMEOUT) ? "timeout" : "lock", ERRSTR);

    /*
     ** This will probably always return 0 and should be left for later
     ** but this requires more work than i am ready to do right now.
     ** The main benefit would be to avoid zombie processes.
     */
    err = waitpid(sendmail, &smstatus, WNOHANG);
    if (err == -1)
	error("waitpid(%d[sendmail for %s notification]) failed: %s",
	      sendmail, (type == SM_TIMEOUT) ? "timeout" : "lock", ERRSTR);

    if (err == sendmail) {
	/* Check termination type */
	if (WIFEXITED(smstatus) && WEXITSTATUS(smstatus) != 0)
	    error("sendmail for %s notification returned: %d",
		  (type == SM_TIMEOUT) ? "timeout" : "lock",
		  WEXITSTATUS(smstatus));
	if (WIFSIGNALED(smstatus))
	    error("sendmail for %s notification died: %s",
		  (type == SM_TIMEOUT) ? "timeout" : "lock",
		  strsignal(WTERMSIG(smstatus)));
    }
}

void
run(char *cfdir, char *myjob, char *myid, char *slfac, int options,
    char *envp[])
{
    struct sigaction sa;
    char fname[PATH_MAX], outlog[PATH_MAX], errlog[PATH_MAX], *outstr,
	*errstr;
    char *action, *tmpstr, *last, *loop;
    char *argv[4];
    int outfd, errfd, err, status, rpt;
#if defined(HAVE_PTHREAD)
    pthread_t outthd, errthd;
#endif
    size_t outlen, errlen;
    time_t delay, end;
    char *utimestr, *stimestr;
    pid_t child;
    struct rusage ru;

    assert(cfdir != NULL);
    assert(myjob != NULL);
    assert(myid != NULL);
    assert(slfac != NULL);

    pw = getpwuid(getuid());
    /* Setup syslog logging */
    copyout = (options & RUN_VERBOSE) != 0;
    job = myjob;
    id = myid;
    jid = (char *) malloc(strlen(job) + strlen(myid) + 2);
    if (jid == NULL) {
	error("malloc() failed: %s", ERRSTR);
	exit(1);
    }
    if (myid[0] != '\0')
	sprintf(jid, "%s[%s]", job, myid);
    else
	strcpy(jid, job);
    if (slfac[0] != '\0') {
	openlog(myname, LOG_PID, syslog_facility(slfac));
	copyslog = 1;
	if (ttyname(fileno(stdin)) != NULL) {
	    log(LOG_INFO, "Running \"%s\" as %s [%u] on %s (version %s)",
		jid, (pw != NULL
		      && pw->pw_name != NULL) ? pw->pw_name : "?",
		getuid(), ttyname(fileno(stdin)), SHUSH_VERSION);
	    variable_set(V_TTY, 1);
	} else {
	    log(LOG_INFO, "Running \"%s\" as %s [%u] (version %s)",
		jid, (pw != NULL
		      && pw->pw_name != NULL) ? pw->pw_name : "?",
		getuid(), SHUSH_VERSION);
	    variable_set(V_TTY, 0);
	}
    }

    /* Setup SIGPIPE handler */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) != 0)
	error("sigaction(SIGPIPE, SIG_IGN, NULL) failed: %s", ERRSTR);

    /* Load the configuration */
    snprintf(fname, PATH_MAX, "%s/%s", cfdir, job);
    cf_load(fname, myid);
    assert(cf_timestamp != 0);
    log(LOG_INFO, "%s: %s %ld %s", jid,
	mystrftime("%Y/%m/%d %H:%M:%S", cf_timestamp), cf_size, cf_md5);
    if ((cf_getstr(CF_SYSLOG) == NULL && strcasecmp(slfac, SYSLOG) != 0)
	|| (cf_getstr(CF_SYSLOG) != NULL
	    && strcasecmp(slfac, cf_getstr(CF_SYSLOG)) != 0))
	error("Current and configuration syslog setting mismatch");
    snprintf(outlog, PATH_MAX, "%s.stdout", cf_getstr(CF_CONFIG));
    snprintf(errlog, PATH_MAX, "%s.stderr", cf_getstr(CF_CONFIG));
    if (analyzer_init(outlog, errlog) == -1)
	exit(1);
    if (out_timestamp != 0)
	log(LOG_INFO, "%s: stdout %s %ld %s", jid,
	    mystrftime("%Y/%m/%d %H:%M:%S", out_timestamp), out_size,
	    out_md5);
    if (err_timestamp != 0)
	log(LOG_INFO, "%s: stderr %s %ld %s", jid,
	    mystrftime("%Y/%m/%d %H:%M:%S", err_timestamp), err_size,
	    err_md5);

    /* Successful startup, initialize state recording */
    state_init(cfdir, job);

    /* Randomly delay start */
    if (cf_getnum(CF_RANDOMDELAY) > 0) {
	if ((options & RUN_FAST) == 0) {
	    srandom(getpid());
	    delay = random() % cf_getnum(CF_RANDOMDELAY);
	    debug(DINFO, "Sleeping %lu (< %lu) seconds before starting...",
		  delay, cf_getnum(CF_RANDOMDELAY));
	    state_delay(delay);
	    log(LOG_INFO, "%s: Delaying start by %s", jid,
		unit_rtime(delay));

	    /* Catch typical user generated signals to allow clean abort */
	    sigemptyset(&sa.sa_mask);
	    sa.sa_flags = 0;
	    sa.sa_handler = sighandler;
	    if (sigaction(SIGHUP, &sa, NULL) != 0)
		error("sigaction(SIGHUP, handler, NULL) failed: %s",
		      ERRSTR);
	    if (sigaction(SIGINT, &sa, NULL) != 0)
		error("sigaction(SIGINT, handler, NULL) failed: %s",
		      ERRSTR);
	    if (sigaction(SIGQUIT, &sa, NULL) != 0)
		error("sigaction(SIGQUIT, handler, NULL) failed: %s",
		      ERRSTR);
	    if (sigaction(SIGTERM, &sa, NULL) != 0)
		error("sigaction(SIGTERM, handler, NULL) failed: %s",
		      ERRSTR);

	    if (sleep(delay) != 0) {
		debug(DINFO, "Sleep interrupted, aborting.");
		log(LOG_INFO, "%s: Sleep interrupted, aborting", jid);
		state_close("Aborted: sleep interrupted");
		exit(0);
	    }
	} else {
	    debug(DINFO, "\"randomdelay\" option disabled by user");
	    log(LOG_INFO, "%s: \"randomdelay\" option disabled by user",
		jid);
	}
    }

    /* Catch typical user generated signals so they can be ignored */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sighandler;
    if (sigaction(SIGHUP, &sa, NULL) != 0)
	error("sigaction(SIGHUP, SA_RESTART, NULL) failed: %s", ERRSTR);
    if (sigaction(SIGINT, &sa, NULL) != 0)
	error("sigaction(SIGINT, SA_RESTART, NULL) failed: %s", ERRSTR);
    if (sigaction(SIGQUIT, &sa, NULL) != 0)
	error("sigaction(SIGQUIT, SA_RESTART, NULL) failed: %s", ERRSTR);
    if (sigaction(SIGTERM, &sa, NULL) != 0)
	error("sigaction(SIGTERM, SA_RESTART, NULL) failed: %s", ERRSTR);

    /* Optionally obtain lock */
    tmpstr = cf_getstr(CF_LOCK);
    if (tmpstr != NULL) {
	int lockfd;

	tmpstr = strdup(tmpstr);
	if (tmpstr == NULL) {
	    error("strdup(%s) failed: %s", cf_getstr(CF_LOCK), ERRSTR);
	    exit(1);
	}
	action = strtok_r(tmpstr, ",", &last);
	loop = NULL;
	if (cf_getstr(CF_LOCKFILE) == NULL)
	    snprintf(fname, PATH_MAX, "%s/.%s-%s", cfdir, jid,
		     get_hostname(0));
	else
	    strcpy(fname, cf_getstr(CF_LOCKFILE));
	start = time(NULL);
	end = 0;
	while ((lockfd =
		open(fname, O_WRONLY | O_CREAT | O_EXCL, 0444)) < 0) {
	    static pid_t lockedby = 0;

	    if (errno != EEXIST) {
		error("Failed to create lock file (%s): %s", fname,
		      ERRSTR);
		state_close("Aborted: fatal lock creation failure");
		exit(1);
	    }
	    /* Lock already exists, check if it's stale. */
	    lockfd = open(fname, O_RDONLY, 0);
	    if (lockfd >= 0) {
		char *locked;
		size_t lockedlen;

		if (mapfile(fname, lockfd, (void **) &locked, &lockedlen)
		    != 0) {
		    state_close("Aborted: fatal lock access failure");
		    exit(1);
		}
		if (lockedlen > 0)
		    locked[lockedlen - 1] = '\0';
		if (lockedlen == 0 || atol(locked) == 0) {
		    static time_t when = 0;

		    if (time(NULL) > when) {
			log(LOG_WARNING,
			    "%s: lock file %s exists but contains no PID.",
			    jid, fname);
			when = time(NULL) + 300;
		    }

		    if (lockedlen > 0)
			if (unmapfile(fname, (void *) locked, lockedlen) !=
			    0) {
			    state_close
				("Aborted: fatal lock access failure");
			    exit(1);
			}
		    close(lockfd);
		} else {
		    if (lockedby == 0)
			log(LOG_INFO, "%s: lock is held by process id %ld",
			    jid, (long) atol(locked));
		    else if (lockedby != atol(locked))
			log(LOG_INFO,
			    "%s: lock is now held by process id %ld (was %ld)",
			    jid, (long) atol(locked), (long) lockedby);
		    lockedby = atol(locked);

		    if (unmapfile(fname, (void *) locked, lockedlen) != 0) {
			state_close("Aborted: fatal lock access failure");
			exit(1);
		    }
		    close(lockfd);

		    if (kill(lockedby, 0) == -1) {
			if (errno == ESRCH) {
			    static time_t when = 0;

			    log(LOG_WARNING,
				"%s: Breaking apparently stale lock (%s) left by process id %ld",
				jid, fname, (long) lockedby);
			    if (unlink(fname) == 0)
				continue;
			    if (time(NULL) > when) {
				error
				    ("Failed to break stale lock (%s): %s",
				     fname, ERRSTR);
				when = time(NULL) + 300;
			    }
			} else {
			    error("kill(%ld, 0) failed: %s",
				  (long) lockedby, ERRSTR);
			    state_close
				("Aborted: fatal lock check failure");
			    exit(1);
			}
		    }
		}
	    }

	    if (time(NULL) > end && action != NULL) {
		/* Time to take the next action */
		if (isdigit((int) action[0])) {
		    end = time(NULL) + unit_time(action);
		    log(LOG_INFO, "%s: Will wait up to %s for lock",
			jid, action);
		} else if (strcasecmp(action, "abort") == 0) {
		    log(LOG_INFO, "%s: Giving up.", jid);
		    state_locked(STATE_LCKFAIL);
		    state_close("");
		    exit(0);
		} else if (strcasecmp(action, "ignore") == 0) {
		    log(LOG_WARNING, "%s: Ignoring lock!", jid);
		    state_locked(STATE_LCKIGN);
		    break;
		} else if (strncmp(action, "notify=", 7) == 0)
		    send_notification(SM_LOCK, action + 7, getpid(),
				      action - tmpstr);
		else if (strcmp(action, "loop") == 0)
		    loop = last;
		else
		    error("Invalid \"lock\" action: %s", action);
		action = strtok_r(NULL, ",", &last);
		if (action == NULL) {
		    if (loop == NULL)
			log(LOG_INFO, "%s: Will keeping waiting for lock",
			    jid);
		    else {
			strcpy(tmpstr, cf_getstr(CF_LOCK));
			last = loop;
			action = strtok_r(NULL, ",", &last);
		    }
		}
	    } else
		sleep(1);
	}
	if (lockfd >= 0) {
	    char mypid[16];

	    state_locked(STATE_LCKOK);
	    sprintf(mypid, "%ld\n", (long) getpid());
	    if (write(lockfd, mypid, strlen(mypid) + 1) !=
		strlen(mypid) + 1) {
		error
		    ("Failed to write process id in new lock file (%s): %s",
		     fname, ERRSTR);
	    } else
		log(LOG_INFO, "%s: Created lock file (%s) after %s",
		    jid, fname, unit_rtime(time(NULL) - start));
	    close(lockfd);
	    mylock = strdup(fname);
	    if (mylock == NULL) {
		error("strdup(%s) failed: %s", fname, ERRSTR);
		if (unlink(fname) != 0)
		    error("Failed to remove lock file: %s", ERRSTR);
		exit(1);
	    }
	    atexit(remove_lock);
	}
    }
    free(tmpstr);

    /* Setup temporary files */
    snprintf(outlog, PATH_MAX, "%s/%s-%s.stdout.XXXXXX",
	     (getenv("TMPDIR") != NULL) ? getenv("TMPDIR") : _PATH_TMP,
	     myname, job);
    outfd = mkstemp(outlog);
    if (outfd < 0) {
	error("Failed to create temporary file: %s", ERRSTR);
	state_close("Aborted: temporary file failure");
	exit(1);
    }
#if defined(HAVE_PTHREAD)
    else if ((options & RUN_TAIL) != 0) {
	tails[0] = outfd;
	tails[1] = fileno(stdout);
	tails[2] = -1;
	status = pthread_create(&outthd, NULL, tail, (void *) &tails[0]);
	if (status != 0)
	    error("Failed to create thread to tail standard output: %s",
		  strerror(status));
    }
#endif
    if (cf_getrptnum(0, CF_RPTSTDERR) != CF_STDERR_MIXED) {
	snprintf(errlog, PATH_MAX, "%s/%s-%s.stderr.XXXXXX",
		 (getenv("TMPDIR") != NULL) ? getenv("TMPDIR") : _PATH_TMP,
		 myname, job);
	errfd = mkstemp(errlog);
	if (errfd < 0) {
	    error("Failed to create temporary file: %s", ERRSTR);
	    state_close("Aborted: temporary file failure");
	    exit(1);
	}
#if defined(HAVE_PTHREAD)
	else if ((options & RUN_TAIL) != 0) {
	    tails[3] = errfd;
	    tails[4] = fileno(stderr);
	    tails[5] = -1;
	    status =
		pthread_create(&errthd, NULL, tail, (void *) &tails[3]);
	    if (status != 0)
		error("Failed to create thread to tail standard error: %s",
		      strerror(status));
	}
#endif
    } else
	errfd = outfd;

    /* Setup environment */
    if (cf_getstr(CF_PATH) != NULL)
	myputenv("PATH", cf_getstr(CF_PATH));
    myputenv("SHUSH_NAME", job);
    myputenv("SHUSH_ID", myid);

    /* Initialize timeout */
    start = time(NULL);
    tmpstr = cf_getstr(CF_TIMEOUT);
    last = NULL;
    loop = NULL;
    if (tmpstr != NULL) {
	tmpstr = strdup(tmpstr);
	if (tmpstr == NULL) {
	    error("strdup(%s) failed: %s", cf_getstr(CF_TIMEOUT), ERRSTR);
	    exit(1);
	}
	debug(DINFO, "Initialized timeout: %s", tmpstr);
	action = strtok_r(tmpstr, ",", &last);
	end = start + unit_time(action);
    } else
	end = 0;

    /* Spawn child process */
    if (cf_getstr(CF_CMD) == NULL) {
        error("Failed to retrieve a runnable command");
        exit(1);
    }
    argv[0] = cf_getstr(CF_SHELL);
    argv[1] = "-c";
    argv[2] = cf_getstr(CF_CMD);
    argv[3] = NULL;
    child = exec(-1, outfd, errfd, 1, argv);
    if (child < 0) {
	state_close("Aborted: spawn failed");
	exit(1);
    }
    debug(DINFO, "Spawned child process (pid %d), waiting..", child);
    state_run();

    /* Wait for child to complete or timeout */
    while (1) {
	err = wait4(child, &status, WNOHANG, &ru);
	if (err == child && (WIFEXITED(status) || WIFSIGNALED(status)))
	    break;
	if (err == -1) {
	    if (errno == EINTR)
		debug(DINFO, "wait4(%d): %s", child, ERRSTR);
	    else {
		error("wait4(%d) failed: %s", child, ERRSTR);
		state_close("Aborted: fatal wait4() failure");
		exit(1);
	    }
	}
	if (end != 0 && time(NULL) > end) {
	    state_timeout();
	    /* Timeout reached, what now? */
	    debug(DINFO, "Timeout expired, job has been running for %s",
		  unit_rtime(time(NULL) - start));
	    action = strtok_r(NULL, ",", &last);
	    if (action == NULL && loop == NULL)
		end = 0;
	    else {
		if (action == NULL) {
		    strcpy(tmpstr, cf_getstr(CF_TIMEOUT));
		    last = loop;
		    action = strtok_r(NULL, ",", &last);
		}
		if (isdigit((int) action[0])) {
		    end = time(NULL) + unit_time(action);
		    debug(DINFO, "Armed new timeout: %s", action);
		} else if (action[0] == '-' && isdigit((int) action[1])) {
		    debug(DINFO,
			  "Sending signal %d to child process group",
			  atoi(action + 1));
		    state_killed();
		    if (kill(child * -1, atoi(action + 1)) == 0)
			log(LOG_INFO,
			    "%s: Sent signal %d to child process group",
			    jid, atoi(action + 1));
		    else
			log(LOG_WARNING,
			    "%s: Failed to send signal %d to child process group: %s",
			    jid, atoi(action + 1), ERRSTR);

		} else if (strncmp(action, "SIG", 3) == 0) {
		    debug(DINFO,
			  "Sending signal %s to child process group",
			  action);
		    state_killed();
		    if (kill(child * -1, getsignumbyname(action + 3)) == 0)
			log(LOG_INFO,
			    "%s: Sent signal %s to child process group",
			    jid, action);
		    else
			log(LOG_WARNING,
			    "%s: Failed to send signal %s to child process group: %s",
			    jid, action, ERRSTR);
		} else if (action[0] == '=' && isdigit((int) action[1])) {
		    debug(DINFO, "Sending signal %d to child",
			  atoi(action + 1));
		    state_killed();
		    if (kill(child, atoi(action + 1)) == 0)
			log(LOG_INFO, "%s: Sent signal %d to child",
			    jid, atoi(action + 1));
		    else
			log(LOG_WARNING,
			    "%s: Failed to send signal %d to child: %s",
			    jid, atoi(action + 1), ERRSTR);
		} else if (strncmp(action, "=SIG", 4) == 0) {
		    debug(DINFO, "Sending signal %s to child", action);
		    state_killed();
		    if (kill(child, getsignumbyname(action + 4)) == 0)
			log(LOG_INFO, "%s: Sent signal %s to child",
			    jid, action);
		    else
			log(LOG_WARNING,
			    "%s: Failed to send signal %s to child: %s",
			    jid, action, ERRSTR);
		} else if (strncmp(action, "notify=", 7) == 0) {
		    debug(DINFO, "Sending timeout notification to %s",
			  action + 7);
		    log(LOG_INFO, "%s: Sending timeout notification to %s",
			jid, action + 7);

		    send_notification(SM_TIMEOUT, action + 7, child,
				      action - tmpstr);
		} else if (strcmp(action, "loop") == 0)
		    loop = last;
		else
		    error("Invalid \"timeout\" action: %s", action);
	    }
	} else
	    sleep(1);
    }
    end = time(NULL);
    state_close(NULL);
    free(tmpstr);
    /* Child is done */

    remove_lock();
    utimestr = strdup(unit_rtime(ru.ru_utime.tv_sec));
    if (utimestr == NULL) {
	error("strdup(%s) failed: %s", unit_rtime(ru.ru_utime.tv_sec),
	      ERRSTR);
	utimestr = "??";
    }
    stimestr = strdup(unit_rtime(ru.ru_stime.tv_sec));
    if (stimestr == NULL) {
	error("strdup(%s) failed: %s", unit_rtime(ru.ru_stime.tv_sec),
	      ERRSTR);
	stimestr = "??";
    }

    /* Check termination type */
    if (WIFEXITED(status)) {
	debug(DINFO, "Normal termination, returned %d",
	      WEXITSTATUS(status));
	log(LOG_INFO, "%s: Command returned %d after %s (%s user, %s sys)",
	    jid, WEXITSTATUS(status), unit_rtime(end - start), utimestr,
	    stimestr);
	variable_set(V_EXIT, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	debug(DINFO, "Child died from a signal: %s",
	      strsignal(WTERMSIG(status)));
	log(LOG_WARNING,
	    "%s: Command died form a signal (%s) after %s (%s user, %s sys)",
	    jid, strsignal(WTERMSIG(status)), unit_rtime(end - start),
	    utimestr, stimestr);
	variable_set(V_EXIT, -1 * WTERMSIG(status));
    } else
	abort();

    /* Map outputs to memory */
    if (mapfile(outlog, outfd, (void **) &outstr, &outlen) != 0)
	exit(1);
    if (errfd == outfd) {
	errstr = NULL;
	errlen = 0;
    } else if (mapfile(errlog, errfd, (void **) &errstr, &errlen) != 0)
	exit(1);

    /* Put an end to any active tail thread */
    tails[2] = outlen;
    tails[5] = errlen;

    /* Set a few variables */
    variable_set(V_SIZE, outlen + errlen);
    variable_set(V_OUTSIZE, outlen);
    variable_set(V_ERRSIZE, errlen);
    variable_set(V_RUNTIME, end - start);
    variable_set(V_UTIME, ru.ru_utime.tv_sec);
    variable_set(V_STIME, ru.ru_stime.tv_sec);

    /* Analyze outputs */
    if (analyzer_run(outstr, outlen, errstr, errlen) != 0)
	exit(1);
    else {
	long outlines, errlines;

	if (variable_get(V_OUTLINES, &outlines) != 0)
	    abort();
	if (variable_get(V_ERRLINES, &errlines) != 0)
	    abort();
	log(LOG_INFO,
	    "%s: Command produced %ld lines (%ld Bytes) on stdout and %ld lines (%ld Bytes) on stderr",
	    jid, outlines, outlen, errlines, errlen);
    }
    debug(DINFO, "Done with analysis.");

    /* Produce reports */
    rpt = 0;
    while (rpt + 1 < cf_getrptcnt()) {
	int mail[2], smstatus, env;
	FILE *sm;
	char *header;

	rpt += 1;

	/* Check whether report should be sent */
	if (cf_getrptstr(rpt, CF_RPTIF) != NULL) {
	    ifparser_init(cf_getrptstr(rpt, CF_RPTIF));
	    err = ifparser_parse();
	    if (err != 0)
		error("%s while evaluating \"%s\" for report \"%s\"",
		      ifparser_errmsg, cf_getrptstr(rpt, CF_RPTIF),
		      cf_getrptstr(rpt, CF_RPTNAME));
	    if (err == 0 && !ifparser_result) {
		debug(DINFO, "Skipping report \"%s\"",
		      cf_getrptstr(rpt, CF_RPTNAME));
		continue;
	    }
	}
	log(LOG_INFO, "%s: Sending \"%s\" report",
	    jid, cf_getrptstr(rpt, CF_RPTNAME));
	debug(DINFO, "Generating \"%s\" report...",
	      cf_getrptstr(rpt, CF_RPTNAME));

	/* Create pipe to sendmail */
	if (pipe(mail) != 0) {
	    error("pipe() failed for report \"%s\": %s",
		  cf_getrptstr(rpt, CF_RPTNAME), ERRSTR);
	    continue;
	}
	sm = fdopen(mail[1], "w");
	if (sm == NULL) {
	    error("fdopen() failed for report \"%s\": %s",
		  cf_getrptstr(rpt, CF_RPTNAME), ERRSTR);
	    close(mail[0]);
	    close(mail[1]);
	    continue;
	}

	/* Spawn sendmail process */
	argv[0] = cf_getstr(CF_SENDMAIL);
	argv[1] = "-t";
	argv[2] = "-i";
	argv[3] = NULL;
	child = exec(mail[0], 1, 2, 0, argv);
	if (child < 0) {
	    error("Failed to send report \"%s\"",
		  cf_getrptstr(rpt, CF_RPTNAME));
	    fclose(sm);
	    continue;
	}

	/* Fill in headers */
	err = 1;
	if (cf_getrptstr(rpt, CF_RPTTO) != NULL) {
	    err = fprintf(sm, "To: %s\n", cf_getrptstr(rpt, CF_RPTTO));
	    debug(DINFO, "-> To: %s", cf_getrptstr(rpt, CF_RPTTO));
	}
	if (err > 0 && cf_getrptstr(rpt, CF_RPTCC) != NULL) {
	    err = fprintf(sm, "Cc: %s\n", cf_getrptstr(rpt, CF_RPTCC));
	    debug(DINFO, "-> Cc: %s", cf_getrptstr(rpt, CF_RPTCC));
	}
	if (err > 0 && cf_getrptstr(rpt, CF_RPTBCC) != NULL) {
	    err = fprintf(sm, "Bcc: %s\n", cf_getrptstr(rpt, CF_RPTBCC));
	    debug(DINFO, "-> Bcc: %s", cf_getrptstr(rpt, CF_RPTBCC));
	}
	if (err > 0 && cf_getrptstr(rpt, CF_RPTSUBJECT) != NULL) {
	    err = mailsubject(sm, -rpt);
	    debug(DINFO, "-> Subject: %s",
		  cf_getrptstr(rpt, CF_RPTSUBJECT));
	}
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Version: %s\n", SHUSH_VERSION);
	env = 0;
	while (err > 0 && envp[env] != NULL)
	    err = fprintf(sm, "X-Shush-Env: <%s>\n", envp[env++]);
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Host-Name: %s\n", get_hostname(0));
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Name: %s %s %ld %s\n", jid,
			  mystrftime("%Y/%m/%d %H:%M:%S", cf_timestamp),
			  cf_size, cf_md5);
	if (err > 0 && out_timestamp != 0)
	    err = fprintf(sm, "X-Shush-Stdout: %s %ld %s\n",
			  mystrftime("%Y/%m/%d %H:%M:%S", out_timestamp),
			  out_size, out_md5);
	if (err > 0 && err_timestamp != 0)
	    err = fprintf(sm, "X-Shush-Stderr: %s %ld %s\n",
			  mystrftime("%Y/%m/%d %H:%M:%S", err_timestamp),
			  err_size, err_md5);
	if (err > 0)
	    err = fprintf(sm, "X-Shush-User: %s [%u]\n",
			  (pw != NULL
			   && pw->pw_name != NULL) ? pw->pw_name : "?",
			  getuid());
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Report-Name: %s\n",
			  cf_getrptstr(rpt, CF_RPTNAME));
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Command: %s\n", cf_getstr(CF_CMD));
	if (err > 0) {
	    if (WIFEXITED(status))
		err = fprintf(sm, "X-Shush-Exit-Status: %d\n",
			      WEXITSTATUS(status));
	    else {
		assert(WIFSIGNALED(status));
		err = fprintf(sm, "X-Shush-Exit-Status: %s\n",
			      strsignal(WTERMSIG(status)));
	    }
	}
	if (err > 0) {
	    long lines;

	    if (variable_get(V_OUTLINES, &lines) != 0)
		abort();
	    err =
		fprintf(sm, "X-Shush-Stdout-Size: %ld lines, %ld Bytes\n",
			lines, outlen);
	}
	if (err > 0) {
	    long lines;

	    if (variable_get(V_ERRLINES, &lines) != 0)
		abort();
	    err =
		fprintf(sm, "X-Shush-Stderr-Size: %ld lines, %ld Bytes\n",
			lines, errlen);
	}
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Rusage: %s (%s user, %s sys)\n",
			  unit_rtime(end - start), utimestr, stimestr);
	cf_getrptstrlist(-1, CF_RPTHEADER);
	while (err > 0
	       && (header = cf_getrptstrlist(rpt, CF_RPTHEADER)) != NULL)
	    err = fprintf(sm, "%s\n", header);
	if (err > 0 && cf_getrptnum(rpt, CF_RPTFORMAT) == CF_FORMAT_RICH)
	    err = fprintf(sm, "Content-Type: text/enriched\n");
	if (err > 0 && cf_getrptnum(rpt, CF_RPTFORMAT) == CF_FORMAT_HTML)
	    err = fprintf(sm, "Content-Type: text/html\n");
	if (err > 0)
	    err = fprintf(sm, "\n");
	if (err <= 0) {
	    error("Error while writing mail headers for report \"%s\": %s",
		  cf_getrptstr(rpt, CF_RPTNAME), ERRSTR);
	    err = -1;
	} else if (analyzer_output(sm, cf_getrptnum(rpt, CF_RPTFORMAT),
				   cf_getrptnum(rpt, CF_RPTSTDERR),
				   cf_getrptnum(rpt, CF_RPTMAXSZ),
				   outstr, outlen, errstr, errlen) != 0)
	    error("Errors while sending output for report \"%s\".",
		  cf_getrptstr(rpt, CF_RPTNAME));
	if (fclose(sm) != 0 && err == 0)
	    error("Errors while sending report \"%s\": %s",
		  cf_getrptstr(rpt, CF_RPTNAME), ERRSTR);

	/* Wait for sendmail child to complete */
	while (1) {
	    err = waitpid(child, &smstatus, 0);
	    if (err == child
		&& (WIFEXITED(smstatus) || WIFSIGNALED(smstatus)))
		break;
	    if (err == -1) {
		if (errno == EINTR)
		    debug(DINFO, "waitpid(%d): %s", child, ERRSTR);
		else
		    error
			("waitpid(%d[sendmail for report \"%s\"]) failed: %s",
			 child, cf_getrptstr(rpt, CF_RPTNAME), ERRSTR);
	    }
	    sleep(1);
	}
	/* Child is done */

	/* Check termination type */
	if (WIFEXITED(smstatus) && WEXITSTATUS(smstatus) != 0)
	    error("sendmail for report \"%s\" returned: %d",
		  cf_getrptstr(rpt, CF_RPTNAME), WEXITSTATUS(smstatus));
	if (WIFSIGNALED(smstatus))
	    error("sendmail for report \"%s\" died from %s",
		  cf_getrptstr(rpt, CF_RPTNAME),
		  strsignal(WTERMSIG(smstatus)));
    }

    if ((options & RUN_NODEL) == 0) {
	if (unlink(outlog) != 0)
	    error("Failed to remove \"%s\": %s", outlog, ERRSTR);
	if (cf_getrptnum(0, CF_RPTSTDERR) != CF_STDERR_MIXED
	    && unlink(errlog) != 0)
	    error("Failed to remove \"%s\": %s", errlog, ERRSTR);
    } else {
	log(LOG_INFO, "Standard output file: %s", outlog);
	log(LOG_INFO, "Standard error  file: %s", errlog);
    }

#if defined(HAVE_PTHREAD)
    if ((options & RUN_TAIL) != 0) {
	err = pthread_join(outthd, NULL);
	if (err != 0 && err != ESRCH)
	    error("pthread_join() failed unexpectedly: %s", strerror(err));
	err = pthread_join(errthd, NULL);
	if (err != 0 && err != ESRCH)
	    error("pthread_join() failed unexpectedly: %s", strerror(err));
    }
#endif

    log(LOG_INFO, "%s: Done.", jid);
}
