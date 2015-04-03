/*
** Copyright (C) 2003-2008 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <sys/types.h>
#if defined(HAVE_MD5_H)
#include <md5.h>
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#if defined(HAVE_PCRE_H)
#include <pcre.h>
#endif

#include "analyzer.h"
#include "byteset.h"
#include "cf.h"
#include "debug.h"
#include "mmap.h"
#include "variable.h"

static char const rcsid[] =
    "@(#)$Id: analyzer.c 1404 2008-03-08 23:25:46Z kalt $";

time_t out_timestamp, err_timestamp;
size_t out_size, err_size;
char out_md5[33], err_md5[33];

struct condition {
    int code;
    char *expression;
    union {
	regex_t re;
#if defined(HAVE_PCRE_H)
	pcre *pcre;
#endif
    } val;
};

struct marker {
    size_t offset;
    char type;
};

#define NONE	0
#define LT	0x01		/* Lower Than character may need to be escaped */
#define GT	0x02		/* Grater Than character may need to be escaped */
#define AMP	0x04		/* Lower Than character may need to be escaped */
#define TOGGLE  0x10		/* Toggle rendering */
#define NL      0x40		/* End of line */

#define RE 1
#define PCRE 2

static int outtype, errtype;
static struct condition *out, *err;
static struct marker *outmarks, *errmarks;

/*
** compile_re
**	Regular expression (regex_t) initialization
*/
static int compile_re(void *reptr, char *str)
{
    regex_t *re;
    int errcode;

    re = reptr;
    errcode = regcomp(re, str, REG_EXTENDED);
    if (errcode != 0) {
	char buf[1024];

	if (regerror(errcode, re, buf, 1024) != 0)
	    error("Bad regular expression: %s", buf);
	else
	    error
		("Failed to compile regular expression and to obtain error code: %s",
		 str);
	return -1;
    }
    return 0;
}

#if defined(HAVE_PCRE_H)
/*
** compile_pcre
**	Perl Compatible Regular Expression (pcre) initialization
*/
static int compile_pcre(void *pcreptr, char *str)
{
    pcre **re;
    const char *errmsg;
    int erroffset;

    re = pcreptr;
    *re = pcre_compile(str, 0, &errmsg, &erroffset, NULL);
    if (*re == NULL) {
	error("Bad PCRE (offset %d): %s", erroffset, errmsg);
	return -1;
    }
    return 0;
}
#endif

/*
** loadfile
**	Read a list of one line conditions from a file
*/
static int loadfile(int *type, char *file, struct condition **list)
{
    int lineno, cond, max;
    size_t len;
    char *str, *ln, *lndup, *nl;

    assert(*list == NULL);

    *type = RE;

    /* Allocate a list of conditions */
    max = 10;
    *list = (struct condition *) malloc(max * sizeof(struct condition));
    if (*list == NULL) {
	error("malloc() failed: %s", ERRSTR);
	return -1;
    }
    cond = 0;
    (*list)[cond].code = -1;

    err_timestamp = 0;
    /* Map the configuration file to memory so it can be parsed */
    switch (mapfile(file, -1, (void **) &str, &len)) {
    case 0:
	/* OK! */
	err_timestamp = mapstat()->st_mtime;
	break;
    case 1:
	/* ENOENT */
	return 0;
    default:
	/* Failure */
	return -1;
    }

    if (len == 0)
	return 0;

    err_size = len;
    MD5((unsigned char *) str, len, err_md5);

    ln = str;
    nl = str;
    lineno = 1;
    lndup = NULL;
    while (ln - str < len) {
	/* Parse the file line by line */
	while (nl - str < len && *nl != '\n')
	    nl += 1;

	if ((nl - str) == len) {
	    if (*(nl - 1) != '\0') {
		assert(lndup == NULL);
		lndup = (char *) malloc(len - (ln - str) + 1);
		if (lndup == NULL) {
		    error("malloc() failed: %s", ERRSTR);
		    return -1;
		}
		memcpy(lndup, ln, len - (ln - str));
		lndup[len - (ln - str)] = '\0';
		ln = lndup;
	    }
	} else
	    *nl = '\0';

	if (*ln == '#') {
	    /* Parse configuration directives */
	    if (strcasecmp(ln, "#pcre") == 0) {
		if (cond != 0) {
		    error
			("\"#pcre\" directive must be specified before any condition.");
		    if (lndup != NULL)
			free(lndup);
		    return -1;
		} else {
#if defined(HAVE_PCRE_H)
		    *type = PCRE;
#else
		    error("This binary was built without PCRE support.");
		    if (lndup != NULL)
			free(lndup);
		    return -1;
#endif
		}
	    }
	} else {
	    /* Make sure the list structure is big enough */
	    if (cond == max - 2) {
		max *= 2;
		*list = (struct condition *) realloc(*list,
						     max *
						     sizeof(struct
							    condition));
		if (*list == NULL) {	/* leakage */
		    error("realloc() failed: %s", ERRSTR);
		    if (lndup != NULL)
			free(lndup);
		    return -1;
		}
	    }

	    /* Finally, compile the expression */
	    if (*ln != '\0') {
		(*list)[cond].code = *ln;
		(*list)[cond].expression = strdup(ln + 1);
		if ((*list)[cond].expression == NULL) {
		    error("strdup() failed: %s", ERRSTR);
		    if (lndup != NULL)
			free(lndup);
		    return -1;
		}
		if (*type == RE) {
		    if (compile_re
			((void *) &((*list)[cond].val.re), ln + 1) != 0) {
			if (lndup != NULL)
			    free(lndup);
			return -1;
		    }
		} else if (*type == PCRE) {
#if defined(HAVE_PCRE_H)
		    if (compile_pcre
			((void *) &((*list)[cond].val.pcre),
			 ln + 1) != 0) {
			if (lndup != NULL)
			    free(lndup);
			return -1;
		    }
#else
		    error("This binary was built without PCRE support.");
		    if (lndup != NULL)
			free(lndup);
		    return -1;
#endif
		} else
		    abort();
		cond += 1;
	    }
	}
	ln = nl + 1;
	lineno += 1;
    }

    if (lndup != NULL)
	free(lndup);

    (*list)[cond].code = -1;	/* Mark the end of the list */

    if (unmapfile(file, str, len) != 0)
	return -1;

    return 0;
}

/*
** analyzer_init
**	Initialization for the analyzer.
*/
int analyzer_init(char *outdef, char *errdef)
{
    assert(outdef != NULL && errdef != NULL);

    outmarks = errmarks = NULL;
    if (loadfile(&outtype, outdef, &out) != 0)
	return -1;
    out_timestamp = err_timestamp;
    err_timestamp = 0;
    out_size = err_size;
    strcpy(out_md5, err_md5);
    if (cf_getrptnum(0, CF_RPTSTDERR) != CF_STDERR_MIXED)
	return loadfile(&errtype, errdef, &err);
    else
	return 0;
}

/*
** grow_marks
**	Grow a list of marker
*/
static void grow_marks(struct marker **marks, int *max)
{
    struct marker *new;

    *max *= 2;
    new = (struct marker *) realloc(*marks, *max * sizeof(struct marker));
    if (new == NULL) {
	error("realloc() failed: %s", ERRSTR);
	free(*marks);
	*marks = NULL;
    } else
	*marks = new;
}

/*
** set_mark
**	Set 1 mark
*/
static void
set_mark(struct marker **marks, int *mark, int *max, size_t offset,
	 int type)
{
    int i;

    i = *mark - 1;
    while (i >= 0 && (*marks)[i].offset > offset)
	i -= 1;

    if (i >= 0 && (*marks)[i].offset == offset)
	(*marks)[i].type |= type;
    else {
	int j;

	if (*mark == *max - 2)
	    grow_marks(marks, max);

	j = *mark;
	while (j > i) {
	    (*marks)[j + 1].offset = (*marks)[j].offset;
	    (*marks)[j + 1].type = (*marks)[j].type;
	    j -= 1;
	}
	(*marks)[i + 1].type = type;
	(*marks)[i + 1].offset = offset;
	*mark += 1;
    }
}

/*
** analyze_file
**	Analyze a file according to user specified regular expressions.
*/
static int
analyze_file(char *str, size_t len, int type,
	     struct condition *list, struct marker **marks)
{
    char *ln, *lndup, *nl;
    int max, mark, lineno, condno;

    assert(type == RE || type == PCRE);
    assert(list != NULL);
    assert(*marks == NULL);

    if (len == 0)
	return 0;

    assert(str != NULL);

    /* Allocate the marks array */
    max = 10;
    *marks = (struct marker *) malloc(max * sizeof(struct marker));
    if (*marks == NULL) {
	error("malloc() failed: %s", ERRSTR);
	return -1;
    }
    mark = 0;
    (*marks)[mark].type = NONE;

    ln = str;
    nl = str;
    lineno = 0;
    lndup = NULL;
    while (ln - str < len) {
	/* Parse the file line by line */
	while (nl - str < len && *nl != '\n' && *nl != '\0') {
	    if (*marks != NULL) {
		if (*nl == '<')
		    set_mark(marks, &mark, &max, nl - str, LT);
		else if (*nl == '>')
		    set_mark(marks, &mark, &max, nl - str, GT);
		else if (*nl == '&')
		    set_mark(marks, &mark, &max, nl - str, AMP);
	    }
	    nl += 1;
	}

	if (*nl == '\0')
	    break;

	if (*nl == '\n') {
	    set_mark(marks, &mark, &max, nl - str, NL);
	    *nl = '\0';
	}
	lineno += 1;

	if (*nl != '\0' && (nl - str) == len) {
	    assert(lndup == NULL);
	    lndup = (char *) malloc(nl - str + 1);
	    if (lndup == NULL) {
		error("malloc() failed: %s", ERRSTR);
		return -1;
	    }
	    memcpy(lndup, ln, nl - str);
	    lndup[nl - str] = '\0';
	    ln = lndup;
	}

	debug(DDATA, "Data line: [%s]", ln);

	condno = 0;
	while (list[condno].code != -1) {
	    int r;

	    if (type == RE) {
		regmatch_t pmatch[1024];

		r = regexec(&(list[condno].val.re), ln, 1024, pmatch, 0);
		if (r != 0 && r != REG_NOMATCH) {
		    /* Something bad happened */
		    char buf[1024];

		    if (regerror(r, &(list[condno].val.re), buf, 1024) !=
			0)
			error
			    ("Fatal error during output analysis: regexec() failed: %s",
			     buf);
		    else
			error
			    ("Fatal error during output analysis: regexec() failed with code %d",
			     r);
		    error("Regular expression used was: %s",
			  list[condno].expression);
		    error("Trying to match the following line of data: %s",
			  buf);
		    if (lndup != NULL)
			free(lndup);
		    else
			*nl = '\n';
		    return -1;
		} else {
		    if (r == 0) {
			/* Matched */
			debug(DDATA,
			      "Matched: #%d %d[%c] (REG_NOMATCH=%d)",
			      condno + 1, r, list[condno].code,
			      REG_NOMATCH);
			byteset_set(list[condno].code,
				    1 + byteset_get(list[condno].code));
			/* Check for substrings */
			r = 1;
			while (r < 1024) {
			    if (pmatch[r].rm_so == -1)
				break;
			    set_mark(marks, &mark, &max,
				     ln - str + pmatch[r].rm_so, TOGGLE);
			    set_mark(marks, &mark, &max,
				     ln - str + pmatch[r].rm_eo, TOGGLE);
			    r += 1;
			}
			break;
		    } else
			debug(DVDATA,
			      "Failed: #%d %d[%c] (REG_NOMATCH=%d)",
			      condno + 1, r, list[condno].code,
			      REG_NOMATCH);
		}
	    }
#if defined(HAVE_PCRE_H)
	    else if (type == PCRE) {
		int ovector[3072];

		r = pcre_exec(list[condno].val.pcre, NULL, ln,
			      (lndup != NULL) ? strlen(lndup) : nl - ln,
			      0, 0, ovector, 3072);
		if (r < 0 && r != PCRE_ERROR_NOMATCH) {
		    /* Something bad happened */
		    error
			("Fatal error during output analysis: pcre_exec() failed with code %d",
			 r);
		    error("Regular expression used was: %s",
			  list[condno].expression);
		    error("Trying to match the following line of data: %s",
			  ln);
		    if (lndup != NULL)
			free(lndup);
		    else
			*nl = '\n';
		    return -1;
		} else {
		    if (r >= 0) {
			int i;

			/* Matched */
			debug(DDATA,
			      "Matched: #%d %d[%c] (PCRE_ERROR_NOMATCH=%d)",
			      condno + 1, r, list[condno].code,
			      PCRE_ERROR_NOMATCH);
			byteset_set(list[condno].code, 1);
			/* Check for substrings */
			i = 1;
			while (i < r) {
			    if (ovector[i * 2] < 0
				|| ovector[i * 2 + 1] < 0)
				continue;
			    set_mark(marks, &mark, &max,
				     ln - str + ovector[i * 2], TOGGLE);
			    set_mark(marks, &mark, &max,
				     ln - str + ovector[i * 2 + 1],
				     TOGGLE);
			    i += 1;
			}
			break;
		    } else
			debug(DVDATA,
			      "Failed: #%d %d[%c] (PCRE_ERROR_NOMATCH=%d)",
			      condno + 1, r, list[condno].code,
			      PCRE_ERROR_NOMATCH);
		}
	    }
#endif
	    else
		abort();

	    condno += 1;
	}
	if (list[condno].code == -1)
	    debug(DDATA, "No match!");

	if (lndup == NULL)
	    *nl = '\n';

	ln = ++nl;
    }

    if (lndup != NULL)
	free(lndup);

    return lineno;
}

/*
** analyzer_run
**	Analyze the given files according to user specified regular expressions.
*/
int analyzer_run(char *ostr, size_t olen, char *estr, size_t elen)
{
    int o, e;

    o = analyze_file(ostr, olen, outtype, out, &outmarks);
    if (estr != NULL)
	e = analyze_file(estr, elen, errtype, err, &errmarks);
    else
	e = 0;

    variable_set(V_LINES, o + e);
    variable_set(V_OUTLINES, o);
    variable_set(V_ERRLINES, e);

    if (o < 0 || e < 0)
	return -1;
    return 0;
}

/*
** output_file
**	Output a single file
*/
static int
output_file(FILE * mail, char *name, int format, size_t limit,
	    char *str, size_t len, struct marker *marks)
{
    size_t ptr;
    char toggle, trunc;

    if (name != NULL) {
	if (format == CF_FORMAT_RICH)
	    toggle =
		fprintf(mail,
			"<Center>----- <Underline>Command standard %s follows</Underline> -----</Center>",
			name);
	else if (format == CF_FORMAT_HTML)
	    toggle =
		fprintf(mail,
			"<p align=center>----- <em>Command standard %s follows</em> -----</p>",
			name);
	else
	    toggle = 1;
	if (toggle <= 0) {
	    error("fprintf() failed: %s", ERRSTR);
	    return -1;
	}
    }

    if (format == CF_FORMAT_RICH && fprintf(mail, "<Nofill>") != 8) {
	error("fprintf() failed: %s", ERRSTR);
	return -1;
    }
    if (format == CF_FORMAT_HTML && fprintf(mail, "<pre>") != 5) {
	error("fprintf() failed: %s", ERRSTR);
	return -1;
    }
    ptr = 0;
    toggle = 0;
    trunc = 0;
    while (marks->type != NONE) {
	if (marks->offset - ptr > 0) {
	    if (fwrite(str + ptr, marks->offset - ptr, 1, mail) != 1) {
		error("fwrite() failed: %s", ERRSTR);
		return -1;
	    }
	    ptr = marks->offset;
	}

	if (format != CF_FORMAT_TEXT && (marks->type & TOGGLE) == TOGGLE) {
	    char *tag;

	    if (toggle == 0)
		tag = (format == CF_FORMAT_RICH) ? "<Bold>" : "<b>";
	    else
		tag = (format == CF_FORMAT_RICH) ? "</Bold>" : "</b>";

	    if (fprintf(mail, tag) != strlen(tag)) {
		error("fprintf() failed: %s", ERRSTR);
		return -1;
	    }
	    toggle = 1 - toggle;
	}

	if (format == CF_FORMAT_HTML
	    && (marks->type & (LT | GT | AMP)) != 0) {
	    char *esc;

	    if ((marks->type & LT) == LT)
		esc = "&lt;";
	    else if ((marks->type & GT) == GT)
		esc = "&gt;";
	    else if ((marks->type & AMP) == AMP)
		esc = "&amp;";
	    else
		abort();

	    if (fprintf(mail, esc) != strlen(esc)) {
		error("fprintf() failed: %s", ERRSTR);
		return -1;
	    }

	    /* Skip the character, it was escaped! */
	    ptr += 1;
	}

	if (format == CF_FORMAT_RICH && (marks->type & LT) == LT
	    && fprintf(mail, "<") != 1) {
	    error("fprintf() failed: %s", ERRSTR);
	    return -1;
	}

	if (trunc == 0 && (marks->type & NL) == NL) {
	    if (len > limit && marks->offset > limit / 2) {
		struct marker *hole;
		u_long lines;

		lines = 0;
		hole = marks;
		while ((marks->type & NL) != NL
		       || hole->offset + len - marks->offset > limit) {
		    if ((marks->type & NL) == NL)
			lines += 1;
		    marks += 1;
		}

		if (format == CF_FORMAT_TEXT
		    && fprintf(mail,
			       "\n[... %lu lines (%ld bytes) of output truncated ...]\n",
			       lines, marks->offset - hole->offset) <= 0) {
		    error("fprintf() failed: %s", ERRSTR);
		    return -1;
		}
		if (format == CF_FORMAT_RICH
		    && fprintf(mail,
			       "</Nofill><Center>----- <Underline>%lu lines (%ld bytes) of output truncated</Underline> -----</Center><Nofill>",
			       lines, marks->offset - hole->offset) <= 0) {
		    error("fprintf() failed: %s", ERRSTR);
		    return -1;
		}
		if (format == CF_FORMAT_HTML
		    && fprintf(mail,
			       "</pre><p align=center>----- <em>%lu lines (%ld bytes) of output truncated</em> -----</p><pre>",
			       lines, marks->offset - hole->offset) <= 0) {
		    error("fprintf() failed: %s", ERRSTR);
		    return -1;
		}

		ptr = marks->offset + 1;
		trunc = 1;
	    }
	}

	marks += 1;
    }

    if (ptr < len && fwrite(str + ptr, len - ptr, 1, mail) != 1) {
	error("fwrite() failed: %s", ERRSTR);
	return -1;
    }
    if (str[len - 1] != '\n' && fprintf(mail, "\n") != 1) {
	error("fprintf() failed: %s", ERRSTR);
	return -1;
    }

    if (format == CF_FORMAT_RICH && fprintf(mail, "</Nofill>") != 9) {
	error("fprintf() failed: %s", ERRSTR);
	return -1;
    }
    if (format == CF_FORMAT_HTML && fprintf(mail, "</pre>") != 6) {
	error("fprintf() failed: %s", ERRSTR);
	return -1;
    }

    return 0;
}

/*
** analyzer_output
**	Output the result of an analysis
*/
int
analyzer_output(FILE * mail, int format, int output, size_t limit,
		char *ostr, size_t olen, char *estr, size_t elen)
{
    size_t olimit, elimit;
    int o, e, over;

    over = 0;
    if (limit <= 0)
	olimit = elimit = olen + elen;
    else if (output == CF_STDERR_OUT) {
	olimit = elimit = limit;
	if (olen > limit)
	    over = 1;
    } else if (output == CF_STDERR_ERR) {
	olimit = elimit = limit;
	if (elen > limit)
	    over = 1;
    } else {
	if (output == CF_STDERR_MIXED)
	    olimit = limit;
	else {
	    olimit = elimit = limit / 2;
	    if (olen < olimit)
		elimit = limit - olen;
	    if (elen < elimit)
		olimit = limit - elen;
	}
	if (olen + elen > limit)
	    over = 1;
    }

    if (over == 1) {
	if (format == CF_FORMAT_TEXT
	    && fprintf(mail,
		       "WARNING: The following output is truncated!\n         (Total of %ld bytes is over the limit of %ld)\n\n",
		       olen + elen, limit) <= 0) {
	    error("fprintf() failed: %s", ERRSTR);
	    return -1;
	}
	if (format == CF_FORMAT_RICH
	    && fprintf(mail,
		       "<Nofill><Bold>WARNING</Bold>: The following output is truncated!\n         (Total of %ld bytes is over the limit of %ld)</Nofill>\n\n",
		       olen + elen, limit) <= 0) {
	    error("fprintf() failed: %s", ERRSTR);
	    return -1;
	}
	if (format == CF_FORMAT_HTML
	    && fprintf(mail,
		       "<p><b>WARNING</b>: The following output is truncated! (Total of %ld bytes is over the limit of %ld)</p>",
		       olen + elen, limit) <= 0) {
	    error("fprintf() failed: %s", ERRSTR);
	    return -1;
	}
    }

    o = e = 0;
    if (output == CF_STDERR_ERR) {
	if (elen > 0)
	    e = output_file(mail, "error ", format, elimit,
			    estr, elen, errmarks);
    } else if (output == CF_STDERR_OUT) {
	if (olen > 0)
	    o = output_file(mail, "output ", format, olimit,
			    ostr, olen, outmarks);
    } else {
	if (elen > 0 && output == CF_STDERR_FIRST)
	    e = output_file(mail, (olen > 0) ? "error " : NULL,
			    format, elimit, estr, elen, errmarks);
	if (olen > 0)
	    o = output_file(mail, (output != CF_STDERR_MIXED
				   && elen > 0) ? "output" : NULL,
			    format, olimit, ostr, olen, outmarks);

	if (elen > 0 && output == CF_STDERR_LAST)
	    e = output_file(mail, (olen > 0) ? "error " : NULL,
			    format, elimit, estr, elen, errmarks);
    }

    if (o != 0 || e != 0)
	return -1;
    return 0;
}
