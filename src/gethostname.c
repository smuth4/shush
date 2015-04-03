/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <netdb.h>

#include "gethostname.h"

static char const rcsid[] =
    "@(#)$Id: gethostname.c 1404 2008-03-08 23:25:46Z kalt $";

char *get_hostname(int dots)
{
    static char hostname[MAXHOSTNAMELEN + 1];
    char *dot;

    if (gethostname(hostname, MAXHOSTNAMELEN) != 0) {
	error("gethostname() failed: %s", ERRSTR);
	return "";
    }
    hostname[MAXHOSTNAMELEN] = '\0';

    if (dots > 0) {
	dot = hostname;
	while (*dot != '\0' && dots > 0) {
	    if (*dot == '.')
		dots -= 1;
	    dot += 1;
	}
	if (dots == 0)
	    *(dot - 1) = '\0';
    } else if (dots < 0) {
	dot = hostname + strlen(hostname) - 1;
	while (dot != hostname && dots < 0) {
	    if (*dot == '.')
		dots += 1;
	    dot -= 1;
	}
	if (dots == 0)
	    *(dot + 1) = '\0';
    }

    return hostname;
}
