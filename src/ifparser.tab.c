/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0

#ifndef yyparse
#define yyparse    ifparser_parse
#endif /* yyparse */

#ifndef yylex
#define yylex      ifparser_lex
#endif /* yylex */

#ifndef yyerror
#define yyerror    ifparser_error
#endif /* yyerror */

#ifndef yychar
#define yychar     ifparser_char
#endif /* yychar */

#ifndef yyval
#define yyval      ifparser_val
#endif /* yyval */

#ifndef yylval
#define yylval     ifparser_lval
#endif /* yylval */

#ifndef yydebug
#define yydebug    ifparser_debug
#endif /* yydebug */

#ifndef yynerrs
#define yynerrs    ifparser_nerrs
#endif /* yynerrs */

#ifndef yyerrflag
#define yyerrflag  ifparser_errflag
#endif /* yyerrflag */

#ifndef yylhs
#define yylhs      ifparser_lhs
#endif /* yylhs */

#ifndef yylen
#define yylen      ifparser_len
#endif /* yylen */

#ifndef yydefred
#define yydefred   ifparser_defred
#endif /* yydefred */

#ifndef yydgoto
#define yydgoto    ifparser_dgoto
#endif /* yydgoto */

#ifndef yysindex
#define yysindex   ifparser_sindex
#endif /* yysindex */

#ifndef yyrindex
#define yyrindex   ifparser_rindex
#endif /* yyrindex */

#ifndef yygindex
#define yygindex   ifparser_gindex
#endif /* yygindex */

#ifndef yytable
#define yytable    ifparser_table
#endif /* yytable */

#ifndef yycheck
#define yycheck    ifparser_check
#endif /* yycheck */

#ifndef yyname
#define yyname     ifparser_name
#endif /* yyname */

#ifndef yyrule
#define yyrule     ifparser_rule
#endif /* yyrule */
#define YYPREFIX "ifparser_"

#define YYPURE 0

#line 2 "ifparser.y"
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

int ifparser_result;
const char *ifparser_errmsg;

#line 20 "ifparser.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union
{
    long	integer;
    char *	string;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 130 "ifparser.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define NUMBER 257
#define UVARNAME 258
#define EQ 259
#define NE 260
#define LE 261
#define GE 262
#define IVARNAME 263
#define NOT 264
#define MINUS 265
#define DOLLAR 266
#define YYERRCODE 256
typedef short YYINT;
static const YYINT ifparser_lhs[] = {                    -1,
    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,
};
static const YYINT ifparser_len[] = {                     2,
    1,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    2,    2,    1,    1,    1,
};
static const YYINT ifparser_defred[] = {                  0,
   15,   16,   17,    0,    0,    0,    0,    0,   13,    0,
   14,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   12,    0,    0,    0,    0,    0,    0,    0,    0,
    2,    3,
};
static const YYINT ifparser_dgoto[] = {                   7,
    8,
};
static const YYINT ifparser_sindex[] = {                -33,
    0,    0,    0,  -33,  -33,  -33,    0,   42,    0,  -27,
    0,  -33,  -33,  -33,  -33,  -33,  -33,  -33,  -33,  -33,
  -33,    0,  -41,  -41,  -41,  -41,   29,   29,  -41,  -41,
    0,    0,
};
static const YYINT ifparser_rindex[] = {                  0,
    0,    0,    0,    0,    0,    0,    0,    6,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    1,    5,    9,   13,    3,   15,   17,   21,
    0,    0,
};
static const YYINT ifparser_gindex[] = {                  0,
   94,
};
#define YYTABLESIZE 304
static const YYINT ifparser_table[] = {                   6,
    4,   20,   10,   21,    5,    1,    5,    0,    7,    0,
   16,    4,    9,   22,   11,   20,    6,   21,    0,    0,
    8,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   18,    0,   19,    0,    0,    0,    4,    0,
   10,    4,    5,   10,    0,    5,    7,    0,    0,    7,
    9,    0,   11,    9,    6,   11,    0,    6,    8,    0,
    4,    8,    4,    0,    5,    0,    5,    0,    7,    0,
    7,   20,    9,   21,    9,    0,    6,    0,    6,   16,
    8,    0,    8,    0,   20,    0,   21,    0,   18,    0,
   19,    0,    0,    0,    0,    0,   17,    9,   10,   11,
    0,   18,    0,   19,    0,   23,   24,   25,   26,   27,
   28,   29,   30,   31,   32,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    4,    0,   10,    0,    5,    0,
    0,    0,    7,    0,    0,    0,    9,    0,   11,    0,
    6,    0,    0,    0,    8,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   17,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    1,    2,    0,    0,    0,    0,    3,
    0,   12,   13,   14,   15,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,
    4,    4,    4,    5,    5,    5,    5,    7,    7,    7,
    7,    9,    9,    9,    9,    6,    6,    6,    6,    8,
    8,    8,    8,    0,    0,    0,    0,   12,   13,   14,
   15,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   12,   13,   14,   15,
};
static const YYINT ifparser_check[] = {                  33,
    0,   43,    0,   45,    0,    0,   40,   -1,    0,   -1,
   38,   45,    0,   41,    0,   43,    0,   45,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   60,   -1,   62,   -1,   -1,   -1,   38,   -1,
   38,   41,   38,   41,   -1,   41,   38,   -1,   -1,   41,
   38,   -1,   38,   41,   38,   41,   -1,   41,   38,   -1,
   60,   41,   62,   -1,   60,   -1,   62,   -1,   60,   -1,
   62,   43,   60,   45,   62,   -1,   60,   -1,   62,   38,
   60,   -1,   62,   -1,   43,   -1,   45,   -1,   60,   -1,
   62,   -1,   -1,   -1,   -1,   -1,  124,    4,    5,    6,
   -1,   60,   -1,   62,   -1,   12,   13,   14,   15,   16,
   17,   18,   19,   20,   21,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  124,   -1,  124,   -1,  124,   -1,
   -1,   -1,  124,   -1,   -1,   -1,  124,   -1,  124,   -1,
  124,   -1,   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  257,  258,   -1,   -1,   -1,   -1,  263,
   -1,  259,  260,  261,  262,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  259,
  260,  261,  262,  259,  260,  261,  262,  259,  260,  261,
  262,  259,  260,  261,  262,  259,  260,  261,  262,  259,
  260,  261,  262,   -1,   -1,   -1,   -1,  259,  260,  261,
  262,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  259,  260,  261,  262,
};
#define YYFINAL 7
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#define YYMAXTOKEN 266
#define YYUNDFTOKEN 270
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const ifparser_name[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,"'&'",0,"'('","')'",0,"'+'",0,"'-'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'<'",0,"'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'|'",0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"NUMBER","UVARNAME","EQ","NE","LE","GE","IVARNAME","NOT","MINUS","DOLLAR",
0,0,0,"illegal-symbol",
};
static const char *const ifparser_rule[] = {
"$accept : condition",
"condition : expr",
"expr : expr '+' expr",
"expr : expr '-' expr",
"expr : expr EQ expr",
"expr : expr NE expr",
"expr : expr '<' expr",
"expr : expr LE expr",
"expr : expr '>' expr",
"expr : expr GE expr",
"expr : expr '&' expr",
"expr : expr '|' expr",
"expr : '(' expr ')'",
"expr : '-' expr",
"expr : '!' expr",
"expr : NUMBER",
"expr : UVARNAME",
"expr : IVARNAME",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 65 "ifparser.y"

static int
yyerror(const char *msg)
{
    ifparser_errmsg = msg;
    return 0;
}
#line 358 "ifparser.tab.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 38 "ifparser.y"
	{ ifparser_result = yyval.integer; }
break;
case 2:
#line 39 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer + yystack.l_mark[0].integer; }
break;
case 3:
#line 40 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer - yystack.l_mark[0].integer; }
break;
case 4:
#line 41 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer == yystack.l_mark[0].integer; }
break;
case 5:
#line 42 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer != yystack.l_mark[0].integer; }
break;
case 6:
#line 43 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer < yystack.l_mark[0].integer; }
break;
case 7:
#line 44 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer <= yystack.l_mark[0].integer; }
break;
case 8:
#line 45 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer > yystack.l_mark[0].integer; }
break;
case 9:
#line 46 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer >= yystack.l_mark[0].integer; }
break;
case 10:
#line 47 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer && yystack.l_mark[0].integer; }
break;
case 11:
#line 48 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-2].integer || yystack.l_mark[0].integer; }
break;
case 12:
#line 49 "ifparser.y"
	{ yyval.integer = yystack.l_mark[-1].integer; }
break;
case 13:
#line 50 "ifparser.y"
	{ yyval.integer = - yystack.l_mark[0].integer; }
break;
case 14:
#line 52 "ifparser.y"
	{ yyval.integer = ! yystack.l_mark[0].integer; }
break;
case 15:
#line 54 "ifparser.y"
	{ yyval.integer = yystack.l_mark[0].integer; }
break;
case 16:
#line 55 "ifparser.y"
	{ yyval.integer = byteset_get(yystack.l_mark[0].integer); }
break;
case 17:
#line 56 "ifparser.y"
	{ if (variable_get(yystack.l_mark[0].string, &yyval.integer) != 0)
				    {
				      yyerror("Unknown variable used");
				      YYERROR;
				    }
				}
break;
#line 633 "ifparser.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                yys = yyname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
