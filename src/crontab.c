/*
** Copyright (C) 2003-2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>
#if defined(HAVE_PATHS_H)
# include <paths.h>
#else
# define _PATH_TMP "/tmp"
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>

#include "crontab.h"
#include "cf.h"
#include "exec.h"
#include "gethostname.h"
#include "mmap.h"
#include "version.h"

static char const rcsid[] = "@(#)$Id: crontab.c 1404 2008-03-08 23:25:46Z kalt $";

#define TAG "# DO NOT MODIFY THIS LINE -- shush depends on it!"

static int
getnext(char *cfdname, char **schedule, char **job, char **id)
{
    static DIR *cfdir = NULL;
    static char *calendar = NULL, *last, *lastid;
    struct dirent *entry;
    static int lineno;
    static size_t clen;
    static char cfname[PATH_MAX];

    if (calendar == NULL && cfdir == NULL)
      {
	struct stat sb;

	if (stat(cfdname, &sb) != 0)
	  {
	    error("stat(%s) failed: %s", cfdname, ERRSTR);
	    exit(1);
	  }

	if ((sb.st_mode & (S_IWGRP|S_IWOTH)) != 0)
	  {
	    error("%s must not be group or world writable!", cfdname);
	    exit(1);
	  }

	if (sb.st_uid != geteuid())
	  {
	    error("someone else owns %s!", cfdname);
	    exit(1);
	  }

	snprintf(cfname, PATH_MAX, "%s/schedule", cfdname);
	switch (mapfile(cfname, -1, (void **) &calendar, &clen))
	  {
	  case 0:
	  case 1:
	      break;
	  default:
	      exit(1);
	  }
	lineno = 0;

	if (calendar == NULL)
	  {
	    cfdir = opendir(cfdname);
	    if (cfdir == NULL)
	      {
		error("opendir(%s) failed: %s", cfdname, ERRSTR);
		exit(1);
	      }
	  }
	else
	    printf("%s: Using schedule found in %s.\n", myname, cfdname);

	last = NULL;
        lastid = NULL;
      }

    if (last != NULL)
      {
	*schedule = strtok_r(NULL, ";", &last);
	if (*schedule != NULL)
	    return 0;
      }

    if (lastid != NULL && (lastid = cf_nextid(lastid)) != NULL)
      {
        *id = strdup(lastid);
        if (*id == NULL)
          {
            error("strdup(%s) failed: %s", lastid, ERRSTR);
            exit(1);
          }
        cf_unload();
        lastid = cf_load(cfname, *id);
        free(*id);
        *id = lastid;
        *schedule = cf_getstr(CF_SCHEDULE);
      }
    else if (calendar != NULL)
      {
	static char *ln = NULL, *lndup = NULL;
	char *token, *jid, *nl, *tmp;

	if (ln == NULL)
	    ln = calendar;
	else if (ln - calendar >= clen)
	  {
	    if (lndup != NULL)
		free(lndup);
	    if (unmapfile(cfdname, (void *) calendar, clen) != 0)
		exit(1);
	    return -1;
	  }

	nl = ln;
	while ((nl - calendar) < clen && *nl != '\n' && *nl != '\0')
	    nl += 1;
	if ((nl - calendar) == clen)
	  {
	    if (*(nl - 1) != '\0')
	      {
		assert( lndup == NULL );
		lndup = (char *) malloc(clen - (ln - calendar) + 1);
                if (lndup == NULL)
                  {
                    fprintf(stderr, "%s: malloc() failed: %s", myname, ERRSTR);
                    exit(1);
                  }
		memcpy(lndup, ln, clen - (ln - calendar));
		lndup[clen - (ln - calendar)] = '\0';
		ln = lndup;
	      }
	  }
	else
	    *nl = '\0';

	lineno += 1;

	if (ln[0] == '#' || ln[0] == '\0')
	  {
	    ln = nl + 1;
	    return getnext(cfdname, schedule, job, id);
	  }

	token = strtok_r(ln, " \t", &tmp);
	ln = nl + 1;
	if (strcmp(token, "*") != 0 && strcasecmp(token, get_hostname(0)) != 0)
	    return getnext(cfdname, schedule, job, id);
	token = strtok_r(NULL, " \t", &tmp);
	if (token == NULL)
	  {
	    error("Invalid syntax on line %d in schedule file", lineno);
	    exit(1);
	  }
        jid = strchr(token, ':');
        if (jid == NULL)
            jid = "";
        else
            *jid++ = '\0';

	snprintf(cfname, PATH_MAX, "%s/%s", cfdname, token);

	cf_unload();
	cf_load(cfname, jid);

	*job = token;
        *id = jid;
	*schedule = strtok_r(NULL, "", &tmp);
	if (*schedule == NULL)
	  {
	    error("Invalid syntax on line %d in schedule file", lineno);
	    exit(1);
	  }
	while (**schedule == ' ' || **schedule == '\t')
	    *schedule += 1;
      }
    else if (cfdir != NULL)
      {
        struct stat sb;

	entry = readdir(cfdir);
	if (entry == NULL)
	  {
	    if (closedir(cfdir) != 0)
		error("closedir(%s) failed: %s", cfdname, ERRSTR);
	    return -1;
	  }

	snprintf(cfname, PATH_MAX, "%s/%s", cfdname, entry->d_name);
        if (stat(cfname, &sb) != 0)
          {
            error("stat(%s) failed: %s", cfname, ERRSTR);
            exit(1);
          }

	if (!S_ISREG(sb.st_mode))
	    /* We're only interested in regular files */
	    return getnext(cfdname, schedule, job, id);

	if (entry->d_name[0] == '.' || entry->d_name[0] == '#'
	    || entry->d_name[strlen(entry->d_name) - 1] == '~')
	    /* Typical backup file names */
	    return getnext(cfdname, schedule, job, id);

	if (strlen(entry->d_name) > 7
	    && (strcmp(entry->d_name + strlen(entry->d_name) - 7,
		       ".stdout") == 0 ||
		strcmp(entry->d_name + strlen(entry->d_name) - 7,
		       ".stderr") == 0))
	    /* shush configuration file names */
	    return getnext(cfdname, schedule, job, id);

	cf_unload();
	lastid = cf_load(cfname, NULL);

	*job = entry->d_name;
        *id = lastid;
	*schedule = cf_getstr(CF_SCHEDULE);
      }
    else
	abort();

    if (*schedule == NULL || **schedule == '#' || **schedule == '\0')
	return getnext(cfdname, schedule, job, id);

    *schedule = strtok_r(*schedule, ";", &last);
    return 0;
}

void
crontab(char *mypath, char *cfdir, int action)
{
    char tag[PATH_MAX+80], *oldtab, *mytab, newtab[PATH_MAX];
    char *schedule, *job, *jid;
    char *argv[3];
    size_t oldlen, len;
    int status, newfd;
    FILE *old, *new;
    pid_t child;
    struct stat sb;

    /* What tag should be used? */
    if (cfdir[0] == 0)
	strcpy(tag, TAG);
    else
	sprintf(tag, "%s [%s]", TAG, cfdir+1);
    strcat(tag, "\n");

    /* Setup needed temporary files */
    old = tmpfile();
    if (old == NULL)
      {
	error("Failed to create temporary file: %s", ERRSTR);
	exit(1);
      }
    snprintf(newtab, PATH_MAX, "%s/%s-crontab.XXXXXX",
	     (getenv("TMPDIR") != NULL) ? getenv("TMPDIR") : _PATH_TMP,
	     myname);
    newfd = mkstemp(newtab);
    if (newfd < 0)
      {
	error("Failed to create temporary file: %s", ERRSTR);
	exit(1);
      }
    new = fdopen(newfd, "w");
    if (new == NULL)
      {
	error("Failed to create temporary file: %s", ERRSTR);
	exit(1);
      }

    /* Retrieve current crontab */
    argv[0] = CRONTAB;
    argv[1] = "-l";
    argv[2] = NULL;
    child = exec(-1, fileno(old), 2, 0, argv);
    if (child < 0)
	exit(1);

    if (waitpid(child, &status, 0) == -1)
      {
	error("waitpid(%d) failed: %s", child, ERRSTR);
	exit(1);
      }

    /* Append a final NUL character to keep code below simple */
    if (write(fileno(old), "\0", 1) != 1) {
	error("write() failed: %s", ERRSTR);
	exit(1);
      }

    /*
    ** Complain about failures, unless setting up a new crontab since
    ** crontab(1) will "fail" if there is no crontba.
    */
    if (action != CRONTAB_SETNEW)
      {
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	  {
	    error("\"crontab -l\" failed with error code %d)",
		  WEXITSTATUS(status));
	    exit(1);
	  }
	else if (WIFSIGNALED(status) || WIFSTOPPED(status))
	  {
	    error("\"crontab -l\" terminated or stopped abnormally (%s)",
		  strsignal(WIFSIGNALED(status) ? WTERMSIG(status)
			    : WSTOPSIG(status)));
	    exit(1);
	  }
      }

    if (mapfile("tmpfile", fileno(old), (void **)&oldtab, &oldlen) != 0)
	exit(1);

    /* Anything in there yet? */
    if (oldlen > 1)
	mytab = strstr(oldtab, tag);
    else
	mytab = NULL;

    /* Check whether there is anything to be done. */
    if (action == CRONTAB_SETNEW || action == CRONTAB_UPDATE)
      {
	if (getnext(cfdir+1, &schedule, &job, &jid) != 0)
	  {
	    if (mytab == NULL)
	      {
		printf("%s: Nothing to do.\n", myname);
		exit(0);
	      }
	    else
	      {
		assert( action == CRONTAB_UPDATE );
		action = CRONTAB_REMOVE;
	      }
	  }

	if (action == CRONTAB_SETNEW && oldlen > 1)
	  {
	    error("ERROR: You already have a crontab!");
	    exit(1);
	  }
      }
    else if (action == CRONTAB_REMOVE)
      {
	if (mytab == NULL)
	  {
	    printf("%s: Nothing to do.\n", myname);
	    exit(0);
	  }
      }
    else
	abort();

    /* Copy the beginning of the existing crontab */
    if (mytab != NULL)
	len = mytab - oldtab;
    else
	len = oldlen - 1;
    
    if (fwrite((void *) oldtab, sizeof(char), len, new) != len)
      {
	error("fwrite(%s) failed: %s", newtab, ERRSTR);
	exit(1);
      }

    if (action == CRONTAB_UPDATE || action == CRONTAB_SETNEW)
      {
	/* Install necessary crontab entries */
	int r;
	if (fprintf(new, "%s", tag) < 0)
	  {
	    error("fprintf(%s) failed: %s", newtab, ERRSTR);
	    exit(1);
	  }
	do
	  {
	    char *slfac;

	    r = fprintf(new, "%s %s ", schedule, mypath);
	    if (r > 0 && cfdir[0] != 0)
		r = fprintf(new, "-c \"%s\" ", cfdir+1);
	    slfac = cf_getstr(CF_SYSLOG);
	    if (slfac != NULL)
	      {
		if (r > 0 && strcasecmp(slfac, SYSLOG) != 0)
		  {
		    if (slfac[0] != '\0')
			r = fprintf(new, "-s %s ", slfac);
		    else
			r = fprintf(new, "-S ");
		  }
	      }
	    if (r > 0)
		r = fprintf(new, "\"%s\" %s\n", job, jid);
          }
	while (r > 0 && getnext(cfdir+1, &schedule, &job, &jid) == 0);
	if (r > 0)
	    r = fprintf(new, "%s", tag);
	if (r <= 0)
	  {
	    error("fprintf(%s) failed: %s", newtab, ERRSTR);
	    exit(1);
	  }
      }
			
    /* Copy the end of the existing crontab */
    if (mytab != NULL)
      {
	mytab = strstr(mytab + 1, tag);
	if (mytab == NULL)
	  {
	    error("Current crontab seems to be corrupted.");
	    if (unlink(newtab) != 0)
		error("Failed to remove temporary file \"%s\": %s",
		      newtab, ERRSTR);
	    exit(1);
	  }
	mytab += strlen(tag);
	if (*mytab != '\0'
	    && fwrite((void *) mytab, sizeof(char), strlen(mytab), new)
	    != strlen(mytab))
	  {
	    error("fwrite(%s) failed: %s", newtab, ERRSTR);
	    exit(1);
	  }
      }
    if (fflush(new) != 0)
      {
	error("fflush(%s) failed: %s", newtab, ERRSTR);
	exit(1);
      }

    if (fstat(newfd, &sb) != 0)
      {
	error("fstat(%s) failed: %s", newtab, ERRSTR);
	exit(1);
      }

    /* Install new or remove crontab */
    if (sb.st_size > 0)
	argv[1] = newtab;
    else
	argv[1] = "-r";
    child = exec(fileno(new), 1, 2, 0, argv);
    if (child < 0)
	exit(1);
    
    if (waitpid(child, &status, 0) == -1)
      {
	error("waitpid(%d) failed: %s", child, ERRSTR);
	exit(1);
      }
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
      {
	if (sb.st_size > 0)
	    printf("%s: crontab updated.\n", myname);
	else
	    printf("%s: crontab removed.\n", myname);
	if (unlink(newtab) == 0)
	    exit(0);
	error("Failed to remove temporary file \"%s\": %s", newtab, ERRSTR);
	exit(1);
      }
    
    if (sb.st_size > 0)
	error("Something went wrong while trying to save new crontab!");
    else
	error("Something went wrong while trying to remove crontab!");
    
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	error("\"crontab %s\" returned %d?!", argv[1], WEXITSTATUS(status));
    else if (WIFSIGNALED(status) || WIFSTOPPED(status))
	error("\"crontab %s\" terminated or stopped abnormally.", argv[1]);
    
    exit(1);
}
