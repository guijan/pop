#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if !defined(SIZE_MAX)
	#define SIZE_MAX ((size_t)-1)
#endif

size_t lflag = 6;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int cook_args(int, char **);
static int strings(char *, FILE *, char *, size_t);
static int pstrings(unsigned char *, size_t, FILE *, char *);

int
main(int argc, char *argv[])
{
	int c;
	const char *err;
	extern char *optarg;

	while ((c = getopt(argc, argv, "l:")) != -1) {
		switch(c) {
		case 'l':
			lflag = strtonum(optarg, 0, MIN(LLONG_MAX, SIZE_MAX),
			    &err);
			if (err != NULL)
				fprintf(stderr, "%s\n", err);
			break;
		default:
			printf("usage: strings filenames");
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (cook_args(argc, argv) == -1)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

/* cook_args: do the main work of the program
 * Returns -1 on error
 */
static int
cook_args(int argc, char *argv[])
{
	char *buf;
	size_t buflen = lflag;
	int i;
	FILE *fin;
	int ret;
	if ((buf = malloc(buflen)) == NULL)
		return (-1);
	for (i = 0; i < argc; i++) {
		if ((fin = fopen(argv[i], "rb")) == NULL) {
			printf("can't open %s:", argv[i]);
		} else {
			ret = strings(argv[i], fin, buf, buflen);
			fclose(fin);
			if (ret == -1)
				goto end;
		}
	}
	ret = 0;
end:
	free(buf);
	return (ret);
}

/* strings: extract printable strings from stream */
static int
strings(char *filename, FILE *fp, char *buf, size_t buflen)
{
	int c;

	for (;;) {
		c = getc(fp);
		if (c == EOF) {
			if (ferror(fp))
				return (-1);
			else if (feof(fp))
				return (0);
		}
		if (isprint(c)) {
			ungetc(c, fp);
			pstrings(buf, buflen, fp, filename);
		}
	}
}

static int
pstrings(unsigned char *buf, size_t buflen, FILE *fp, char *filename)
{
	size_t i;
	int c;
	int ret = -1;

	for (i = 0;;) {
		c = getc(fp);
		if (!isprint(c))
			goto end;
		i++;
		if (c == EOF) {
			if (ferror(fp))
				goto end;
			if (feof(fp))
				break;
		}
		if (i < buflen) {
			buf[i] = c;
		} else if (i == buflen) {
			printf("%s: ", filename);
			fwrite(buf, sizeof(*buf), buflen, stdout);
			putchar(c);
		} else {
			putchar(c);
		}
	}
	ret = 0;
end:
	if (i >= buflen)
		putchar('\n');
	return (ret);
}
