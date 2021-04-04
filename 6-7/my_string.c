#include <stddef.h>
#include <stdlib.h>

#include "my_string.h"

void *
my_memcpy(void *_dst, const void *_src, size_t len)
{
	unsigned char *dst = _dst;
	const unsigned char *src = _src;
	size_t i;

	for (i = 0; i < len; i++)
		dst[i] = src[i];
	return (_dst);
}

void *
my_memmove(void *dst, const void *src, size_t len)
{
	void *buf;
	if ((buf = malloc(len)) == NULL)
		abort();

	my_memcpy(buf, src, len);
	my_memcpy(dst, buf, len);
	free(buf);
	return (dst);
}

int
my_memcmp(const void *_a, const void *_b, size_t len)
{
	const unsigned char *a = _a;
	const unsigned char *b = _b;
	size_t i;

	for (i = 0; i < len; i++)
		if (a[i] != b[i])
			break;
	return (a - b);
}

void *
my_memset(void *_buf, int _c, size_t len)
{
	size_t i;
	unsigned char *buf = _buf;
	unsigned char c = _c;

	for (i = 0; i < len; i++)
		buf[i] = c;
	return (_buf);
}

void *
my_memchr(const void *_b, int c, size_t len)
{
	const unsigned char *b = _b;

	for (; *b != '\0'; b++)
		if (*b == c)
			return ((void *)b);
	return (NULL);
}
