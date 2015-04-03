/*
** Copyright (C) 2003-2007 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: os.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_OS_H_)
#define _OS_H_

/* get some help from autoconf, it's an evil world out there. */

#include "config.h"

/* include directives common to most source files */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/param.h>
#include <limits.h>

#include <string.h>

#if !defined(HAVE_STRLCPY)
#define strlcpy(x, y, z) strcpy(x, y)
#endif

#if defined(HAVE_MD5DATA)
#define MD5(x, y, z) MD5Data(x, y, z);
#else
#if defined(HAVE_MD5_CALC)
#define MD5(x, y, z) md5_calc(z+16, x, y); \
	{ \
	  int i; \
	  for(i = 0 ; i<16 ; i++) \
	  sprintf(z + i*2, "%.2x", (unsigned char) z[i+16]); \
	} \
	z[32] = '\0';
#else
#undef HAVE_MD5_H
#define MD5(x, y, z) z[0] = '\0';
#endif
#endif

#include "error.h"
extern char *myname;

#endif
