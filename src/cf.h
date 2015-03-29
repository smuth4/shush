/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: cf.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_CF_H_)
# define _CF_H_

enum
{
    CF_RPTNAME = 0,
    CF_RPTTO,
    CF_RPTCC,
    CF_RPTBCC,
    CF_RPTSUBJECT,
    CF_RPTUSER,
    CF_RPTHOST,
    CF_RPTMAXSZ,
    CF_RPTIF,
    CF_RPTFORMAT,
    CF_RPTSTDERR,
    CF_RPTHEADER,
    CF_RPT_MAX, /* order of the above enums MATTERS */
    CF_CMD,
    CF_CONFIG,
    CF_LOCK,
    CF_LOCKFILE,
    CF_LOCKSUBJECT,
    CF_PATH,
    CF_RANDOMDELAY,
    CF_SCHEDULE,
    CF_SENDMAIL,
    CF_SHELL,
#if defined(HAVE_PTHREAD)
    CF_STATEDIR,
#endif
    CF_SYSLOG,
    CF_TIMEOUT,
    CF_TIMEOUTSUBJECT,
    CF_MAX
};

#define CF_STDERR_MIXED 1
#define CF_STDERR_FIRST 2
#define CF_STDERR_LAST  3
#define CF_STDERR_OUT   4
#define CF_STDERR_ERR   5

#define CF_FORMAT_TEXT 1
#define CF_FORMAT_RICH 2
#define CF_FORMAT_HTML 3

extern time_t cf_timestamp;
extern size_t cf_size;
extern char cf_md5[];

char *cf_load(char *, char *);
char *cf_nextid(char *);
void cf_unload(void);
char *cf_getstr(int);
long cf_getnum(int);
int cf_getrptcnt(void);
char *cf_getrptstr(int, int);
char *cf_getrptstrlist(int, int);
long cf_getrptnum(int, int);

#endif
