/*
** Copyright (C) 2003 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <libgen.h>
#include "ifparser.tab.h"

extern int ifparser_result;

char *myname;

char *tests[] = {
};

int main(int argc, char **argv)
{
    myname = basename(argv[0]);
    ifparser_init("30+2+4+ 99 && a");
    printf("= %d (%d)\n", ifparser_result, ifparser_parse());
    exit(0);
}
