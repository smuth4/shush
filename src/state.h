/*
** Copyright (C) 2005 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
**
** $Id: state.h 1404 2008-03-08 23:25:46Z kalt $
*/

#if !defined(_STATE_H_)
#define _STATE_H_

void state_init(char *, char *);
void state_delay(time_t);
void state_run(void);
void state_timeout(void);
#define STATE_LCKOK   1
#define STATE_LCKIGN  2
#define STATE_LCKFAIL 3
void state_locked(int);
void state_killed(void);
void state_close(char *msg);

#endif
