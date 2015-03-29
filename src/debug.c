/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <stdarg.h>

#include "debug.h"

#ifndef lint
static char const rcsid[] = "@(#)$Id: debug.c 1404 2008-03-08 23:25:46Z kalt $";
#endif

static int debug_level;

/*
** debug_init
**	sets up our logging facility */
void
debug_init(int level)
{
    debug_level = level;
}

/*
** debug
**	output a message if applicable
*/
void
debug(int level, char *format, ...)
{
    va_list va;

    if (level > debug_level)
	return;

    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fprintf(stderr, "\n");
    fflush(stderr);
}
