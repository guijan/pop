#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_memset.h"

#define ACNT(arr) (sizeof((arr)) / sizeof((*arr)))

/* Buffer sizes to test. */
const static size_t sizes[] = {0, 1, 2, 65537, 65535, 65534, 32768, 35, 95};
/* Bytes to be set and tested, signed char. */
const static signed char bytessc[] = {0xFF, 0x7F, 0x00, 0x32, 0x95, 0xF0, 0x8D};
/* bytes to be set and tested, unsigned char. */
const static unsigned char bytesuc[] = {0xFF, 0x00, 0x7F, 0x8F, 0x22, 0xAD};

const char *const progname = "my_memset";

/* Sizes for the extra space before and after the buffer. No particular
 * reasoning behind the exact values. */
enum {
	PREFIX = 512,
	SUFFIX = 512
};

static int test_my_memset(size_t, int);
static int bytecmp(const void *, unsigned char, size_t);
static int p_memsetprob(const char *, const void *, const void *, int, size_t);
static int p_memsetargs(const void *, const void *, int, size_t);
int
main(void)
{
	size_t i, j;
	for (i = 0; i < ACNT(sizes); i++) {
		for (j = 0; j < ACNT(bytessc); j++)
			if (test_my_memset(sizes[i], bytessc[j]) == -1)
				goto err;
		for (j = 0; j < ACNT(bytesuc); j++)
			if (test_my_memset(sizes[i], bytesuc[j]) == -1)
				goto err;
	}

	return (EXIT_SUCCESS);
err:
	perror(progname);
	return (EXIT_FAILURE);
}

static int
test_my_memset(size_t size, int byte)
{
	enum {cnt = 2};
	int i;
	unsigned char *buf[cnt] = {0};
	size_t len = PREFIX + size + SUFFIX;
	void *(*set[cnt])(void *, int, size_t) = {my_memset, memset};
	void *ret[cnt];
	int rval = -1;
	unsigned char inverse = UCHAR_MAX ^ (unsigned char)byte;
	for (i = 0; i < cnt; i++)
		if ((buf[i] = malloc(len)) == NULL)
			goto end;
	for (i = 0; i < cnt; i++)
		memset(buf[i], inverse, len);
	for (i = 0; i < cnt; i++)
		ret[i] = set[i](buf[i] + PREFIX, byte, size);
	/*
	for (i = 0; i < cnt; i++)
		if (ret[i] != (void *)buf[i])
			if (p_memsetprob("pointers don't match", ret[i], buf,
			    byte, len) == -1)
				goto end;
	*/
	for (i = 0; i < cnt; i++)
		if (bytecmp(buf[i], inverse, PREFIX))
			if (p_memsetprob("underruns buffer", ret[i], buf, byte,
			    len) == -1)
				goto end;
	for (i = 0; i < cnt; i++)
		if (bytecmp(buf[i] + PREFIX + size, inverse, SUFFIX))
			if (p_memsetprob("overruns buffer", ret[i], buf, byte,
			    len) == -1)
				goto end;
	rval = 0;
end:
	for (i = 0; i < cnt; i++)
		free(buf[i]);
	return (rval);
}

static int
bytecmp(const void *buf, unsigned char c, size_t len)
{
	size_t i;
	const unsigned char *p = buf;
	for (i = 0; i < len; i++)
		if (p[i] != c)
			return (p[i] - c);
	return (0);
}

static int
p_memsetprob(const char *msg, const void *retbuf, const void *buf, int byte,
    size_t len)
{
	if (printf("%s: ", progname) < 0)
		return (-1);
	if (p_memsetargs(retbuf, buf, byte, len) == -1)
		return (-1);
	if (printf(" and %s\n", msg) < 0)
		return (-1);
	return (0);
}

/* p_memsetargs: print memset args, retbuf is memset's return value */
static int
p_memsetargs(const void *retbuf, const void *buf, int byte, size_t len)
{
	if (printf("memset(%p, %d, %lu) returns %p", buf, byte,
	    (unsigned long)len, retbuf) < 0)
		return (-1);
	return (0);
}
