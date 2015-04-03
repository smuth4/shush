/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: crontab.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_CRONTAB_H_)
#define _CRONTAB_H_

#define CRONTAB_SETNEW 1
#define CRONTAB_UPDATE 2
#define CRONTAB_REMOVE 3

void crontab(char *, char *, int);

#endif
