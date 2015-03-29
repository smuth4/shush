/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: analyzer.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_ANALYZER_H_)
# define _ANALYZER_H_

extern time_t out_timestamp, err_timestamp;
extern size_t out_size, err_size;
extern char out_md5[], err_md5[];

int analyzer_init(char *, char *);
int analyzer_run(char *, size_t, char *, size_t);
int analyzer_output(FILE *, int, int, size_t, char *, size_t, char *, size_t);

#endif
