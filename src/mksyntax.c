/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#ifndef lint
static char const copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mksyntax.c	8.2 (Berkeley) 5/4/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * This program creates syntax.h and syntax.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


struct synclass {
	const char *name;
	const char *comment;
};

/* Syntax classes */
static const struct synclass synclass[] = {
	{ "CWORD",	"character is nothing special" },
	{ "CNL",	"newline character" },
	{ "CQNL",	"newline character in quotes" },
	{ "CBACK",	"a backslash character" },
	{ "CSBACK",	"a backslash character in single quotes" },
	{ "CSQUOTE",	"single quote" },
	{ "CDQUOTE",	"double quote" },
	{ "CENDQUOTE",	"a terminating quote" },
	{ "CBQUOTE",	"backwards single quote" },
	{ "CVAR",	"a dollar sign" },
	{ "CENDVAR",	"a '}' character" },
	{ "CLP",	"a left paren in arithmetic" },
	{ "CRP",	"a right paren in arithmetic" },
	{ "CEOF",	"end of file" },
	{ "CCTL",	"like CWORD, except it must be escaped" },
	{ "CSPCL",	"these terminate a word" },
	{ "CIGN",       "character should be ignored" },
	{ NULL,		NULL }
};


/*
 * Syntax classes for is_ functions.  Warning:  if you add new classes
 * you may have to change the definition of the is_in_name macro.
 */
static const struct synclass is_entry[] = {
	{ "ISDIGIT",	"a digit" },
	{ "ISUPPER",	"an upper case letter" },
	{ "ISLOWER",	"a lower case letter" },
	{ "ISUNDER",	"an underscore" },
	{ "ISSPECL",	"the name of a special parameter" },
	{ NULL, 	NULL }
};

static const char writer[] = "\
/*\n\
 * This file was generated by the mksyntax program.\n\
 */\n\
\n";


static FILE *cfile;
static FILE *hfile;

static void add_default(void);
static void finish(void);
static void init(const char *);
static void add(const char *, const char *);
static void output_type_macros(void);

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	int i;
	char buf[80];
	int pos;

	/* Create output files */
	if ((cfile = fopen("syntax.c", "w")) == NULL) {
		perror("syntax.c");
		exit(2);
	}
	if ((hfile = fopen("syntax.h", "w")) == NULL) {
		perror("syntax.h");
		exit(2);
	}
	fputs(writer, hfile);
	fputs(writer, cfile);

	fputs("#include <sys/cdefs.h>\n", hfile);
	fputs("#include <limits.h>\n\n", hfile);

	/* Generate the #define statements in the header file */
	fputs("/* Syntax classes */\n", hfile);
	for (i = 0 ; synclass[i].name ; i++) {
		sprintf(buf, "#define %s %d", synclass[i].name, i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", synclass[i].comment);
	}
	putc('\n', hfile);
	fputs("/* Syntax classes for is_ functions */\n", hfile);
	for (i = 0 ; is_entry[i].name ; i++) {
		sprintf(buf, "#define %s %#o", is_entry[i].name, 1 << i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", is_entry[i].comment);
	}
	putc('\n', hfile);
	fputs("#define SYNBASE (1 - CHAR_MIN)\n", hfile);
	fputs("#define PEOF -SYNBASE\n\n", hfile);
	putc('\n', hfile);
	fputs("#define BASESYNTAX (basesyntax + SYNBASE)\n", hfile);
	fputs("#define DQSYNTAX (dqsyntax + SYNBASE)\n", hfile);
	fputs("#define SQSYNTAX (sqsyntax + SYNBASE)\n", hfile);
	fputs("#define ARISYNTAX (arisyntax + SYNBASE)\n", hfile);
	putc('\n', hfile);
	output_type_macros();		/* is_digit, etc. */
	putc('\n', hfile);

	/* Generate the syntax tables. */
	fputs("#include \"parser.h\"\n", cfile);
	fputs("#include \"shell.h\"\n", cfile);
	fputs("#include \"syntax.h\"\n\n", cfile);

	fputs("/* syntax table used when not in quotes */\n", cfile);
	init("basesyntax");
	add_default();
	add("\n", "CNL");
	add("\\", "CBACK");
	add("'", "CSQUOTE");
	add("\"", "CDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("<>();&| \t", "CSPCL");
	finish();

	fputs("\n/* syntax table used when in double quotes */\n", cfile);
	init("dqsyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CBACK");
	add("\"", "CENDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	/* ':/' for tilde expansion, '-^]' for [a\-x] pattern ranges */
	add("!*?[]=~:/-^", "CCTL");
	finish();

	fputs("\n/* syntax table used when in single quotes */\n", cfile);
	init("sqsyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CSBACK");
	add("'", "CENDQUOTE");
	/* ':/' for tilde expansion, '-^]' for [a\-x] pattern ranges */
	add("!*?[]=~:/-^", "CCTL");
	finish();

	fputs("\n/* syntax table used when in arithmetic */\n", cfile);
	init("arisyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CBACK");
	add("`", "CBQUOTE");
	add("\"", "CIGN");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("(", "CLP");
	add(")", "CRP");
	finish();

	fputs("\n/* character classification table */\n", cfile);
	init("is_type");
	add("0123456789", "ISDIGIT");
	add("abcdefghijklmnopqrstuvwxyz", "ISLOWER");
	add("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "ISUPPER");
	add("_", "ISUNDER");
	add("#?$!-*@", "ISSPECL");
	finish();

	exit(0);
}


/*
 * Output the header and declaration of a syntax table.
 */

static void
init(const char *name)
{
	fprintf(hfile, "extern const char %s[];\n", name);
	fprintf(cfile, "const char %s[SYNBASE + CHAR_MAX + 1] = {\n", name);
}


static void
add_one(const char *key, const char *type)
{
	fprintf(cfile, "\t[SYNBASE + %s] = %s,\n", key, type);
}


/*
 * Add default values to the syntax table.
 */

static void
add_default(void)
{
	add_one("PEOF",                "CEOF");
	add_one("CTLESC",              "CCTL");
	add_one("CTLVAR",              "CCTL");
	add_one("CTLENDVAR",           "CCTL");
	add_one("CTLBACKQ",            "CCTL");
	add_one("CTLBACKQ + CTLQUOTE", "CCTL");
	add_one("CTLARI",              "CCTL");
	add_one("CTLENDARI",           "CCTL");
	add_one("CTLQUOTEMARK",        "CCTL");
	add_one("CTLQUOTEEND",         "CCTL");
}


/*
 * Output the footer of a syntax table.
 */

static void
finish(void)
{
	fputs("};\n", cfile);
}


/*
 * Add entries to the syntax table.
 */

static void
add(const char *p, const char *type)
{
	for (; *p; ++p) {
		char c = *p;
		switch (c) {
		case '\t': c = 't';  break;
		case '\n': c = 'n';  break;
		case '\'': c = '\''; break;
		case '\\': c = '\\'; break;

		default:
			fprintf(cfile, "\t[SYNBASE + '%c'] = %s,\n", c, type);
			continue;
		}
		fprintf(cfile, "\t[SYNBASE + '\\%c'] = %s,\n", c, type);
	}
}


/*
 * Output character classification macros (e.g. is_digit).  If digits are
 * contiguous, we can test for them quickly.
 */

static const char *macro[] = {
	"#define is_digit(c)\t((unsigned int)((c) - '0') <= 9)",
	"#define is_eof(c)\t((c) == PEOF)",
	"#define is_alpha(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER))",
	"#define is_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER))",
	"#define is_in_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER|ISDIGIT))",
	"#define is_special(c)\t((is_type+SYNBASE)[(int)c] & (ISSPECL|ISDIGIT))",
	"#define digit_val(c)\t((c) - '0')",
	NULL
};

static void
output_type_macros(void)
{
	const char **pp;

	for (pp = macro ; *pp ; pp++)
		fprintf(hfile, "%s\n", *pp);
}
