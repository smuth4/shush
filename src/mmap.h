/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: mmap.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_MMAP_H_)
#define _MMAP_H_

#include <sys/types.h>
#include <sys/stat.h>

int mapfile(char *, int, void **, size_t *);
struct stat *mapstat(void);
int unmapfile(char *, void *, size_t);

#endif
