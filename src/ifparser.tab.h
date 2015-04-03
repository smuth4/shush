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
typedef union {
    long integer;
    char *string;
} YYSTYPE;
extern YYSTYPE ifparser_lval;
