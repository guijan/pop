#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/* Fold at fflag length if true. */
static int fflag;
/* Don't print non-printable and non-space characters if true. */
static int dflag;

#define TABWIDTH 8

static int	cook_args(int, const char *[]);
static int	vis(FILE *, void *, size_t);
static int	visbuf(FILE *, void *, size_t);
static void	tohex(char[], unsigned char);
static int	fflagnl(FILE *, size_t *, size_t);

/* vis: print files presented as arguments, escaping them */
int
main(int argc, char *argv[])
{
	int c;
	const char *err;
	while ((c = getopt(argc, argv, "df:")) != -1) {
		switch(c) {
		case 'd':
			dflag = 1;
			break;
		case 'f':
			fflag = strtonum(optarg, 0, INT_MAX, &err);
			if (err)
				printf("fold length is %s\n", err);
			break;
		default:
			abort();
			break;
		}
	}
	argv += optind;
	argc -= optind;

	if (cook_args(argc, (const char **)argv) == -1)
		return (EXIT_FAILURE);
	fflush(stdout);
	return (EXIT_SUCCESS);
}

/* cook_args: do the bulk of this program's work
 * argc is the count of files to proccess.
 * argv is an array of filenames to proccess.
 *
 * Returns -1 on error.
 */
static int
cook_args(int argc, const char *argv[])
{
	void *buf;
	const size_t buflen = BUFSIZ;
	int i;
	FILE *fp;
	int ret;
	int rval = -1;
	if ((buf = malloc(buflen)) == NULL)
		goto end;

	for (i = 0; i < argc; i++) {
		if ((fp = fopen(argv[i], "rb")) == NULL) {
			goto end;
		} else {
			ret = vis(fp, buf, buflen);
			fclose(fp);
			if (ret == -1)
				goto end;
		}
	}
	rval = 0;
end:
	free(buf);
	return (rval);
}

/* vis: vis fp to stdout
 * fp must be open and at least on mode "rb"
 * buf is a buffer to read into and can be of any buflen except 0
 *
 * Returns -1 on FILE error.
 */
static int
vis(FILE *fp, void *buf, size_t buflen)
{
	size_t n;

	for (;;) {
		n = fread(buf, sizeof(char), buflen, fp);
		if (n == 0) {
			if (ferror(fp))
				return (-1);
			if (feof(fp))
				return (0);
		}
		if (visbuf(stdout, buf, n) == -1)
			return (-1);
	}

}

/* visbuf: vis n bytes from _buf into out
 *
 * Returns -1 on error
 */
static int
visbuf(FILE *out, void *_buf, size_t n)
{
	size_t i;
	int clen;
	size_t written;
	unsigned char *buf = _buf;
	/* {'\\', 'X', h, h} where h are hex nibbles */
	char hex[4] = {'\\', 'X'};
	char *hnp = &hex[2]; /* pointer to where the hex nibbles in hex are */

	for (written = i = 0; i < n; i++) {
		if (isprint(buf[i]) || isspace(buf[i])) {
			clen = buf[i] == '\t' ? TABWIDTH : 1;
			if (fflagnl(out, &written, clen) == -1)
				return (-1);
			if (putc(buf[i], out) == EOF)
				return (-1);
		} else if (!dflag) {
			tohex(hnp, buf[i]);
			if (fflagnl(out, &written, sizeof(hex)) == -1)
				return (-1);
			if (fwrite(hex, sizeof(*hex), sizeof(hex), out) !=
			    sizeof(hex))
				return (-1);
		}
	}
	return (0);
}

/* tohex: makes byte into exactly 2 hexadeximal characters in arr */
static void
tohex(char arr[], unsigned char byte)
{
	sprintf(arr, "%.2x", (unsigned int)byte);
}

/* fflagnl: print the newline when fflag requires it
 * Returns -1 on error
 */
static int
fflagnl(FILE *out, size_t *written, size_t add)
{
	if (fflag) {
		*written += add;
		if (*written >= fflag) {
			*written = 0;
			if (putc('\n', out) == EOF)
				return (-1);
		}
	}
	return (0);
}
