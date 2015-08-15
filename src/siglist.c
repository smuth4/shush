/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#if defined(HAVE_DECL_SYS_SIGNAME)
#include <signal.h>
#endif

#include "siglist.h"
#include "signals.h"

int getsignumbyname(char *name)
{
    int i;

    i = 0;
#if defined(HAVE_SYS_SIGNAME)
    while (i < NSIG) {
	if (strcmp(sys_signame[i], name) == 0)
	    return i;
#else
    while (signame[i].name != NULL) {
	if (strcmp(signame[i].name, name) == 0)
	    return signame[i].num;
#endif
	i += 1;
    }
    return -1;
}

char *getsignamebynum(int num)
{
    int i;

    i = 0;
#if defined(HAVE_SYS_SIGNAME)
    assert(num < NSIG);
    return sys_signame[num];
#else
    while (signame[i].name != NULL) {
	if (signame[i].num == num)
	    return signame[i].name;
	i += 1;
    }
#endif
    return "???";
}
