%{
/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include "ifparser.h"
#include "ifparser.tab.h"

static char *inputstr;
static int inputstrcur, inputstrlen;

static int ifparser_input(char *, int);

#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = ifparser_input(b, ms))

%}

%option noyywrap

%%

[ \t]+		; /* Eat white spaces */

-?[0-9]+	{ ifparser_lval.integer = atoi(ifparser_text); return NUMBER; }

&&|\|\|		{ return ifparser_text[0]; }
==		{ return EQ; }
!=		{ return NE; }
\<=		{ return LE; }
>=		{ return GE; }
[!()-+<>]	{ return ifparser_text[0]; }

\$[a-zA-Z]+	{ ifparser_lval.string = ifparser_text + 1; return IVARNAME; }
[^!()-+<>]	{ ifparser_lval.integer = ifparser_text[0]; return UVARNAME; }

%%

void
ifparser_init(char *str)
{
    inputstr = str;
    inputstrcur = 0;
    inputstrlen = strlen(str);
}
    
static int
ifparser_input(char *buf, int max_size)
{
    int count;

    count = inputstrlen - inputstrcur;
    if (count > max_size)
	count = max_size;

    if (count == 0)
	return YY_NULL;

    memcpy((void *)buf, (void *)(inputstr + inputstrcur), count);
    inputstrcur += count;
    return count;
}
