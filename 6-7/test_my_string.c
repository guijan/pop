#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_string.h"

#define ACNT(arr) (sizeof(arr) / sizeof(*arr))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TEST_MEMCM(func) test_memcm(#func, func)

struct funcnname {
	const char *name;
	void *(*memcm)(void *, const void *, size_t);
};

static int test_memcm(const char *name, void *(memcm)(void *,
    const void *, size_t));
static int real_test_memcm(const struct funcnname *, size_t);
static int test_memcm_uorun(const struct funcnname *, const void *, size_t,
    size_t, size_t, unsigned char);
static int test_memcm_propercpy(const struct funcnname *, const void *, size_t,
    size_t, size_t, unsigned char);
static int test_memcmp(void);
static int test_memset(void);

int
main(void)
{

	if (TEST_MEMCM(my_memcpy) == -1)
		goto err;
	if (TEST_MEMCM(my_memmove) == -1)
		goto err;
	if (test_memcmp() == -1)
		goto err;
	if (test_memset() == -1)
		goto err;

	return (EXIT_SUCCESS);
err:
	perror("test_my_string");
	return (EXIT_FAILURE);
}

/* test_memcm: backend for test_memcm macro.
 * Call test_memcm(memcm) instead.
 *
 * Tests memcpy or memmove equivalent and prints out any errors.
 * return -1 if a syscall fails.
 */
static int
test_memcm(const char *name, void *(memcm)(void *, const void *, size_t))
{
	struct funcnname fn;
	int i;
	size_t lens[] = {0, 1, 2, UCHAR_MAX, UCHAR_MAX << 4, 95};
	fn.name = name;
	fn.memcm = memcm;
	for (i = 0; i < ACNT(lens); i++)
		if (real_test_memcm(&fn, lens[i]) == -1)
			return (-1);
	return (0);
}

/* real_test_memcm: suite of tests for memmove/memcpy equivalent inside fn.
 * Tests a buffer size len.
 *
 * return -1 on syscal error
 */
static int
real_test_memcm(const struct funcnname *fn, size_t len)
{
	int i;
	void *buf[2] = {0};
	enum {
		PREFIX = 512,
		SUFFIX = 512
	};
	int ret = -1;
	for (i = 0; i < ACNT(buf); i++)
		if ((buf[i] = malloc(PREFIX + len + SUFFIX)) == NULL)
			goto end;
	memset(buf[1], 0, PREFIX);
	memset(buf[1] + PREFIX, 0xFF, len);
	memset(buf[1] + PREFIX + len, 0, SUFFIX);
	fn->memcm(buf[0] + PREFIX, buf[1] + PREFIX, len);
	if (test_memcm_uorun(fn, buf, PREFIX, len, SUFFIX, 0) == -1)
		goto end;
	if (test_memcm_propercpy(fn, buf, PREFIX, len, SUFFIX, 0xFF) == -1)
		goto end;
	ret = 0;
end:
	for (i = 0; i < ACNT(buf); i++)
		free(buf[i]);
	return (ret);

}

/* test_memcm_uorun: test function in fn for buffer underruns or overruns.
 *
 * return -1 if a syscall fails. */
static int
test_memcm_uorun(const struct funcnname *fn, const void *buf, size_t pref,
    size_t len, size_t suf, unsigned char byte)
{
	const unsigned char *p;
	size_t i;
	for (p = buf, i = 0; i < pref; i++)
		if (p[i] != byte)
			if (printf("%s underruns buffer with len %lu\n",
		    		fn->name, (unsigned long)len) < 0)
					return (-1);
	for (p = (char *)buf + pref + len; i < suf; i++)
		if (p[i] != byte)
			if (printf("%s  overruns buffer with len %lu\n",
		    		fn->name, (unsigned long)len) < 0)
					return (-1);
	return (0);
}

/* test_memcm_propercpy: test that memcpy/memmove equivalent properly copies.
 *
 * return -1 if a syscall fails. */
static int
test_memcm_propercpy(const struct funcnname *fn, const void *buf, size_t pref,
    size_t len, size_t suf, unsigned char byte)
{
	const unsigned char *p;
	size_t i;

	for (p = (char *)buf + pref + len, i = 0; i < suf; i++)
		if (p[i] != byte)
			if (printf("%s improperly copied 0-indexed byte %lu "
			    "with len %lu\n",
			    fn->name, (unsigned long)i, (unsigned long)len) < 0)
				return (-1);
	return (0);
}

static int
test_memcmp(void)
{
	int aret, bret;
	int i, j;
	unsigned char strings[][8] = {
		"dogs",
		"dogs",
		"cats",
		"thox",
		"\xff\xf0\x0f\x00",
		"\xff\xf0\x0f\x00",
		"\x00\x0f\xf0\xff",
		"\xab\xcd\xef\x01",
		"\x82\x33\x92\x2f",
		"\xb7\xc7\x24\x0e"
	};
	for (i = 0; i < ACNT(strings); i++) {
		for (j = 0; i + j < ACNT(strings); j++) {
			aret = my_memcmp(strings[i], strings[i + j],
			    MIN(sizeof(strings[i]), sizeof(strings[i + j])));
			bret = memcmp(strings[i], strings[i + j],
			    MIN(sizeof(strings[i]), sizeof(strings[i + j])));
			if ((aret < 0 && bret < 0) || (aret == 0 && bret == 0)
			    || (aret > 0 && bret > 0))
				continue;
			if (printf("memcmp failed between strings \"%s\" "
			    "and \"%s\", aret = %d, bret = %d", strings[i],
			    strings[i + j], aret, bret) < 0)
				return (-1);
		}
	}
	return (0);
}

static int
test_memset(void)
{
	size_t i, j;
	size_t sizes[] = {512, 1024, 0, 1, 32768, 95, 55, 7, 13, 17, 665};
	enum {
		SUFFIX = 512,
		PREFIX = 512,
	};
	int chars[] = {INT_MAX, INT_MIN, 0, UCHAR_MAX, UCHAR_MAX + 1, 65535};
	int inverse;
	unsigned char *buf;
	int ret = -1;

	for (i = 0; i < ACNT(sizes); i++) {
		for (j = 0; j < ACNT(chars); j++) {
			if ((buf = calloc(PREFIX + sizes[i] + SUFFIX, 1))
			    == NULL)
				goto end;
			inverse = ~chars[j];
			memset(buf, inverse, PREFIX + sizes[i] + SUFFIX);
			memset(buf + PREFIX, chars[j], sizes[i]);
			if (buf[PREFIX - 1] == chars[j]
			    || buf[sizes[i] + SUFFIX] == chars[j])
				if (printf("memset buffer overrun/underrun for"
				    "size %lu\n char argument %d",
				    (unsigned long)sizes[i], chars[j]) < 0)
					goto end;
			free(buf);
		}
	}

	ret = (-1);
end:
	free(buf);
	return (ret);
}

static int
test_memchr(void)
{
	char *strings[3] = {
		"test string",
		"there was an attempt",
		"\xFF\xFF\xFF\x01\x01\x01\xF0\x0F"
	};
	if (my_memchr(strings[0], 's', strlen(strings[0]) + 1)
	    != &strings[0][2])
		if (printf("memchr failed to find %c in string \"%s\"",
		    strings[0][2], strings[0]) < 0)
			return (-1);
	if (my_memchr(strings[1], 'w', strlen(strings[0]) + 1)
	    != &strings[1][6])
		if (printf("memchr failed to find %c in string \"%s\"",
		    strings[1][6], strings[1]) < 0)
			return (-1);
	if (my_memchr(strings[2], '\xFF', strlen(strings[0]) + 1)
	    != &strings[2][0])
		if (printf("memchr failed to find %c in string \"%s\"",
		    strings[2][0], strings[2]) < 0)
			return (-1);

	return (0);
}
