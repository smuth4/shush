/*
** Copyright (C) 2002, 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: units.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_UNITS_H_)
# define _UNITS_H_

u_int unit_time(char *);
char *unit_rtime(u_int);
u_int unit_size(char *);
int syslog_facility(char *);

#endif
