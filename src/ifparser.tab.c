#include <stdlib.h>
#ifndef lint
#if 0
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#else
#if defined(__NetBSD__) && defined(__IDSTRING)
__IDSTRING(yyrcsid,
	   "$NetBSD: skeleton.c,v 1.23 2002/01/31 21:01:15 tv Exp $");
#endif				/* __NetBSD__ && __IDSTRING */
#endif				/* 0 */
#endif				/* lint */
#ifndef __P
#ifdef __STDC__
#define __P(x) x
#else
#define __P(x) ()
#endif
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define yyparse ifparser_parse
#define yylex ifparser_lex
#define yyerror ifparser_error
#define yychar ifparser_char
#define yyval ifparser_val
#define yylval ifparser_lval
#define yydebug ifparser_debug
#define yynerrs ifparser_nerrs
#define yyerrflag ifparser_errflag
#define yyss ifparser_ss
#define yysslim ifparser_sslim
#define yyssp ifparser_ssp
#define yyvs ifparser_vs
#define yyvsp ifparser_vsp
#define yystacksize ifparser_stacksize
#define yylhs ifparser_lhs
#define yylen ifparser_len
#define yydefred ifparser_defred
#define yydgoto ifparser_dgoto
#define yysindex ifparser_sindex
#define yyrindex ifparser_rindex
#define yygindex ifparser_gindex
#define yytable ifparser_table
#define yycheck ifparser_check
#define yyname ifparser_name
#define yyrule ifparser_rule
#define YYPREFIX "ifparser_"
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

static char const rcsid[] =
    "@(#)$Id: ifparser.tab.c 1404 2008-03-08 23:25:46Z kalt $";

int ifparser_result;
const char *ifparser_errmsg;

#line 22 "ifparser.y"
typedef union {
    long integer;
    char *string;
} YYSTYPE;
#line 79 "ifparser.tab.c"
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
const short ifparser_lhs[] = { -1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
};

const short ifparser_len[] = { 2,
    1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 2, 2, 1, 1, 1,
};

const short ifparser_defred[] = { 0,
    15, 16, 17, 0, 0, 0, 0, 0, 13, 0,
    14, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 12, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 3,
};

const short ifparser_dgoto[] = { 7,
    8,
};

const short ifparser_sindex[] = { -33,
    0, 0, 0, -33, -33, -33, 0, 42, 0, -27,
    0, -33, -33, -33, -33, -33, -33, -33, -33, -33,
    -33, 0, -41, -41, -41, -41, 29, 29, -41, -41,
    0, 0,
};

const short ifparser_rindex[] = { 0,
    0, 0, 0, 0, 0, 0, 0, 6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 5, 9, 13, 3, 15, 17, 21,
    0, 0,
};

const short ifparser_gindex[] = { 0,
    94,
};

#define YYTABLESIZE 304
const short ifparser_table[] = { 6,
    4, 20, 10, 21, 5, 1, 5, 0, 7, 0,
    16, 4, 9, 22, 11, 20, 6, 21, 0, 0,
    8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 18, 0, 19, 0, 0, 0, 4, 0,
    10, 4, 5, 10, 0, 5, 7, 0, 0, 7,
    9, 0, 11, 9, 6, 11, 0, 6, 8, 0,
    4, 8, 4, 0, 5, 0, 5, 0, 7, 0,
    7, 20, 9, 21, 9, 0, 6, 0, 6, 16,
    8, 0, 8, 0, 20, 0, 21, 0, 18, 0,
    19, 0, 0, 0, 0, 0, 17, 9, 10, 11,
    0, 18, 0, 19, 0, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 0, 10, 0, 5, 0,
    0, 0, 7, 0, 0, 0, 9, 0, 11, 0,
    6, 0, 0, 0, 8, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 17, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 0, 0, 0, 0, 3,
    0, 12, 13, 14, 15, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 4, 4, 5, 5, 5, 5, 7, 7, 7,
    7, 9, 9, 9, 9, 6, 6, 6, 6, 8,
    8, 8, 8, 0, 0, 0, 0, 12, 13, 14,
    15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 13, 14, 15,
};

const short ifparser_check[] = { 33,
    0, 43, 0, 45, 0, 0, 40, -1, 0, -1,
    38, 45, 0, 41, 0, 43, 0, 45, -1, -1,
    0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 60, -1, 62, -1, -1, -1, 38, -1,
    38, 41, 38, 41, -1, 41, 38, -1, -1, 41,
    38, -1, 38, 41, 38, 41, -1, 41, 38, -1,
    60, 41, 62, -1, 60, -1, 62, -1, 60, -1,
    62, 43, 60, 45, 62, -1, 60, -1, 62, 38,
    60, -1, 62, -1, 43, -1, 45, -1, 60, -1,
    62, -1, -1, -1, -1, -1, 124, 4, 5, 6,
    -1, 60, -1, 62, -1, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 124, -1, 124, -1, 124, -1,
    -1, -1, 124, -1, -1, -1, 124, -1, 124, -1,
    124, -1, -1, -1, 124, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 124, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 257, 258, -1, -1, -1, -1, 263,
    -1, 259, 260, 261, 262, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 259,
    260, 261, 262, 259, 260, 261, 262, 259, 260, 261,
    262, 259, 260, 261, 262, 259, 260, 261, 262, 259,
    260, 261, 262, -1, -1, -1, -1, 259, 260, 261,
    262, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    259, 260, 261, 262,
};

#define YYFINAL 7
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#define YYMAXTOKEN 266
#if YYDEBUG
const char *const ifparser_name[] = {
    "end-of-file", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    "'!'", 0, 0, 0, 0, "'&'", 0, "'('", "')'", 0, "'+'", 0, "'-'", 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    "'<'", 0, "'>'", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, "'|'", 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, "NUMBER", "UVARNAME", "EQ", "NE", "LE", "GE", "IVARNAME",
	"NOT", "MINUS", "DOLLAR",
};

const char *const ifparser_rule[] = {
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
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
int yyparse __P((void));
#line 67 "ifparser.y"

static int yyerror(const char *msg)
{
    ifparser_errmsg = msg;
    return 0;
}

#line 260 "ifparser.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack __P((void));
static int yygrowstack __P((void))
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
	newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
	return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
	newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    if ((newss = (short *) realloc(yyss, newsize * sizeof *newss)) == NULL)
	return -1;
    yyss = newss;
    yyssp = newss + i;
    if ((newvs =
	 (YYSTYPE *) realloc(yyvs, newsize * sizeof *newvs)) == NULL)
	return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse __P((void))
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != NULL) {
	yyn = *yys;
	if (yyn >= '0' && yyn <= '9')
	    yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack())
	goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

  yyloop:
    if ((yyn = yydefred[yystate]) != 0)
	goto yyreduce;
    if (yychar < 0) {
	if ((yychar = yylex()) < 0)
	    yychar = 0;
#if YYDEBUG
	if (yydebug) {
	    yys = 0;
	    if (yychar <= YYMAXTOKEN)
		yys = yyname[yychar];
	    if (!yys)
		yys = "illegal-symbol";
	    printf("%sdebug: state %d, reading %d (%s)\n",
		   YYPREFIX, yystate, yychar, yys);
	}
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
	yyn <= YYTABLESIZE && yycheck[yyn] == yychar) {
#if YYDEBUG
	if (yydebug)
	    printf("%sdebug: state %d, shifting to state %d\n",
		   YYPREFIX, yystate, yytable[yyn]);
#endif
	if (yyssp >= yysslim && yygrowstack()) {
	    goto yyoverflow;
	}
	*++yyssp = yystate = yytable[yyn];
	*++yyvsp = yylval;
	yychar = (-1);
	if (yyerrflag > 0)
	    --yyerrflag;
	goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
	yyn <= YYTABLESIZE && yycheck[yyn] == yychar) {
	yyn = yytable[yyn];
	goto yyreduce;
    }
    if (yyerrflag)
	goto yyinrecovery;
    goto yynewerror;
  yynewerror:
    yyerror("syntax error");
    goto yyerrlab;
  yyerrlab:
    ++yynerrs;
  yyinrecovery:
    if (yyerrflag < 3) {
	yyerrflag = 3;
	for (;;) {
	    if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
		yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE) {
#if YYDEBUG
		if (yydebug)
		    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
		if (yyssp >= yysslim && yygrowstack()) {
		    goto yyoverflow;
		}
		*++yyssp = yystate = yytable[yyn];
		*++yyvsp = yylval;
		goto yyloop;
	    } else {
#if YYDEBUG
		if (yydebug)
		    printf("%sdebug: error recovery discarding state %d\n",
			   YYPREFIX, *yyssp);
#endif
		if (yyssp <= yyss)
		    goto yyabort;
		--yyssp;
		--yyvsp;
	    }
	}
    } else {
	if (yychar == 0)
	    goto yyabort;
#if YYDEBUG
	if (yydebug) {
	    yys = 0;
	    if (yychar <= YYMAXTOKEN)
		yys = yyname[yychar];
	    if (!yys)
		yys = "illegal-symbol";
	    printf
		("%sdebug: state %d, error recovery discards token %d (%s)\n",
		 YYPREFIX, yystate, yychar, yys);
	}
#endif
	yychar = (-1);
	goto yyloop;
    }
  yyreduce:
#if YYDEBUG
    if (yydebug)
	printf("%sdebug: state %d, reducing by rule %d (%s)\n",
	       YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1 - yym];
    switch (yyn) {
    case 1:
#line 40 "ifparser.y"
	{
	    ifparser_result = yyval.integer;
	}
	break;
    case 2:
#line 41 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer + yyvsp[0].integer;
	}
	break;
    case 3:
#line 42 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer - yyvsp[0].integer;
	}
	break;
    case 4:
#line 43 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer == yyvsp[0].integer;
	}
	break;
    case 5:
#line 44 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer != yyvsp[0].integer;
	}
	break;
    case 6:
#line 45 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer < yyvsp[0].integer;
	}
	break;
    case 7:
#line 46 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer <= yyvsp[0].integer;
	}
	break;
    case 8:
#line 47 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer > yyvsp[0].integer;
	}
	break;
    case 9:
#line 48 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer >= yyvsp[0].integer;
	}
	break;
    case 10:
#line 49 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer && yyvsp[0].integer;
	}
	break;
    case 11:
#line 50 "ifparser.y"
	{
	    yyval.integer = yyvsp[-2].integer || yyvsp[0].integer;
	}
	break;
    case 12:
#line 51 "ifparser.y"
	{
	    yyval.integer = yyvsp[-1].integer;
	}
	break;
    case 13:
#line 52 "ifparser.y"
	{
	    yyval.integer = -yyvsp[0].integer;
	}
	break;
    case 14:
#line 54 "ifparser.y"
	{
	    yyval.integer = !yyvsp[0].integer;
	}
	break;
    case 15:
#line 56 "ifparser.y"
	{
	    yyval.integer = yyvsp[0].integer;
	}
	break;
    case 16:
#line 57 "ifparser.y"
	{
	    yyval.integer = byteset_get(yyvsp[0].integer);
	}
	break;
    case 17:
#line 58 "ifparser.y"
	{
	    if (variable_get(yyvsp[0].string, &yyval.integer) != 0) {
		yyerror("Unknown variable used");
		YYERROR;
	    }
	}
	break;
#line 498 "ifparser.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0) {
#if YYDEBUG
	if (yydebug)
	    printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
	yystate = YYFINAL;
	*++yyssp = YYFINAL;
	*++yyvsp = yyval;
	if (yychar < 0) {
	    if ((yychar = yylex()) < 0)
		yychar = 0;
#if YYDEBUG
	    if (yydebug) {
		yys = 0;
		if (yychar <= YYMAXTOKEN)
		    yys = yyname[yychar];
		if (!yys)
		    yys = "illegal-symbol";
		printf("%sdebug: state %d, reading %d (%s)\n",
		       YYPREFIX, YYFINAL, yychar, yys);
	    }
#endif
	}
	if (yychar == 0)
	    goto yyaccept;
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
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack()) {
	goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
  yyoverflow:
    yyerror("yacc stack overflow");
  yyabort:
    return (1);
  yyaccept:
    return (0);
}
