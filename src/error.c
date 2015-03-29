/*
** Copyright (C) 2003-2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <stdarg.h>
#include <syslog.h>

#include "error.h"
#include "gethostname.h"

#ifndef lint
static char const rcsid[] = "@(#)$Id: error.c 1404 2008-03-08 23:25:46Z kalt $";
#endif

static int prefix, tosyslog;
static char *task, *id;

/*
** error_init
**	Whether or not to write a header prior to any error message
*/
void
error_init(char *name, char *jid, int copytosyslog)
{
    task = name;
    id = jid;
    if (name == NULL)
	prefix = 1;
    else
	prefix = 0;
    tosyslog = copytosyslog;
}

/*
** error
**	output a message optionally preceeded by a header
*/
void
error(char *format, ...)
{
    va_list va;

    if (task != NULL)
      {
        if (*id != '\0')
            fprintf(stderr, "%s[%s] encountered the following error(s) while running \"%s\" on host %s:\n\n", myname, task, id, get_hostname(0));
        else
            fprintf(stderr, "%s encountered the following error(s) while running \"%s\" on host %s:\n\n", myname, task, get_hostname(0));
	task = NULL;
      }

    if (prefix)
	fprintf(stderr, "%s: ", myname);
    va_start(va, format);
    vfprintf(stderr, format, va);
    fprintf(stderr, "\n");
    fflush(stderr);

    if (tosyslog)
	vsyslog(LOG_ERR, format, va);

    va_end(va);
}

#undef abort

/*
** myassert
**	_assert() replacement
*/
void
myassert(char *assertion, char *file, char *function, int line)
{
    if (function != NULL)
	error("Assertion failed: %s, file %s, function %s, line %d",
	      assertion, file, function, line);
    else
	error("Assertion failed: %s, file %s, line %d", assertion, file, line);
    abort();
}

/*
** myabort()
**	abort() replacement
*/
void
myabort(char *file, char *function, int line)
{
    if (function != NULL)
	error("The impossible just happened: line %d in function \"%s\" in file %s was reached!", line, function, file);
    else
	error("The impossible just happened: line %d in file %s was reached!",
	      line, file);
    abort();
}
