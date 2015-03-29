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
# include <paths.h>
#else
# define _PATH_TMP "/tmp"
#endif
#include <pwd.h>
#include <stdarg.h>
#include <sys/wait.h>

#include "check.h"
#include "analyzer.h"
#include "cf.h"
#include "debug.h"
#include "exec.h"
#include "mmap.h"
#include "variable.h"
#include "version.h"

static char const rcsid[] = "@(#)$Id: check.c 1404 2008-03-08 23:25:46Z kalt $";

static char *
mystrftime(char *format, time_t when)
{
    struct tm *tm;
    static char timestr[256];

    tm = localtime(&when);
    if (strftime(timestr, 256, format, tm) == 0)
	timestr[255] = '\0';
    return timestr;
}

void
checkrun(char *cfdir, char *job, char *outlog, char *errlog, char *to[], char *envp[])
{
    char fname[PATH_MAX], outre[PATH_MAX], errre[PATH_MAX], *outstr, *errstr;
    size_t outlen, errlen;
    int rpt;
    struct passwd *pw;

    assert( cfdir != NULL );
    assert( job != NULL );
    assert( outlog != NULL || errlog == NULL );

    pw = getpwuid(getuid());
    /* Load the configuration */
    snprintf(fname, PATH_MAX, "%s/%s", cfdir, job);
    cf_load(fname, "");
    assert( cf_timestamp != 0 );
    printf("%s: %s %ld %s\n", job,
	   mystrftime("%Y/%m/%d %H:%M:%S", cf_timestamp), cf_size, cf_md5);
    snprintf(outre, PATH_MAX, "%s.stdout", cf_getstr(CF_CONFIG));
    snprintf(errre, PATH_MAX, "%s.stderr", cf_getstr(CF_CONFIG));
    if (analyzer_init(outre, errre) == -1)
	exit(1);
    if (out_timestamp != 0)
	printf("%s: stdout %s %ld %s\n", job,
	       mystrftime("%Y/%m/%d %H:%M:%S", out_timestamp),
	       out_size, out_md5);
    if (err_timestamp != 0)
	printf("%s: stderr %s %ld %s\n", job,
	       mystrftime("%Y/%m/%d %H:%M:%S", err_timestamp),
	       err_size, err_md5);

    if (outlog == NULL && errlog == NULL)
	return;

    /* Map outputs to memory */
    if (mapfile(outlog, -1, (void **)&outstr, &outlen) != 0)
	exit(1);
    if (errlog == NULL)
      {
	errstr = NULL;
	errlen = 0;
      }
    else if (mapfile(errlog, -1, (void **)&errstr, &errlen) != 0)
	exit(1);

    /* Analyze outputs */
    if (analyzer_run(outstr, outlen, errstr, errlen) != 0)
	exit(1);
    debug(DINFO, "Done with analysis.");

    /* Produce reports */
    rpt = 0;
    while (rpt < 3)
      {
	int mail[2], smstatus, err, env;
	FILE *sm;
	char *format, *argv[4];
	pid_t child;

	/* Check whether report should be sent */
	if (to[rpt] == NULL)
	  {
	    rpt += 1;
	    continue;
	  }
	format = (rpt == 0) ? "HTML" : (rpt == 1) ? "Enriched" : "Text";
	debug(DINFO, "Generating %s report to \"%s\"...", format, to[rpt]);

	/* Create pipe to sendmail */
	if (pipe(mail) != 0)
	  {
	    error("pipe() failed for %s report: %s", format, ERRSTR);
	    continue;
	  }
	sm = fdopen(mail[1], "w");
	if (sm == NULL)
	  {
	    error("fdopen() failed for %s report: %s", format, ERRSTR);
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
	if (child < 0)
	  {
	    error("Failed to send %s report", format);
	    fclose(sm);
	    continue;
	  }

	/* Fill in headers */
	err = 1;
	err = fprintf(sm, "To: %s\n", to[rpt]);
	if (err > 0)
	    err = fprintf(sm, "Subject: %s report sample for %s\n",
			  format, job);
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Version: %s\n", SHUSH_VERSION);
	env = 0;
	while (err > 0 && envp[env] != NULL)
	    err = fprintf(sm, "X-Shush-Env: <%s>\n", envp[env++]);
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Name: %s %s %ld %s\n", job,
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
			  (pw != NULL && pw->pw_name != NULL) ? pw->pw_name:"?",
			  getuid());
	if (err > 0)
	    err = fprintf(sm, "X-Shush-Command: %s\n", cf_getstr(CF_CMD));
	if (err > 0)
	  {
	    long lines;

	    if (variable_get(V_OUTLINES, &lines) != 0)
		abort();
	    err = fprintf(sm, "X-Shush-Stdout-Size: %ld lines, %ld Bytes\n",
			  lines, outlen);
	  }
	if (err > 0)
	  {
	    long lines;

	    if (variable_get(V_ERRLINES, &lines) != 0)
		abort();
	    err = fprintf(sm, "X-Shush-Stderr-Size: %ld lines, %ld Bytes\n",
			  lines, errlen);
	  }
	if (err > 0 && rpt == 0)
	    err = fprintf(sm, "Content-Type: text/html\n");
	if (err > 0 && rpt == 1)
	    err = fprintf(sm, "Content-Type: text/enriched\n");
	if (err > 0)
	    err = fprintf(sm, "\n");
	if (err <= 0)
	  {
	    error("Error while writing mail headers for %s report: %s",
		  format, ERRSTR);
	    err = -1;
	  }
	else if (analyzer_output(sm,
				 (rpt == 0) ? CF_FORMAT_HTML :
				 (rpt == 1) ? CF_FORMAT_RICH : CF_FORMAT_TEXT,
				 cf_getrptnum(rpt, CF_RPTSTDERR),
				 cf_getrptnum(rpt, CF_RPTMAXSZ),
				 outstr, outlen, errstr, errlen) != 0)
	    error("Errors while sending output for %s report.", format);
	if (fclose(sm) != 0 && err == 0)
	    error("Errors while sending %s report: %s", format, ERRSTR);

	/* Wait for sendmail child to complete */
	while (1)
	  {
	    err = waitpid(child, &smstatus, 0);
	    if (err == child && (WIFEXITED(smstatus) || WIFSIGNALED(smstatus)))
		break;
	    if (err == -1)
	      {
		if (errno == EINTR)
		    debug(DINFO, "waitpid(%d): %s", child, ERRSTR);
		else
		    error("waitpid(%d[sendmail for %s report]) failed: %s",
			  child, format, ERRSTR);
	      }
	    sleep(1);
	  }
	/* Child is done */

	/* Check termination type */
	if (WIFEXITED(smstatus) && WEXITSTATUS(smstatus) != 0)
	    error("sendmail for %s report returned: %d",
		  format, WEXITSTATUS(smstatus));
	if (WIFSIGNALED(smstatus))
	    error("sendmail for %s report died from %s",
		  format, strsignal(WTERMSIG(smstatus)));

	rpt += 1;
      }
}
