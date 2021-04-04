#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* How many numbers testqsort_sorted will compare */
#define SORTEDUINT32 (1 << 20)
/* How many numbers testqsort_rand will compare */
#define RANDUINT32 SORTEDUINT32
/* How many numbers testqsort_all0 will compare */
#define ALL0UINT32 SORTEDUINT32
/* The block size of the memory region to test */
#define RANDREGBLK 10240
/* How many blocks of random memory to test */
#define RANDREGCNT 1000

static int testqsort_sorted(struct timespec *);
static int testqsort(struct timespec *, void *, size_t, size_t,
    int (*compar)(const void *, const void *));
static int uint32cmp(const void *, const void *);
static void ts_sub(struct timespec *, const struct timespec *,
    const struct timespec *b);
static int presult(char [], struct timespec *);
static int testqsort_rand(struct timespec *);
static int testqsort_all0(struct timespec *);
static int testqsort_randreg(struct timespec *);
static int blkcmp(const void *, const void *);

/* This program benchmarks the system's implementation of qsort */
int
main(void)
{
	struct timespec ts;

	if (testqsort_sorted(&ts) == -1)
		goto err;
	if (presult("sorted uint32_t", &ts) == -1)
		goto err;
	if (testqsort_rand(&ts) == -1)
		goto err;
	if (presult("random uint32_t", &ts) == -1)
		goto err;
	if (testqsort_all0(&ts) == -1)
		goto err;
	if (presult("all-0-bits uint32_t", &ts) == -1)
		goto err;
	if (testqsort_randreg(&ts) == -1)
		goto err;
	if (presult("random region of memory", &ts) == -1)
		goto err;

	return (EXIT_SUCCESS);
err:
	perror("2-3");
	return (EXIT_FAILURE);
}

/* testqsort_sorted: benchmarks sorting an array of sorted uint32_t */
static int
testqsort_sorted(struct timespec *tsp)
{
	uint32_t *num;
	const size_t nmemb = SORTEDUINT32;
	size_t i;
	int rval = -1;
	if ((num = reallocarray(NULL, nmemb, sizeof(*num))) == NULL)
		goto end;

	for (i = 0; i < nmemb; i++)
		num[i] = i;

	if (testqsort(tsp, num, nmemb, sizeof(*num), uint32cmp) == -1)
		goto end;

	rval = 0;
end:
	free(num);
	return (rval);

}

/* testqsort: generic qsort benchmark function
 *
 * tsp stores how long sorting took
 * v is the vector to sort
 * nmemb is how many members are in the vector
 * size is the size of each member
 * compar is a compare function for comparing members of the vector
 *
 * Returns -1 on error.
 */
static int
testqsort(struct timespec *tsp, void *v, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
	struct timespec ts[2];

	if (clock_gettime(CLOCK_MONOTONIC, &ts[0]) == -1)
		goto err;
	qsort(v, nmemb, size, compar);
	if (clock_gettime(CLOCK_MONOTONIC, &ts[1]) == -1)
		goto err;

	ts_sub(tsp, &ts[1], &ts[0]);

	return (0);
err:
	return (-1);
}

/* uint32cmp: compare a and b, which are of type uint32_t, for qsort() */
static int
uint32cmp(const void *a, const void *b)
{
	const uint32_t *const anum = a;
	const uint32_t *const bnum = b;

	if (*anum == *bnum)
		return (0);
	else if (*anum < *bnum)
		return (-1);
	else
		return (1);
}

/* ts_sub: subtract a by b and store the resut in dst */
static void
ts_sub(struct timespec *dst, const struct timespec *a,
    const struct timespec *b)
{
	/* How many nanoseconds are in a second */
	const size_t NSEC_MAX = 1000000000;

	dst->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_nsec < b->tv_nsec) { /* borrow */
		dst->tv_sec--;
		dst->tv_nsec = NSEC_MAX - (b->tv_nsec - a->tv_nsec);
	} else {
		dst->tv_nsec = a->tv_nsec - b->tv_nsec;
	}
}

/* presult: print the result of a benchmark
 *
 * Returns -1 on error
 */
static int
presult(char testname[], struct timespec *tsp)
{
	int rval = -1;

	if (printf("Time taken by test %s:\n"
	    "%ld.%lds\n", testname, (long)tsp->tv_sec, (long)tsp->tv_nsec)
	    < 0)
		goto end;

	rval = 0;
end:
	return (rval);
}

/* testqsort_randuint: benchmarks sorting an array of random uint32
 *
 * Returns -1 on error */
static int
testqsort_rand(struct timespec *tsp)
{
	uint32_t *num;
	const size_t nmemb = RANDUINT32;
	int ret = -1;

	if ((num = reallocarray(NULL, nmemb, sizeof(*num))) == NULL)
		goto end;
	arc4random_buf(num, nmemb * sizeof(*num));
	testqsort(tsp, num, nmemb, sizeof(*num), uint32cmp);
	ret = 0;
end:
	free(num);
	return (ret);
}

/* testqsort_all0: benchmarks sorting an array of all-0 uint32 */
static int
testqsort_all0(struct timespec *tsp)
{
	uint32_t *num;
	const size_t nmemb = ALL0UINT32;
	int ret = -1;

	if ((num = reallocarray(NULL, nmemb, sizeof(*num))) == NULL)
		goto end;
	memset(num, 0, nmemb * sizeof(*num));
	testqsort(tsp, num, nmemb, sizeof(*num), uint32cmp);

	ret = 0;
end:
	free(num);
	return (ret);
}

/* testqsort_randreg: test a memory region filled with random bytes */
static int
testqsort_randreg(struct timespec *tsp)
{
	void *buf;
	const size_t nmemb = RANDREGCNT;
	const size_t size = RANDREGBLK;
	int ret = -1;

	if ((buf = reallocarray(NULL, nmemb, size)) == NULL)
		goto end;
	arc4random_buf(buf, nmemb * sizeof(buf));
	testqsort(tsp, buf, nmemb, size, blkcmp);

	ret = 0;
end:
	free(buf);
	return (ret);
}

/* blkcmp: memcmp regions a and b, which are of size RANDREGBLK, for qsort()*/
static int
blkcmp(const void *a, const void *b)
{
	const size_t size = RANDREGBLK;
	return (memcmp(a, b, size));
}
