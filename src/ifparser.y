%{
/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include "ifparser.h"
#include "byteset.h"
#include "variable.h"

static char const rcsid[] = "@(#)$Id: ifparser.y,v 1.5 2003-06-17 01:30:20 kalt Exp $";

int ifparser_result;
const char *ifparser_errmsg;

%}

%union
{
    long	integer;
    char *	string;
}

%token <integer> NUMBER UVARNAME EQ NE LE GE
%token <string>  IVARNAME

%type  <integer> condition expr

%left '&' '|'
%left EQ NE '<' LE '>' GE
%left '+' '-'
%right NOT MINUS DOLLAR

%%

condition: expr			{ ifparser_result = $$; }
expr	:  expr '+' expr	{ $$ = $1 + $3; }
	|  expr '-' expr	{ $$ = $1 - $3; }
	|  expr EQ expr		{ $$ = $1 == $3; }
	|  expr NE expr		{ $$ = $1 != $3; }
	|  expr '<' expr	{ $$ = $1 < $3; }
	|  expr LE expr		{ $$ = $1 <= $3; }
	|  expr '>' expr	{ $$ = $1 > $3; }
	|  expr GE expr		{ $$ = $1 >= $3; }
	|  expr '&' expr	{ $$ = $1 && $3; }
	|  expr '|' expr	{ $$ = $1 || $3; }
	|  '(' expr ')'		{ $$ = $2; }
	|  '-' expr		{ $$ = - $2; }
	   %prec MINUS
	|  '!' expr		{ $$ = ! $2; }
	   %prec NOT
	|  NUMBER		{ $$ = $1; }
	|  UVARNAME		{ $$ = byteset_get($1); }
	|  IVARNAME		{ if (variable_get($1, &$$) != 0)
				    {
				      yyerror("Unknown variable used");
				      YYERROR;
				    }
				}
        ;

%%

static int
yyerror(const char *msg)
{
    ifparser_errmsg = msg;
    return 0;
}
