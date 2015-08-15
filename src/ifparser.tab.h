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
extern YYSTYPE ifparser_lval;
