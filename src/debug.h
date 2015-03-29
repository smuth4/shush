/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: debug.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_DEBUG_H_)
#define _DEBUG_H_

void	debug_init(int);
void	debug(int, char *, ...)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
	__attribute__((__format__(__printf__, 2, 3)))
#endif
	;

#define DINFO	1 /* Useless informative messages */
#define DCF	2 /* Configuration parser */
#define DDATA	3 /* Output data details */
#define DVDATA	4 /* VERBOSE Output data details */

#endif
