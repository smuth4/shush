/*
** Copyright (C) 2003-2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: error.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_ERROR_H_)
# define _ERROR_H_

#define ERRSTR strerror(errno)

void error_init(char *, char *, int);

void error(char *, ...)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
	__attribute__((__format__(__printf__, 1, 2)))
#endif
	;

void myassert(char *, char *, char *, int)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
	__attribute__((__noreturn__))
#endif
	;
#if !defined(__func__)
# define __func__ NULL
#endif
#if defined(__STDC__)
# define assert(x) if (!(x)) myassert(#x, __FILE__, __func__, __LINE__)
#else
# define assert(x) if (!(x)) myassert("x", __FILE__, __func__, __LINE__)
#endif
void myabort(char *, char *, int)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
	__attribute__((__noreturn__))
#endif
	;
#define abort() myabort(__FILE__, __func__, __LINE__)

#endif
