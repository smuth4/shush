/*
** Copyright (C) 2003-2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: run.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_RUN_H_)
#define _RUN_H_

#define RUN_FAST	0x01
#define RUN_TAIL	0x02
#define RUN_VERBOSE	0x04
#define RUN_NODEL	0x08

void run(char *, char *, char *, char *, int, char **);

#endif
