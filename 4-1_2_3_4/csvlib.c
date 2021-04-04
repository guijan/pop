#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csvlib.h"

struct csvstate {
	char		*line;		/* input chars */
	size_t		linelen;	/* strlen(line) */
	char		*sline;		/* line copy used to split */
	size_t	 	 maxline;	/* buffer size for line and sline */
	char		 **field;	/* pointers to fields */
	size_t	 	 maxfield;	/* size of field[] */
	size_t	 	 nfield;	/* number of fields in field[]. */
	const char	*sep;		/* field separator string */
	int	 	 sepalloc;	/* is sep allocated? */
};

/* initializes a struct csvstate with null pointers and default settings */
static const struct csvstate CSV_INITIALIZER = {
	.maxline = 512,
	.maxfield = 32,
	.sep = ","
};

static int	csv_readline(struct csvstate *, FILE *);
static void	csv_splitstr(struct csvstate *);
static size_t	advquoted(struct csvstate *, char *);
static size_t	advunquoted(struct csvstate *, char *);

/* csv_init: init state, return state
 * call csv_destroy() to free the state's resources, and before reusing a
 * buffer.
 * If state is NULL, allocate it.
 *
 * Returns NULL on malloc error.
 */
struct csvstate *
csv_init(struct csvstate *state)
{
	const void *ostate = state;
	if (state == NULL)
		if ((state = malloc(sizeof(*state))) == NULL)
				return (NULL);
	*state = CSV_INITIALIZER;
	if ((state->line = malloc(state->maxline * sizeof(*state->line)))
	    == NULL)
		goto err;
	if ((state->sline = malloc(state->maxline * sizeof(*state->sline)))
	    == NULL)
		goto err;
	if ((state->field = calloc(state->maxfield, sizeof(*state->field)))
	    == NULL)
		goto err;
	return (state);
err:
	free(state->line);
	free(state->sline);
	free(state->field);
	if (ostate == NULL)
		free(state);
	return (NULL);
}

/* csv_setopt: set csvstate options
 * Currently only supports setting the separator.
 */
int
csv_setopt(struct csvstate *state, int cmd, ...)
{
	va_list ap;
	int ret = -1;

	va_start(ap, cmd);
	switch (cmd) {
	case CSV_SEP:
		if (state->sepalloc)
			free((void *)state->sep);
		state->sep = va_arg(ap, char *);
		state->sepalloc = va_arg(ap, int);
		state->sepalloc = !!state->sepalloc;
	default:
		goto end;
	}

	ret = 0;
end:
	va_end(ap);
	return (ret);
}

/* csv_destroy: destroy resources in state, doesn't free state */
void
csv_destroy(struct csvstate *state)
{
	free(state->line);
	free(state->sline);
	free(state->field);
}

/* csv_nfield: return number of fields */
size_t
csv_nfield(struct csvstate *state)
{
	if (state->nfield == 0)
		csv_splitstr(state);
	return (state->nfield);
}

/* csv_field: return 0-indexed field n */
char *
csv_getfield(struct csvstate *state, size_t n)
{
	if (n >= csv_nfield(state))
		return (NULL);
	return (state->field[n]);
}

/* csv_getline: read a single csv line, return an untouched pointer to it
 * The buffer behind the pointer is this function's property and in addition to
 * being read-only must not be freed.
 *
 * Returns NULL on error.
 */
const char *
csv_getline(struct csvstate *state, FILE *fp)
{
	return (csv_readline(state, fp) == -1 ? NULL : state->line);
}

/* csv_readline: read line into csvstate, update structures accordingly
 * Returns -1 on error.
 */
static int
csv_readline(struct csvstate *state, FILE *fp)
{
	const size_t oldmaxline = state->maxline;
	size_t nread;
	void *tp;
	int retval = -1;

	/*
	 * nread == strlen(state->line) after this function call,
	 * state->line[nread] is the line's null terminator.
	 */
	if ((nread = getline(&state->line, &state->maxline, fp)) == -1)
		goto end;
	if (oldmaxline < state->maxline) {
		if ((tp = realloc(state->sline, state->maxline)) == NULL)
			goto end;
		state->sline = tp;
	}
	state->linelen = nread;
	nread++; /* add space for null terminator */
	memcpy(state->sline, state->line, nread);
	state->nfield = 0;
	retval = 0;
end:
	return (retval);
}

/* csv_splitstr: split CSV string into fields, update state. */
static void
csv_splitstr(struct csvstate *state)
{
	size_t flen;
	char *p;
	p = state->nfield == 0 ? state->sline : state->field[state->nfield];
	while (p - state->sline < state->linelen) {
		if (*p == '"')
			flen = advquoted(state, ++p); /* ++p skips the quote */
		else
			flen = advunquoted(state, p);
		state->field[state->nfield++] = p;
		p += flen + 1; /* +1 to skip over the terminating '\0' */
	}
}

/* advquoted: advanced a quoted CSV, return field length
 * Skip the quote before calling this function.
 */
static size_t
advquoted(struct csvstate *state, char *p)
{
	size_t i, j, k;
	for (i = j = 0; p[j] != '\0'; i++, j++) {
		if (p[j] == '"' && p[++j] != '"') {
			/* copy up to next separator or \0 */
			k = strcspn(p + j, state->sep);
			memmove(p + i, p + j, k);
			i += k;
			break;

		}
		p[i] = p[j];
	}
	p[i] = '\0';
	return (i);
}

/* advunquoted: advance an unquoted CSV, return field length */
static size_t
advunquoted(struct csvstate *state, char *p)
{
	size_t span;
	span = strcspn(p, state->sep);
	p[span] = '\0';
	return (span);
}
