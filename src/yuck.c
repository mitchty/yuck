/*** yuck.c -- generate umbrella commands
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of yuck.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif	/* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif	/* UNLIKELY */
#if !defined UNUSED
# define UNUSED(_x)	_x __attribute__((unused))
#endif	/* !UNUSED */

#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*x))
#endif	/* !countof */

#define _paste(x, y)	x ## y
#define paste(x, y)	_paste(x, y)
#if !defined with
# define with(args...)							\
	for (args, *paste(__ep, __LINE__) = (void*)1;			\
	     paste(__ep, __LINE__); paste(__ep, __LINE__) = 0)
#endif	/* !with */

struct usg_s {
	char *umb;
	char *cmd;
	char *desc;
};

struct opt_s {
	char sopt;
	char *lopt;
	char *larg;
	char *desc;
};


static __attribute__((format(printf, 1, 2))) void
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}

static inline __attribute__((unused)) void*
deconst(const void *cp)
{
	union {
		const void *c;
		void *p;
	} tmp = {cp};
	return tmp.p;
}

static FILE*
get_fn(int argc, char *argv[])
{
	FILE *res;

	if (argc > 1) {
		const char *fn = argv[1];
		if (UNLIKELY((res = fopen(fn, "r")) == NULL)) {
			error("cannot open file `%s'", fn);
		}
	} else {
		res = stdin;
	}
	return res;
}

static inline __attribute__((always_inline)) unsigned int
fls(unsigned int x)
{
	return x ? sizeof(x) * 8U - __builtin_clz(x) : 0U;
}

static inline __attribute__((always_inline)) size_t
max_zu(size_t x, size_t y)
{
	return x > y ? x : y;
}

static size_t
xstrncpy(char *restrict dst, const char *src, size_t ssz)
{
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static char*
xstrdup(const char *str)
{
	if (str == NULL) {
		return NULL;
	}
	return strdup(str);
}

static __attribute__((unused)) bool
xstreqp(const char *s1, const char *s2)
{
	if (s1 == NULL && s2 == NULL) {
		return true;
	} else if (s1 == NULL || s2 == NULL) {
		/* one of them isn't NULL */
		return false;
	}
	/* resort to normal strcmp */
	return !strcasecmp(s1, s2);
}

static void
xlcase_(char *restrict buf)
{
	if (buf == NULL) {
		/* nothing to do */
		return;
	}
	for (char *restrict bp = buf; *bp; bp++) {
		if (ispunct(*bp)) {
			*bp = '_';
		} else if (isupper(*bp)) {
			*bp = (char)tolower(*bp);
		}
	}
	return;
}


/* bang buffers */
typedef struct {
	/* the actual buffer (resizable) */
	char *s;
	/* current size */
	size_t z;
}  bbuf_t;

static char*
bbuf_cpy(bbuf_t b[static 1U], const char *str, size_t ssz)
{
	size_t nu = max_zu(fls(ssz + 1U) + 1U, 6U);
	size_t ol = b->z ? max_zu(fls(b->z) + 1U, 6U) : 0U;

	if (UNLIKELY(nu > ol)) {
		b->s = realloc(b->s, (1U << nu) * sizeof(*b->s));
	}
	xstrncpy(b->s, str, ssz);
	b->z += ssz;
	return b->s;
}

static char*
bbuf_cat(bbuf_t b[static 1U], const char *str, size_t ssz)
{
	size_t nu = max_zu(fls(b->z + ssz + 1U) + 1U, 6U);
	size_t ol = b->z ? max_zu(fls(b->z) + 1U, 6U) : 0U;

	if (UNLIKELY(nu > ol)) {
		b->s = realloc(b->s, (1U << nu) * sizeof(*b->s));
	}
	xstrncpy(b->s + b->z, str, ssz);
	b->z += ssz;
	return b->s;
}


static void yield_usg(const struct usg_s *arg);
static void yield_opt(const struct opt_s *arg);

static int
usagep(const char *line, size_t llen)
{
#define STREQLITP(x, lit)      (!strncasecmp((x), lit, sizeof(lit) - 1))
	static struct usg_s cur_usg;
	static bbuf_t umb[1U];
	static bbuf_t cmd[1U];
	static bool cur_usg_ylddp;
	const char *sp;
	const char *up;
	const char *cp;
	const char *const ep = line + llen;

	if (UNLIKELY(line == NULL)) {
		goto yield;
	}

	if (!STREQLITP(line, "usage:")) {
	yield:
		if (!cur_usg_ylddp) {
			yield_usg(&cur_usg);
			cur_usg_ylddp = true;
		}
		return 0;
	}
	/* overread whitespace then */
	for (sp = line + sizeof("usage:") - 1; sp < ep && isspace(*sp); sp++);
	/* first thing should name the umbrella, find its end */
	for (up = sp; sp < ep && !isspace(*sp); sp++);

	if (cur_usg.umb && !strncasecmp(cur_usg.umb, up, sp - up)) {
		/* nothing new and fresh */
		;
	} else {
		cur_usg.umb = bbuf_cpy(umb, up, sp - up);
	}

	/* overread more whitespace then */
	for (; sp < ep && isspace(*sp); sp++);
	/* time for the command innit */
	for (cp = sp; sp < ep && !isspace(*sp); sp++);

	if (cur_usg.cmd && !strncasecmp(cur_usg.cmd, cp, sp - cp)) {
		/* nothing new and fresh */
		;
	} else if ((*cp != '<' || cp[--sp - cp++] == '>') &&
		   !strncasecmp(cp, "command", sp - cp)) {
		/* special command COMMAND or <command> */
		cur_usg.cmd = NULL;
	} else {
		cur_usg.cmd = bbuf_cpy(cmd, cp, sp - cp);
	}
	cur_usg_ylddp = false;
	return 1;
}

static int
optionp(const char *line, size_t llen)
{
	static struct opt_s cur_opt;
	static bbuf_t desc[1U];
	static bbuf_t lopt[1U];
	static bbuf_t larg[1U];
	const char *sp = line;
	const char *const ep = line + llen;

	if (UNLIKELY(line == NULL)) {
		goto yield;
	}
	/* overread whitespace */
	for (; sp < ep && isspace(*sp); sp++);
	if (sp - line >= 8) {
		/* we dont expect option specs that far out */
		goto desc;
	}

yield:
	/* must yield the old current option before it's too late */
	if (cur_opt.sopt || cur_opt.lopt) {
		yield_opt(&cur_opt);
	}
	if (cur_opt.lopt != NULL) {
		cur_opt.lopt = NULL;
		cur_opt.larg = NULL;
	}
	cur_opt.sopt = '\0';
	if (sp - line < 2) {
		/* can't be an option, can it? */
		return 0;
	}

	/* no yield pressure anymore, try parsing the line */
	sp++;
	if (*sp >= '0') {
		char sopt = *sp++;

		/* eat a comma as well */
		if (ispunct(*sp)) {
			sp++;
		}
		if (!isspace(*sp)) {
			/* dont know -x.SOMETHING? */
			return 0;
		}
		/* start over with the new option */
		sp++;
		cur_opt.sopt = sopt;
		if (*sp++ == '-') {
			/* must be a --long now, maybe */
			;
		}
	} else if (*sp == '-') {
		/* --option */
		;
	} else {
		/* dont know what this is */
		return 0;
	}

	/* --option */
	if (*sp++ == '-') {
		const char *op;

		for (op = sp; sp < ep && !isspace(*sp) && *sp != '='; sp++);
		cur_opt.lopt = bbuf_cpy(lopt, op, sp - op);

		if (*sp++ == '=') {
			/* has got an arg */
			const char *ap;
			for (ap = sp; sp < ep && !isspace(*sp); sp++);
			cur_opt.larg = bbuf_cpy(larg, ap, sp - ap);
		}
	}
	/* require at least one more space? */
	;
	/* space eater */
	for (; sp < ep && isspace(*sp); sp++);
	/* dont free but reset the old guy */
	desc->z = 0U;
desc:
	with (size_t sz = llen - (sp - line)) {
		cur_opt.desc = bbuf_cat(desc, sp, sz);
	}
	return 1;
}


static struct usg_s *usgs;
static size_t nusgs;
static struct ou_s {
	size_t usg;
	struct opt_s opt;
} *opts;
static size_t nopts;

static void
yield_usg(const struct usg_s *arg)
{
	size_t idx = nusgs++;

	if (!(idx % 64U)) {
		usgs = realloc(usgs, (idx + 64U) * sizeof(*usgs));
	}
	/* clone usg */
	usgs[idx].umb = xstrdup(arg->umb);
	usgs[idx].cmd = xstrdup(arg->cmd);
	usgs[idx].desc = xstrdup(arg->desc);
	return;
}

static void
yield_opt(const struct opt_s *arg)
{
	size_t idx = nopts++;

	if (!(idx % 64U)) {
		opts = realloc(opts, (idx + 64U) * sizeof(*opts));
	}
	/* clone opt */
	opts[idx].usg = nusgs - 1U;
	opts[idx].opt.sopt = arg->sopt;
	opts[idx].opt.lopt = xstrdup(arg->lopt);
	opts[idx].opt.larg = xstrdup(arg->larg);
	opts[idx].opt.desc = xstrdup(arg->desc);
	return;
}

static void
pr_opt(const struct opt_s *o)
{
	const char *lopt;

	if ((lopt = o->lopt) == NULL) {
		static char buf[8U] = "dash";
		buf[4U] = o->sopt;
		lopt = buf;
	}
	if (o->larg == NULL) {
		printf("\tunsigned int %s_given;\n", lopt);
	} else {
		printf("\tchar *%s_arg;\n", lopt);
	}
	return;
}

static void
pr_struct_none(const struct usg_s *u)
{
	for (const struct ou_s *o = opts; o < opts + nopts; o++) {
		if (u - usgs != o->usg) {
			continue;
		}
		pr_opt(&o->opt);
	}
	return;
}

static void
pr_struct(const struct usg_s *u)
{
	printf("\t\tstruct %s_%s_s {\n", u->umb, u->cmd);
	for (const struct ou_s *o = opts; o < opts + nopts; o++) {
		if (u - usgs != o->usg) {
			continue;
		}
		putchar('\t');
		putchar('\t');
		pr_opt(&o->opt);
	}
	printf("\t\t} %s;\n", u->cmd);
	return;
}

static void
pr_hdr(void)
{
	puts("\
#if !defined INCLUDED_yuck_h_\n\
#define INCLUDED_yuck_h_\n\
#include <stdlib.h>\n");

	puts("enum yuck_cmds_e {");
	for (const struct usg_s *u = usgs; u < usgs + nusgs; u++) {
		xlcase_(u->umb);
		xlcase_(u->cmd);

		printf("\t%s_%s,\n", u->umb, u->cmd ?: "NONE");
	}
	with (const struct usg_s *lu = usgs + nusgs - 1U) {
		printf("\tyuck_NCMDS = %s_%s\n", lu->umb, lu->cmd ?: "NONE");
	}
	puts("};\n");

	for (const struct ou_s *o = opts; o < opts + nopts; o++) {
		xlcase_(o->opt.lopt);
	}

	puts("struct yuck_s {");
	puts("\tenum yuck_cmds_e cmd;");
	/* first cmd lists all the common options, innit? */
	pr_struct_none(usgs);
	puts("\tunion {");
	for (const struct usg_s *u = usgs + 1U; u < usgs + nusgs; u++) {
		pr_struct(u);
	}
	puts("\t};");
	puts("\tsize_t nargs;");
	puts("\tchar *const *args;");
	puts("};\n");

	puts("\
\n\
extern int yuck_parse(struct yuck_s *restrict, int argc, char *const argv[]);\n\
extern void yuck_free(struct yuck_s *restrict);\n");

	puts("\
#endif  /* INCLUDED_yuck_h_ */");
	return;
}


static enum {
	UNKNOWN,
	SET_UMBCMD,
	SET_OPTION,
}
snarf_ln(char *line, size_t llen)
{
	static unsigned int st;

start_over:
	switch (st) {
	case UNKNOWN:
	case SET_UMBCMD:
		/* first keep looking for Usage: lines */
		if (usagep(line, llen)) {
			st = SET_UMBCMD;
		} else if (st == SET_UMBCMD) {
			/* finally, a yield! */
			st = UNKNOWN;
			goto start_over;
		}
	case SET_OPTION:
		/* check them option things */
		if (optionp(line, llen)) {
			st = SET_OPTION;
		} else if (st == SET_OPTION) {
			/* yield */
			st = UNKNOWN;
			goto start_over;
		}
	default:
		break;
	}
	return UNKNOWN;
}

static int
snarf_f(FILE *f)
{
	char *line = NULL;
	size_t llen = 0U;
	ssize_t nrd;

	while ((nrd = getline(&line, &llen, f)) > 0) {
		if (*line == '#') {
			continue;
		}
		snarf_ln(line, nrd);
	}
	/* drain */
	snarf_ln(NULL, 0U);

	free(line);
	return 0;
}


int
main(int argc, char *argv[])
{
	int rc = 0;
	FILE *yf;

	if (UNLIKELY((yf = get_fn(argc, argv)) == NULL)) {
		rc = -1;
	} else {
		/* let the snarfing begin */
		rc = snarf_f(yf);

		pr_hdr();

		/* clean up */
		fclose(yf);
	}

	return -rc;
}

/* yuck.c ends here */