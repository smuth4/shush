/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: ifparser.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_IFPARSER_H_)
# define _IFPARSER_H_

void ifparser_init(char *);
int ifparser_lex(void);
int ifparser_parse(void);

extern int ifparser_result;
extern const char *ifparser_errmsg;

#endif
