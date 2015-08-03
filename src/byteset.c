/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <ctype.h>

#include "byteset.h"

static long set[256];
static int once = 1;

/*
** byteset_init
**	Initialize set[] from a range definition string
*/
void byteset_init(char *definition, int value)
{
    char *str, *tok, *dash;
    int i, j;

    assert(definition != NULL);

    if (once == 1) {
	once = 0;
	i = 0;
	while (i < 256)
	    set[i++] = 0;
    }

    str = strdup(definition);
    tok = strtok(str, ",");
    while (tok != NULL) {
	if (tok[0] == '-')
	    i = 0;
	else if (isdigit((int) tok[0]) != 0)
	    i = atoi(tok);
	else
	    i = -1;

	j = i;
	dash = index(tok, '-');
	if (dash != NULL) {
	    if (*(dash + 1) != '\0') {
		if (isdigit((int) *(dash + 1)) != 0)
		    j = atoi(dash + 1);
		else
		    j = -1;
	    } else
		j = 255;
	}

	if (i < 0 || i > 255 || j < 0 || j > 255 || j < i) {
	    error("Invalid range: %s", tok);
	    exit(1);
	}

	while (i <= j)
	    set[i++] = value;

	tok = strtok(NULL, ",");
    }
    free(str);
}

/*
** byteset_set
**	Sets the value of a byte in the set
*/
void byteset_set(int byte, int value)
{
    assert(byte >= 0 && byte <= 255);

    set[byte] = value;
}

/*
** byteset_get
**	Return the value of a byte in the set
*/
int byteset_get(int byte)
{
    assert(byte >= 0 && byte <= 255);

    return set[byte];
}
