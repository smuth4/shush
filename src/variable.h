/*
** Copyright (C) 2003, 2004 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: variable.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_VARIABLE_H_)
# define _VARIABLE_H_

#define V_EXIT "exit"
#define V_SIZE "size"
#define V_OUTSIZE "outsize"
#define V_ERRSIZE "errsize"
#define V_LINES "lines"
#define V_OUTLINES "outlines"
#define V_ERRLINES "errlines"
#define V_RUNTIME "runtime"
#define V_UTIME "utime"
#define V_STIME "stime"
#define V_TTY "tty"

void variable_set(char *, long);
int  variable_get(char *, long *);

#endif
