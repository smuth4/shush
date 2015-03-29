/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: byteset.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_BYTESET_H_)
# define _BYTESET_H_

void byteset_init(char *, int);
void byteset_set(int, int);
int  byteset_get(int);

#endif
