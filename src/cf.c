/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>
#include <sys/types.h>
#if defined(HAVE_MD5_H)
#include <md5.h>
#endif

#include "cf.h"
#include "byteset.h"
#include "gethostname.h"
#include "ifparser.h"
#include "mmap.h"
#include "siglist.h"
#include "units.h"

static char const rcsid[] =
    "@(#)$Id: cf.c 1404 2008-03-08 23:25:46Z kalt $";

#define CFN  0			/* long */
#define CFS  1			/* string */
#define CFSL 2			/* list of strings */

time_t cf_timestamp;
size_t cf_size;
char cf_md5[33];

typedef void *any;
typedef struct list_ list;
struct list_ {
    any value;
    list *nextl;
};

/* Conversion and validation routines */
static void *str2format(char *);
static void *str2lock(char *);
static void *str2size(char *);
static void *str2stderr(char *);
static void *str2sysfac(char *);
static void *str2time(char *);
static void *str2timeout(char *);
static void *str2header(char *);

/* Main configuration options */
struct cfelmt {
    int code;
    char *name;
    char type;
    void *(*valid) (char *);
    any value;
};

static struct cfelmt globalconfig[] = {
    {CF_CONFIG, "config", CFS, NULL, NULL},	/* must be first */
    {CF_CMD, "command", CFS, NULL, NULL},
    {CF_LOCK, "lock", CFS, str2lock, NULL},
    {CF_LOCKFILE, "lockfile", CFS, NULL, NULL},
    {CF_LOCKSUBJECT, "lockmsg", CFS, NULL, NULL},
    {CF_PATH, "path", CFS, NULL, NULL},
    {CF_RANDOMDELAY, "randomdelay", CFN, str2time, NULL},
    {CF_SCHEDULE, "schedule", CFS, NULL, NULL},
    {CF_SENDMAIL, "sendmail", CFS, NULL, NULL},
    {CF_SHELL, "shell", CFS, NULL, NULL},
#if defined(HAVE_PTHREAD)
    {CF_STATEDIR, "statedir", CFS, NULL, NULL},
#endif
    {CF_SYSLOG, "syslog", CFS, str2sysfac, NULL},
    {CF_TIMEOUT, "timeout", CFS, str2timeout, NULL},
    {CF_TIMEOUTSUBJECT, "timeoutmsg", CFS, NULL, NULL},
    {0, NULL, CFS, NULL, NULL}
};

/* Report configuration */
struct rptdef {
    char *name;
    char type;
    void *(*valid) (char *);
};

static const struct rptdef reportconfig[CF_RPT_MAX + 1] = {
    /* order of these MATTERS */
    {"name", CFS, NULL},
    {"to", CFS, NULL},
    {"cc", CFS, NULL},
    {"bcc", CFS, NULL},
    {"subject", CFS, NULL},
    {"userprefix", CFN, NULL},
    {"hostprefix", CFN, NULL},
    {"sizelimit", CFN, str2size},
    {"if", CFS, NULL},
    {"format", CFN, str2format},
    {"stderr", CFN, str2stderr},
    {"header", CFSL, str2header},
    {NULL, CFS, NULL}
};

static any *reports = NULL;
static int reportssz;
static char **idlist = NULL;
static int idlistsz;

/*
** str2format
**	format option validation routine
*/
static void *str2format(char *str)
{
    static long value;

    value = CF_FORMAT_TEXT;
    if (strcasecmp(str, "text") == 0)
	value = CF_FORMAT_TEXT;
    else if (strcasecmp(str, "rich") == 0)
	value = CF_FORMAT_RICH;
    else if (strcasecmp(str, "html") == 0)
	value = CF_FORMAT_HTML;
    else
	error("Invalid \"format\" setting: %s", str);
    return (void *) &value;
}

/*
** str2lock
**	string to lock validation routine
*/
static void *str2lock(char *str)
{
    char *str2, *tmp, *last;
    int endless, silent, loop;

    str2 = strdup(str);
    if (str2 == NULL) {
	error("strdup(%s) failed: %s", str, ERRSTR);
	exit(1);
    }

    endless = 1;
    silent = 1;
    loop = 0;
    tmp = strtok_r(str2, ",", &last);
    while (tmp != NULL) {
	if (isdigit((int) tmp[0])) {
	    if (loop == 1)
		loop = 2;
	    unit_time(tmp);
	} else if (strcasecmp(tmp, "abort") == 0)
	    endless = 0;
	else if (strcasecmp(tmp, "ignore") == 0)
	    endless = 0;
	else if (strncmp(tmp, "notify=", 7) == 0)
	    silent = 0;
	else if (strcmp(tmp, "loop") == 0) {
	    if (loop > 0)
		error("Only one `loop' supported in locking strategy!");
	    silent = 1;
	    loop = 1;
	} else
	    error("Invalid \"lock\" action: %s", tmp);
	tmp = strtok_r(NULL, ",", &last);
    }
    free(str2);

    if (endless == 1 && silent == 1)
	error
	    ("Defined locking strategy (\"%s\") may result in a silent endless wait!",
	     str);
    if (loop == 1) {
	error
	    ("Defined locking strategy (\"%s\") contains an invalid loop!",
	     str);
	exit(1);
    }

    return (void *) str;
}

/*
** str2size
**	string to size conversion routine
*/
static void *str2size(char *str)
{
    static long value;

    value = unit_size(str);
    return (void *) &value;
}

/*
** str2stderr
**	stderr option validation routine
*/
static void *str2stderr(char *str)
{
    static long value;

    value = CF_STDERR_FIRST;
    if (strcasecmp(str, "errfirst") == 0
	|| strcasecmp(str, "first") == 0
	|| strcasecmp(str, "outlast") == 0)
	value = CF_STDERR_FIRST;
    else if (strcasecmp(str, "errlast") == 0
	     || strcasecmp(str, "last") == 0
	     || strcasecmp(str, "outfirst") == 0)
	value = CF_STDERR_LAST;
    else if (strcasecmp(str, "mixed") == 0)
	value = CF_STDERR_MIXED;
    else if (strcasecmp(str, "outonly") == 0)
	value = CF_STDERR_OUT;
    else if (strcasecmp(str, "erronly") == 0)
	value = CF_STDERR_ERR;
    else
	error("Invalid \"stderr\" setting: %s", str);
    return (void *) &value;
}

/*
** str2sysfac
**	syslog option validation routine
*/
static void *str2sysfac(char *str)
{
    if (str[0] != '\0')
	syslog_facility(str);
    return (void *) str;
}

/*
** str2time
**	string to time conversion routine
*/
static void *str2time(char *str)
{
    static long value;

    value = unit_time(str);
    return (void *) &value;
}

/*
** str2timeout
**	string to timeout validation routine
*/
static void *str2timeout(char *str)
{
    char *str2, *tmp, *last;
    int useful, loop;

    str2 = strdup(str);
    if (str2 == NULL) {
	error("strdup(%s) failed: %s", str, ERRSTR);
	exit(1);
    }

    tmp = strtok_r(str2, ",", &last);
    unit_time(tmp);
    useful = 0;
    loop = 0;
    while ((tmp = strtok_r(NULL, ",", &last)) != NULL) {
	if (isdigit((int) tmp[0])) {
	    if (loop == 1)
		loop = 2;
	    unit_time(tmp);
	} else if (tmp[0] == '-' && isdigit((int) tmp[1]))
	    useful = 1;
	else if (strncmp(tmp, "SIG", 3) == 0) {
	    if (getsignumbyname(tmp + 3) < 0)
		error("Invalid \"timeout\" signal: %s", tmp);
	    useful = 1;
	} else if (tmp[0] == '=' && isdigit((int) tmp[1]))
	    useful = 1;
	else if (strncmp(tmp, "=SIG", 4) == 0) {
	    if (getsignumbyname(tmp + 4) < 0)
		error("Invalid \"timeout\" signal: %s", tmp);
	    useful = 1;
	} else if (strncmp(tmp, "notify=", 7) == 0)
	    useful = 1;
	else if (strcmp(tmp, "loop") == 0) {
	    if (loop > 0)
		error("Only one `loop' supported in \"timeout\" action!");
	    loop = 1;
	} else
	    error("Invalid \"timeout\" action: %s", tmp);
    }
    free(str2);

    if (useful == 0)
	error("Defined \"timeout\" action (\"%s\") has no effect.", str);
    if (loop == 1) {
	error
	    ("Defined \"timeout\" action (\"%s\") contains an invalid loop!",
	     str);
	exit(1);
    }

    return (void *) str;
}

/*
** str2header
**	string to header validation routine
*/
static void *str2header(char *str)
{
    char *tmp;

    tmp = str;
    while (*tmp != '\0') {
	if (*tmp == ':')
	    break;
	if (*tmp < 33 || *tmp > 126)
	    break;
	tmp += 1;
    }
    if (*tmp != ':')
	error("Defined \"header\" (\"%s\") is NOT RFC2822 compliant.",
	      str);

    return (void *) str;
}

/*
** myldup
**	Allocate a long and assign it
*/
static void myldup(long **var, long val)
{
    if (*var != NULL)
	free(*var);
    *var = (long *) malloc(sizeof(long));
    if (*var == NULL) {
	error("malloc() failed: %s", ERRSTR);
	exit(1);
    }
    **var = val;
}

/*
** mystrdup
**	Allocate a copy of a string, and assign it
*/
static void mystrdup(char **var, char *str)
{
    if (*var != NULL)
	free(*var);
    *var = strdup(str);
    if (*var == NULL) {
	error("strdup(%s) failed: %s", str, ERRSTR);
	exit(1);
    }
}

/*
** cf_load
**	Parse a configuration file
*/
char *cf_load(char *fname, char *instance)
{
    char *conf, *ln, *lndup, *nl;
    int lncnt, idl, rpt, std;
    size_t cflen;

    assert(reports == NULL);

    cf_timestamp = 0;
    /* Map the configuration file to memory so it can be parsed */
    switch (mapfile(fname, -1, (void **) &conf, &cflen)) {
    case 0:
	/* OK! */
	cf_timestamp = mapstat()->st_mtime;
	if (mapstat()->st_uid != geteuid())
	    error("someone else owns %s!", fname);
	else if ((mapstat()->st_mode & (S_IWGRP | S_IWOTH)) != 0)
	    error("%s must not be group or world writable!", fname);
	else
	    break;
	exit(1);
    case 1:
	error("Configuration file \"%s\" does not exist.", fname);
	/* FALLTHRU */
    default:
	exit(1);
    }

    /* Uhoh, it's empty */
    if (cflen == 0) {
	error("Configuration file (%s) is empty!", fname);
	exit(1);
    }

    cf_size = cflen;
    MD5((unsigned char *) conf, cflen, cf_md5);

    /* Initialize "config" -- Assumes CF_CONFIG is the first array element */
    mystrdup((char **) &globalconfig[0].value, fname);

    /* Initialize the idlist array */
    idlistsz = 5;
    idlist = (char **) malloc(idlistsz * sizeof(char **));
    if (idlist == NULL) {
	error("malloc() failed: %s", ERRSTR);
	exit(1);
    }
    idlist[0] = NULL;
    idl = -1;

    /* Initialize the reports array */
    reportssz = 5;
    reports = (any *) malloc(reportssz * CF_RPT_MAX * sizeof(any));
    if (reports == NULL) {
	error("malloc() failed: %s", ERRSTR);
	exit(1);
    }
    rpt = 0;
    reports[CF_RPTNAME] = NULL;
    mystrdup((char **) &reports[CF_RPTNAME], "<defaults>");
    reports[CF_RPTTO] = reports[CF_RPTCC] = reports[CF_RPTBCC] = NULL;
    reports[CF_RPTSUBJECT] = NULL;
    reports[CF_RPTUSER] = NULL;
    reports[CF_RPTHOST] = NULL;
    myldup((long **) &reports[CF_RPTUSER], 1);
    myldup((long **) &reports[CF_RPTHOST], 255);
    reports[CF_RPTMAXSZ] = NULL;
    if (MAXSZ == NULL)
	myldup((long **) &reports[CF_RPTMAXSZ], -1);
    else
	myldup((long **) &reports[CF_RPTMAXSZ], unit_size(MAXSZ));
    reports[CF_RPTIF] = NULL;
    reports[CF_RPTFORMAT] = NULL;
    myldup((long **) &reports[CF_RPTFORMAT], CF_FORMAT_RICH);
    reports[CF_RPTSTDERR] = NULL;
    myldup((long **) &reports[CF_RPTSTDERR], CF_STDERR_FIRST);
    reports[CF_RPTHEADER] = NULL;

    /* Go crazy, parse the damn thing */
    ln = conf;
    lncnt = 1;
    lndup = NULL;
    do {
	nl = ln;
	while ((nl - conf) < cflen && *nl != '\n' && *nl != '\0')
	    nl += 1;
	if ((nl - conf) == cflen) {
	    if (*(nl - 1) != '\0') {
		assert(lndup == NULL);
		lndup = (char *) malloc(cflen - (ln - conf) + 1);
		if (lndup == NULL) {
		    error("malloc() failed: %s", ERRSTR);
		    exit(1);
		}
		memcpy(lndup, ln, cflen - (ln - conf));
		lndup[cflen - (ln - conf)] = '\0';
		ln = lndup;
	    }
	} else
	    *nl = '\0';

	if (*ln == '#' || *ln == '\0');
	else if (*ln == '[' && ln[strlen(ln) - 1] == ']') {
	    int i;

	    /* New report section */
	    ln[strlen(ln) - 1] = '\0';
	    ln += 1;
	    rpt += CF_RPT_MAX;
	    if (rpt == reportssz * CF_RPT_MAX) {
		/* The array is full already, expand it */
		reportssz *= 2;
		reports = (any *) realloc(reports,
					  reportssz * CF_RPT_MAX *
					  sizeof(any));
		if (reports == NULL) {
		    error("realloc() failed: %s", ERRSTR);
		    exit(1);
		}
	    }
	    i = rpt;
	    while (i < rpt + CF_RPT_MAX)
		reports[i++] = NULL;
	    mystrdup((char **) &reports[rpt + CF_RPTNAME], ln);
	} else {
	    int i;
	    char *val, *at, *pc;

	    /* Separate option name from its value */
	    val = strchr(ln, '=');
	    if (val == NULL) {
		error("Invalid option in %s, line %d: %s", fname, lncnt,
		      ln);
		exit(1);
	    }
	    *val = '\0';
	    val += 1;
	    /* Check for @hostname and %ID components */
	    pc = strchr(ln, '%');
	    if (pc != NULL)
		*pc++ = '\0';
	    at = strchr(ln, '@');
	    if (at != NULL) {
		*at++ = '\0';
		if (strcasecmp(get_hostname(0), at) != 0)
		    /* Wrong host */
		    goto next;
	    }

	    if (pc != NULL) {
		i = 0;
		while (idlist[i] != NULL && strcasecmp(pc, idlist[i]) != 0)
		    i += 1;
		if (idlist[i] == NULL) {
		    idlist[i] = strdup(pc);
		    if (idlist[i] == NULL) {
			error("strdup(%s) failed: %s", pc, ERRSTR);
			exit(1);
		    }
		    if (instance == NULL
			|| strcasecmp(instance, idlist[i]) == 0)
			idl = i;
		    i += 1;
		    if (i == idlistsz) {
			/* The array is full already, expand it */
			idlistsz *= 2;
			idlist = (char **) realloc(idlist,
						   idlistsz *
						   sizeof(char **));
			if (idlist == NULL) {
			    error("realloc() failed: %s", ERRSTR);
			    exit(1);
			}
		    }
		    idlist[i] = NULL;
		}
		if (instance == NULL)
		    instance = idlist[i - 1];
		if (*instance == '\0') {
		    error
			("Configuration requires the use of an instance ID.");
		    exit(1);
		}

		if (strcasecmp(instance, pc) != 0)
		    /* wrong ID */
		    goto next;
	    }

	    i = 0;
	    if (rpt == 0) {
		/* Global configuration options */
		while (globalconfig[i].name != NULL) {
		    if (strcasecmp(globalconfig[i].name, ln) == 0) {
			switch (globalconfig[i].type) {
			case CFN:
			    if (globalconfig[i].valid == NULL)
				myldup((long **) &globalconfig[i].value,
				       atol(val));
			    else
				myldup((long **) &globalconfig[i].value,
				       *(long *)
				       globalconfig[i].valid(val));
			    break;
			case CFS:
			    if (globalconfig[i].valid == NULL)
				mystrdup((char **) &globalconfig[i].value,
					 val);
			    else
				mystrdup((char **) &globalconfig[i].value,
					 (char *)
					 globalconfig[i].valid(val));
			    break;
			default:
			    abort();
			}
			val = NULL;
			break;
		    }
		    i += 1;
		}
	    }

	    i = 0;
	    if (val != NULL) {
		/* Report configuration options, including defaults */
		if (strcasecmp("output", ln) == 0)
		    ln = "stderr";	/* Ugh! */
		while (reportconfig[i].name != NULL) {
		    if (strcasecmp(reportconfig[i].name, ln) == 0) {
			list **alist;

			switch (reportconfig[i].type) {
			case CFN:
			    if (reportconfig[i].valid == NULL)
				myldup((long **) &reports[rpt + i],
				       atol(val));
			    else
				myldup((long **) &reports[rpt + i],
				       *(long *)
				       reportconfig[i].valid(val));
			    break;
			case CFS:
			    if (reportconfig[i].valid == NULL)
				mystrdup((char **) &reports[rpt + i], val);
			    else
				mystrdup((char **) &reports[rpt + i],
					 (char *)
					 reportconfig[i].valid(val));
			    break;
			case CFSL:
			    alist = (list **) & (reports[rpt + i]);
			    while (*alist != NULL)
				alist = &((*alist)->nextl);
			    *alist = malloc(sizeof(list));
			    if (*alist == NULL) {
				error("malloc() failed: %s", ERRSTR);
				exit(1);
			    }
			    (*alist)->nextl = NULL;
			    if (reportconfig[i].valid == NULL)
				(*alist)->value = strdup(val);
			    else
				(*alist)->value =
				    strdup((char *)
					   reportconfig[i].valid(val));
			    if ((*alist)->value == NULL) {
				error("strdup() failed: %s", ERRSTR);
				exit(1);
			    }
			    break;
			default:
			    abort();
			}
			val = NULL;
			break;
		    }
		    i += 1;
		}
	    }

	    if (val != NULL)
		error("Invalid option \"%s\" in %s, line %d", ln, fname,
		      lncnt);
	}

      next:
	ln = nl + 1;
	lncnt += 1;
    }
    while (ln - conf < cflen);
    if (lndup != NULL)
	free(lndup);
    reports[rpt + CF_RPT_MAX] = NULL;	/* Mark the end of the list */

    std = (int) *((long *) reports[CF_RPTSTDERR]);
    if (std == CF_STDERR_OUT || std == CF_STDERR_ERR) {
	error("Invalid default output: \"%s\"!",
	      (std == CF_STDERR_OUT) ? "outonly" : "erronly");
	exit(1);
    }
    while (rpt > 0) {
	assert(reports[rpt + CF_RPTNAME] != NULL);
	if (reports[CF_RPTTO] == NULL && reports[CF_RPTCC] == NULL
	    && reports[CF_RPTBCC] == NULL
	    && reports[rpt + CF_RPTTO] == NULL
	    && reports[rpt + CF_RPTCC] == NULL
	    && reports[rpt + CF_RPTBCC] == NULL) {
	    error("No recipient specified for report \"%s\"!",
		  (char *) reports[rpt + CF_RPTNAME]);
	    exit(1);
	}
	/*
	 ** Could be done before rather than here until ifparser is enhanced
	 ** to support referring to other reports.
	 */
	if (reports[rpt + CF_RPTIF] != NULL) {
	    ifparser_init(reports[rpt + CF_RPTIF]);
	    if (reports[rpt + CF_RPTIF] != NULL && ifparser_parse() != 0) {
		error("%s in if clause for report \"%s\"!",
		      ifparser_errmsg, (char *) reports[rpt + CF_RPTNAME]);
		exit(1);
	    }
	}
	if ((std == CF_STDERR_MIXED
	     && reports[rpt + CF_RPTSTDERR] != NULL
	     && *((long *) reports[rpt + CF_RPTSTDERR]) != std)
	    ||
	    (std != CF_STDERR_MIXED
	     && reports[rpt + CF_RPTSTDERR] != NULL
	     && *((long *) reports[rpt + CF_RPTSTDERR]) ==
	     CF_STDERR_MIXED)) {
	    error("Invalid output option for report \"%s\"!",
		  (char *) reports[rpt + CF_RPTNAME]);
	    exit(1);
	}
	rpt -= CF_RPT_MAX;
    }

    if (idlist[0] == NULL && instance == NULL)
	return "";
    if (instance != NULL && *instance != '\0' && idl == -1) {
	error("Instance ID \"%s\" not found in configuration!", instance);
	exit(1);
    }
    assert(instance != NULL);
    return instance;
}

/*
** cf_nextid
**	Returns the next ID defined/seen in the configuration
*/
char *cf_nextid(char *id)
{
    int i = 0;

    assert(idlist != NULL);

    while (idlist[i] != NULL && strcasecmp(id, idlist[i]) != 0)
	i += 1;

    if (idlist[i] == NULL)
	return NULL;
    else
	return idlist[i + 1];
}

/*
** cf_unload
**	Reverse actions taken by cf_load()
*/
void cf_unload(void)
{
    int i;

    for (i = 0; globalconfig[i].name != NULL; i += 1) {
	if (globalconfig[i].value != NULL) {
	    free(globalconfig[i].value);
	    globalconfig[i].value = NULL;
	}
    }
    if (reports != NULL) {
	for (i = 0; reports[i] != NULL || (i % CF_RPT_MAX) != 0; i += 1)
	    if (reports[i] != NULL)
		free(reports[i]);
	free(reports);
	reports = NULL;
    }
    if (idlist != NULL) {
	for (i = 0; idlist[i] != NULL; i += 1)
	    free(idlist[i]);
	free(idlist);
	idlist = NULL;
    }
}

/*
** cf_getstr
**	Return a string configuration option
*/
char *cf_getstr(int code)
{
    int i;

    assert(code > CF_RPT_MAX && code < CF_MAX);

    i = 0;
    while (globalconfig[i].name != NULL)
	if (globalconfig[i].code == code)
	    break;
	else
	    i += 1;

    assert(i < CF_MAX);
    assert(globalconfig[i].type == CFS);

    /* This is ugly, there should be a generic way of setting defaults. */
    if (code == CF_LOCKSUBJECT && globalconfig[i].value == NULL)
	return "[%u@%h] **PENDING** %N [%t]";
    if (code == CF_SENDMAIL) {
	if (getenv("SHUSH_SENDMAIL") != NULL)
	    return getenv("SHUSH_SENDMAIL");
	else if (globalconfig[i].value == NULL)
	    return SENDMAIL;
    }
    if (code == CF_SHELL && globalconfig[i].value == NULL)
	return "/bin/sh";
    if (code == CF_TIMEOUTSUBJECT && globalconfig[i].value == NULL)
	return "[%u@%h] **TIMEOUT** %N [%t]";

    return (char *) globalconfig[i].value;
}

/*
** cf_getnum
**	Return a number configuration option
*/
long cf_getnum(int code)
{
    int i;

    assert(code > CF_RPT_MAX && code < CF_MAX);

    i = 0;
    while (globalconfig[i].name != NULL)
	if (globalconfig[i].code == code)
	    break;
	else
	    i += 1;

    assert(i < CF_MAX);
    assert(globalconfig[i].type == CFN);

    if (globalconfig[i].value != NULL)
	return *((long *) globalconfig[i].value);
    else
	return -1;
}

/*
** cf_getrptcnt
**	Return the number of defined reports
*/
int cf_getrptcnt(void)
{
    static int i = -1;

    if (i == -1) {
	i = 0;
	while (reports[i * CF_RPT_MAX] != NULL)
	    i += 1;
    } else
	assert(reports[i * CF_RPT_MAX] == NULL);

    return i;
}

/*
** cf_getrptstr
**	Return a report string configuration option
*/
char *cf_getrptstr(int report, int code)
{
    assert(report < cf_getrptcnt());
    assert(code < CF_RPT_MAX && reportconfig[code].type == CFS);

    if (reports[report * CF_RPT_MAX + code] != NULL)
	return (char *) reports[report * CF_RPT_MAX + code];
    else
	return (char *) reports[code];
}


/*
** cf_getrptstrlist
**	Return a report string list configuration option
*/
char *cf_getrptstrlist(int report, int code)
{
    static int which;
    int i;
    list *alist;

    assert(report < cf_getrptcnt());
    assert(code < CF_RPT_MAX && reportconfig[code].type == CFSL);

    if (report < 0) {
	which = 0;
	return NULL;
    }

    if (reports[report * CF_RPT_MAX + code] != NULL)
	alist = (list *) reports[report * CF_RPT_MAX + code];
    else
	alist = (list *) reports[code];

    i = 0;
    while (i++ < which && alist != NULL)
	alist = alist->nextl;
    which += 1;
    if (alist != NULL)
	return (char *) alist->value;
    return NULL;
}

/*
** cf_getrptnum
**	Return a report number configuration option
*/
long cf_getrptnum(int report, int code)
{
    assert(report < cf_getrptcnt());
    assert(code < CF_RPT_MAX && reportconfig &&
	   reportconfig[code].type == CFN);

    if (reports[report * CF_RPT_MAX + code] != NULL)
	return *((long *) reports[report * CF_RPT_MAX + code]);
    else if (reports[code] != NULL)
	return *((long *) reports[code]);
    else
	return -1;
}
