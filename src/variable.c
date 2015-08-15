/*
** Copyright (C) 2003, 2004 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>

#include "variable.h"

struct variable {
    char *name;
    long value;
};

static struct variable variables[] = {
    {V_EXIT, 0},
    {V_SIZE, 0},
    {V_OUTSIZE, 0},
    {V_ERRSIZE, 0},
    {V_LINES, 0},
    {V_OUTLINES, 0},
    {V_ERRLINES, 0},
    {V_RUNTIME, 0},
    {V_UTIME, 0},
    {V_STIME, 0},
    {V_TTY, 0},
    {NULL, -1}
};

/*
** variable_set
**	Sets the value of a byte in a particular set
*/
void variable_set(char *name, long value)
{
    int i;

    i = 0;
    while (variables[i].name != NULL) {
	if (strcasecmp(variables[i].name, name) == 0) {
	    variables[i].value = value;
	    return;
	}
	i += 1;
    }

    abort();
}

/*
** variable_get
**	Return the value of a byte in a particular set
*/
int variable_get(char *name, long *value)
{
    int i;

    i = 0;
    while (variables[i].name != NULL) {
	if (strcasecmp(variables[i].name, name) == 0) {
	    *value = variables[i].value;
	    return 0;
	}
	i += 1;
    }

    return -1;
}
