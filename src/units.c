/*
** Copyright (C) 2002, 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>
#include <syslog.h>

#include "units.h"

static char const rcsid[] = "@(#)$Id: units.c 1404 2008-03-08 23:25:46Z kalt $";

u_int
unit_time(timestr)
char *timestr;
{ 
    char *unit;

    unit = timestr;
    while (*unit != '\0' && isdigit((int) *unit) != 0)
	unit += 1;
    switch (*unit)
      {
      case 'W':
      case 'w':
	  return 7*24*60*60 * atoi(timestr);
      case 'D':
      case 'd':
	  return 24*60*60 * atoi(timestr);
      case 'H':
      case 'h':
	  return 60*60 * atoi(timestr);
      case '\0':
      case 'M':
      case 'm':
	  return 60 * atoi(timestr);
      case 'S':
      case 's':
	  return atoi(timestr);
      default:
	  error("Invalid time unit: %c", *unit);
	  exit(1);
      }
}

char *
unit_rtime(u_int timeval)
{
    static char timestr[80];
    int width;

    if (timeval == 0)
	return "0s";

    timestr[0] = '\0';
    if (timeval > 7*24*60*60)
      {
	sprintf(timestr + strlen(timestr), "%uw",
		(u_int) (timeval / (7*24*60*60)));
        width = 2;
      }
    else
        width = 1;
    timeval %= 7*24*60*60;
    if (timeval > 24*60*60)
      {
	sprintf(timestr + strlen(timestr), "%ud",
                (u_int) (timeval / (24*60*60)));
        width = 2;
      }
    timeval %= 24*60*60;
    if (timeval > 60*60)
      {
	sprintf(timestr + strlen(timestr), "%.*uh",
                width, (u_int) (timeval / (60*60)));
        width = 2;
      }
    timeval %= 60*60;
    if (timeval > 60)
      {
	sprintf(timestr + strlen(timestr), "%.*um",
                width, (u_int) (timeval / 60));
        width = 2;
      }
    timeval %= 60;
    if (timeval > 0)
	sprintf(timestr + strlen(timestr), "%.*us", width, timeval);

    return timestr;
}

u_int
unit_size(sizestr)
char *sizestr;
{ 
    char *unit;

    unit = sizestr;
    while (*unit != '\0' && isdigit((int) *unit) != 0)
	unit += 1;
    switch (*unit)
      {
      case 'G':
      case 'g':
	  return 1024*1024*1024 * atoi(sizestr);
      case 'M':
      case 'm':
	  return 1024*1024 * atoi(sizestr);
      case '\0':
      case 'K':
      case 'k':
	  return 1024 * atoi(sizestr);
      case 'B':
      case 'b':
      case 'C':
      case 'c':
	  return atoi(sizestr);
      default:
	  error("Invalid size unit: %c", *unit);
	  exit(1);
      }
}

struct codestr
{
    char *name;
    int code;
};

static struct codestr sysfac[] =
{
    {"kern", LOG_KERN},
    {"user", LOG_USER},
    {"mail", LOG_MAIL},
    {"daemon", LOG_DAEMON},
    {"auth", LOG_AUTH},
    {"security", LOG_AUTH},
    {"syslog", LOG_SYSLOG},
    {"lpr", LOG_LPR},
    {"news", LOG_NEWS},
    {"uucp", LOG_UUCP},
    {"cron", LOG_CRON},
    {"local0", LOG_LOCAL0},
    {"local1", LOG_LOCAL1},
    {"local2", LOG_LOCAL2},
    {"local3", LOG_LOCAL3},
    {"local4", LOG_LOCAL4},
    {"local5", LOG_LOCAL5},
    {"local6", LOG_LOCAL6},
    {"local7", LOG_LOCAL7},
    {NULL, -1}
};

int
syslog_facility(char *name)
{
    int i;

    i = 0;
    while (sysfac[i].name != NULL)
      {
	if (strcasecmp(sysfac[i].name, name) == 0)
	    return sysfac[i].code;
	i += 1;
      }
    error("Invalid syslog facility: %s", name);
    exit(1);
}
