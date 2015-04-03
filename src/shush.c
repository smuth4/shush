/*
** Copyright (C) 2003-2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <libgen.h>
#include <time.h>
#if defined(HAVE_PCRE_H)
#include <pcre.h>
#endif

#include "version.h"

#include "debug.h"
#include "check.h"
#include "crontab.h"
#include "run.h"

static char const rcsid[] =
    "@(#)$Id: shush.c 1404 2008-03-08 23:25:46Z kalt $";

extern char *optarg;
extern int optind, opterr;

char *myname;

static void usage(int);

static void usage(detailed)
int detailed;
{
    fprintf(stderr, "Usage: %s [ -h | [ -v ] -V ]\n", myname);
    fprintf(stderr,
	    "Usage: %s [ -c <dir> ] [ -s <facility> | -S ] [ -vfmk ] <name> [ <ID> ]\n",
	    myname);
    fprintf(stderr,
	    "Usage: %s [ -c <dir> ] [ -H <to> ] [ -R <to> ] [ -T <to> ] -C <name> [ <stdout> [ <stderr> ] ]\n",
	    myname);
    fprintf(stderr, "Usage: %s [ -c <dir> ] [ -i | -u | -r ]\n", myname);
    if (detailed == 0)
	exit(0);
    fprintf(stderr, "  -h            Print this message.\n");
    fprintf(stderr, "  -V            Output version info.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -c <dir>      Specify configuration directory.\n");
    fprintf(stderr, "  -d <level>    Enable debugging.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -s <facility> Use specified syslog facility.\n");
#if defined(SYSLOG)
    fprintf(stderr, "  -S            Disable use of syslog.\n");
#endif
    fprintf(stderr,
	    "  -v            Copy syslog messages to the standard output.\n");
    fprintf(stderr,
	    "  -f            Run the command without any (random) delay.\n");
#if defined(HAVE_PTHREAD)
    fprintf(stderr,
	    "  -m            Monitor the command output while it is running.\n");
#endif
    fprintf(stderr,
	    "  -k            Keep the command output log files.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -C            Configuration check.\n");
    fprintf(stderr, "  -H <to>       HTML report recipient(s).\n");
    fprintf(stderr, "  -R <to>       Enriched report recipient(s).\n");
    fprintf(stderr, "  -T <to>       Text report recipient(s).\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -i            Install crontab.\n");
    fprintf(stderr, "  -u            Update crontab.\n");
    fprintf(stderr, "  -r            Remove any entry from crontab.\n");
    exit(1);
}

int main(int argc, char **argv, char *envp[])
{
    int badopt, check, ctab, runopts;
    char cfdir[PATH_MAX + 1], *slfac, *to[3];

    myname = basename(argv[0]);

    check = ctab = runopts = 0;
    cfdir[1] = '\0';
    slfac = SYSLOG;
    to[0] = to[1] = to[2] = NULL;

    badopt = 0;
    while (1) {
	int c;

	c = getopt(argc, argv, "c:Cd:fhH:ikmrR:s:ST:vVu");

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c) {
	case 'c':
	    cfdir[0] = '1';
	    strlcpy(cfdir + 1, optarg, PATH_MAX);
	    break;
	case 'C':
	    check = 1;
	    break;
	case 'd':
	    debug_init(atoi(optarg));
	    break;
	case 'f':
	    runopts |= RUN_FAST;
	    break;
	case 'h':
	    usage(1);
	case 'H':
	    to[0] = optarg;
	    break;
	case 'i':
	    ctab = CRONTAB_SETNEW;
	    break;
	case 'k':
	    runopts |= RUN_NODEL;
	    break;
	case 'm':
#if defined(HAVE_PTHREAD)
	    runopts |= RUN_TAIL;
	    break;
#else
	    fprintf(stderr,
		    "%s: -m option is not supported in this system\n",
		    myname);
	    exit(1);
#endif
	case 'r':
	    ctab = CRONTAB_REMOVE;
	    break;
	case 'R':
	    to[1] = optarg;
	    break;
	case 's':
	    slfac = optarg;
	    break;
	case 'S':
	    slfac = "";
	    break;
	case 'T':
	    to[2] = optarg;
	    break;
	case 'v':
	    runopts |= RUN_VERBOSE;
	    break;
	case 'V':
#if !defined(HAVE_PCRE_H)
	    printf("%s version %s\n", myname, SHUSH_VERSION);
#else
	    printf("%s version %s (PCRE version %s)\n",
		   myname, SHUSH_VERSION, pcre_version());
#endif
	    if ((runopts & RUN_VERBOSE) != 0) {
		printf("\tDefault \"syslog\"   : %s\n", SYSLOG);
		printf("\tDefault \"sizelimit\": %s\n", MAXSZ);
	    }
	    exit(0);
	case 'u':
	    ctab = CRONTAB_UPDATE;
	    break;
	case '?':
	    badopt += 1;
	    break;
	default:
	    abort();
	}
    }

    /* Check that the options given make sense */
    if (((ctab | check) == 0 && (optind == argc	/* One or Two arguments */
				 || optind < argc - 2))
	|| (check != 0 && ((ctab | runopts) != 0	/* One of three modes only! */
			   || optind >= argc	/* Need at least one argument */
			   || optind < argc - 3))	/* And no more than 3 */
	||(ctab != 0 && ((check | runopts) != 0	/* One of three modes only! */
			 || optind != argc))	/* No extra arguments */
	||badopt > 0) {
	usage(0);
	exit(1);
    }

    if (cfdir[1] == '\0') {
	/* -c not explicitely specified, use default: "$HOME/.shush" */
	if (getenv("HOME") == NULL) {
	    fprintf(stderr, "%s: HOME is not set, you must use -c!\n",
		    myname);
	    exit(1);
	}
	cfdir[0] = 0;
	snprintf(cfdir + 1, PATH_MAX, "%s/.shush", getenv("HOME"));
    }

    if ((check | ctab) == 0) {
	/* Let's run! */
	char *job, *id;

	if (to[0] != NULL || to[1] != NULL || to[2] != NULL) {
	    usage(0);
	    exit(1);
	}

	job = argv[optind];
	if (optind == argc - 1)
	    id = "";
	else
	    id = argv[argc - 1];
	error_init(((runopts & RUN_VERBOSE) != 0) ? NULL : job, id,
		   slfac[0] != '\0');
	run(cfdir + 1, job, id, slfac, runopts, envp);
    } else if (check != 0) {
	/* Configuration check mode */
	char *job, *outlog, *errlog;

	job = argv[optind];
	outlog = (optind < argc - 1) ? argv[optind + 1] : NULL;
	errlog = (optind < argc - 2) ? argv[optind + 2] : NULL;
	error_init(NULL, NULL, 0);
	checkrun(cfdir + 1, job, outlog, errlog, to, envp);
    } else if (ctab != 0) {
	/* Crontab update mode */
	if (to[0] != NULL || to[1] != NULL || to[2] != NULL) {
	    usage(0);
	    exit(1);
	}

	error_init(NULL, NULL, 0);
	if (strcasecmp(slfac, SYSLOG) != 0)
	    error("-%c argument ignored.", (slfac != NULL) ? 's' : 'S');
	crontab(argv[0], cfdir, ctab);
    }

    exit(0);
}
