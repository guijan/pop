#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* OP_TYPE_DECL: declares functions for serializing standard integer types. */
#define OP_TYPE_DECL(NAME)						\
	static int							\
	op_##NAME(unsigned char **, struct parsedspec *, va_list *, 	\
	    const char **_fmt)

/* OP_TYPE_DEF: generates functions for serializing standard integer types.
 *
 * TYPE is the standard integer type the function serializes.
 * PROMOTION is what it promotes to, it's only different from TYPE for char and
 * short.
 * NAME creates the function's name through op_##NAME.
 *
 * Updates _buf to point to a pointer to one past the last modified character.
 * Reinitializes specp for the next conversion specifier.
 * Updates _fmt to point to one operator past the one that invoked the function.
 *
 * Never fails.
 */
#define OP_TYPE_DEF(TYPE, PROMOTION, NAME)				\
	static int							\
	op_##NAME(unsigned char **_buf, struct parsedspec *specp,	\
	    va_list *vap, const char **_fmt)				\
	{								\
		unsigned char *buf = *_buf;				\
		unsigned TYPE conv;					\
		int i, j;						\
									\
		for (i = 0; i < specp->repeat; i++) {			\
			if (specp->sign)				\
				conv = va_arg(*vap, signed PROMOTION);	\
			else						\
				conv = va_arg(*vap, unsigned PROMOTION);\
			j = sizes[(unsigned char)**_fmt];		\
			while (j > 0)					\
				*buf++ = conv >> --j * CHAR_BIT;	\
		}							\
									\
		*specp = PS_INITIALIZER;				\
		(*_fmt)++;						\
		*_buf = buf;						\
		return (0);						\
	}

struct parsedspec {
	size_t repeat;		/* How many times to repeat. */
	int chars;		/* How many chars are in the type. */
	unsigned char opind;	/* Index of operator on function arrays. */
	int sign;		/* True if negative. */
};

/* operation: a class of functions that are operators in the language.
 * specp must be initialized by assigning PS_INITIALIZER to it before its first
 * use with a new format string.
 *
 * Returns -1 on failure and leaves the area of memory it's supposed to modify
 * undefined.
 */
typedef int (operation)(unsigned char **, struct parsedspec *specp, va_list *,
    const char **);


static int op_invalid(unsigned char **, struct parsedspec *, va_list *,
    const char **);
static int op_plainchar(unsigned char **, struct parsedspec *, va_list *,
    const char **);
OP_TYPE_DECL(char);
OP_TYPE_DECL(short);
OP_TYPE_DECL(int);
OP_TYPE_DECL(long);
OP_TYPE_DECL(llong);
static int op_string(unsigned char **, struct parsedspec *, va_list *,
    const char **);
static int op_repeat(unsigned char **, struct parsedspec *, va_list *,
    const char **);
static int op_sign(unsigned char **, struct parsedspec *, va_list *,
    const char **);

static int de_plainchar(unsigned char **, struct parsedspec *, va_list *,
    const char **);

static const unsigned char operators[UCHAR_MAX] = {
	[0]   = 0,	/* All zeroed space is an invalid operator. */

	['C'] = 1,	/* plain char */
	['c'] = 2,	/* signed/unsigned char */
	['s'] = 3,	/* short */
	['i'] = 4,	/* int */
	['l'] = 5,	/* long */
	['L'] = 6,	/* long long */
	['S'] = 7,	/* C string */

	['0'] = 8,	/* repeat */
	['1'] = 8,
	['2'] = 8,
	['3'] = 8,
	['4'] = 8,
	['5'] = 8,
	['6'] = 8,
	['7'] = 8,
	['8'] = 8,
	['9'] = 8,

	['-'] =	9,	/* sign */
};

static const unsigned char sizes[] = {
	0,	/* invalid */
	1,	/* plain char */
	1,	/* signed/unsigned char */
	2,	/* short */
	2,	/* int */
	4,	/* long */
	8,	/* long long */
	1,	/* One character of a C string */
};

static operation *const operations[] = {
	op_invalid,
	op_plainchar,
	op_char,
	op_short,
	op_int,
	op_long,
	op_llong,
	op_string,

	op_repeat,

	op_sign,
};

static operation *const deserializers[] = {
	op_invalid,
	de_plainchar,
	de_char,
	de_short,
	de_int,
	de_long,
	de_llong,
	de_string,

	de_repeat,

	de_sign,
};

static const struct parsedspec PS_INITIALIZER = {
	.repeat = 1,
};


/* packbe: serialize varargs according to fmt.
 *
 * Returns a pointer to one past the last defined byte on success.
 * Returns NULL on parse error.
 */
void *
packbe(void *_buf, const char *fmt, ...)
{
	va_list va;
	unsigned char op;
	unsigned char *buf = _buf;
	void *retp = NULL;
	struct parsedspec spec;

	va_start(va, fmt);
	spec = PS_INITIALIZER;
	while (*fmt != '\0') {
		op = operators[(unsigned char)*fmt];
		if (operations[op](&buf, &spec, &va, &fmt) == -1)
			goto end;
	}
	retp = buf;
end:
	va_end(va);
	return (retp);
}

/* op_invalid: operator is invalid, print it and error exit */
static int
op_invalid(/* unused */ unsigned char **_buf,
    /* unused */ struct parsedspec *specp, /* unused */ va_list *vap,
    const char **_fmt)
{
	const char *fmt = *_fmt;
	fprintf(stderr, "invalid operator: %c\n", *fmt);
	return (-1);
}

static int
op_plainchar(unsigned char **_buf, struct parsedspec *specp, va_list *vap,
    const char **_fmt)
{
	specp->sign = (char)-1 < 0;
	return (op_char(_buf, specp, vap, _fmt));
}

OP_TYPE_DEF(char, int, char);
OP_TYPE_DEF(short, int, short);
OP_TYPE_DEF(int, int, int);
OP_TYPE_DEF(long, long, long);
OP_TYPE_DEF(long long, long long, llong);

static int
op_string(/* unused */ unsigned char **_buf, struct parsedspec *specp,
    va_list *vap, const char **_fmt)
{
	unsigned char *buf = *_buf;
	char *strp;

	strp = va_arg(*vap, char *);
	while ((*buf++ = *strp++) != '\0')
		;

	*specp = PS_INITIALIZER;
	*_buf = buf;
	(*_fmt)++;
	return (0);
}

/* op_repeat: set repeat count and advance *_fmt to the proper place */
static int
op_repeat(/* unused */ unsigned char **_buf, struct parsedspec *specp,
    /* unused */ va_list *vap, const char **_fmt)
{
	const char *fmt = *_fmt;
	long cnt;
	char *endptr;

	endptr = (char *)fmt;
	errno = 0;
	cnt = strtol(fmt, &endptr, 10);
	if (cnt <= 0 || (errno == ERANGE && cnt == LONG_MAX))
		return (-1);
	specp->repeat = cnt;
	fmt = endptr;

	*_fmt = fmt;
	return (0);
}

/* op_sign: our number is signed. */
static int
op_sign(/* unused */ unsigned char **_buf, struct parsedspec *specp,
    /* unused */ va_list *vap, const char **_fmt)
{
	(*_fmt)++;
	specp->sign = 1;
	return (0);
}

/* unpackbe: */
void *
unpackbe(void *_buf, const char *fmt, ...)
{
	va_list va;
	unsigned char op;
	unsigned char *buf = _buf;
	void *retp = NULL;
	struct parsedspec spec;

	va_start(va, fmt);
	spec = PS_INITIALIZER;
	while (*fmt != '\0') {
		op = operators[(unsigned char)*fmt];
		if (deserializers[op](&buf, &spec, &va, &fmt) == -1)
			goto end;
	}
	retp = buf;
end:
	va_end(va);
	return (retp);
}

static int
de_plainchar(unsigned char **_buf, struct parsedspec *specp, va_list *vap,
    const char **_fmt)
{
	specp->sign = (char)-1 < 0;
	return (de_char(_buf, specp, vap, _fmt));
}

static int
de_char(unsigned char **_buf, struct parsedspec *specp, va_list *vap,
    const char **_fmt)
{
	unsigned char *p;
	union {
		unsigned char	u;
		signed char	s;
	} n;
	size_t i;
	unsigned char *buf = *_buf;

	p = va_arg(*vap, unsigned char *);
	n.u = 0;
	for (i = 1; i > 0; )
		n.u |= *buf++ << (--i * CHAR_BIT);

	if (specp->sign) {

	} else {

	}
	*p = n.u;
	return (0);
}
