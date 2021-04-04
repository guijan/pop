#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef int (*Compar)(const void *, const void *);

typedef struct gstackfram gstackfram;
typedef struct {
	size_t bufsize;		/* Size of a buffer. */
	gstackfram *top;	/* Top of the stack. */
} gstack;
struct gstackfram {
	gstackfram *prev;	/* Previous stack member. */
	void *buf; 		/* What gets pushed and popped. */
};

void rqsort(void *, size_t, size_t, Compar);
int iqsort(void *, size_t, size_t, Compar);
int ncmp(const void *, size_t, const void *, size_t, size_t, Compar);
void memswap(void *, size_t, void *, size_t, size_t);

gstack *stacknew(size_t);
int stackpush(gstack *, const void *);
void *stackpop(gstack *);
void stackdel(gstack *);

#if 0
/* iqsort: iterative generic quicksort */
int
iqsort(void *base, size_t nmemb, size_t size, Compar cmp)
{

}
#endif

/* rqsort: recursive generic quicksort */
void
rqsort(void *base, size_t nmemb, size_t size, Compar cmp)
{
	size_t i, last;
	if (nmemb <= 1)
		return;

	memswap(base, 0, base, nmemb >> 1, size);
	for (i = 1, last = 0; i < nmemb; i++)
		if (ncmp(base, i, base, 0, size, cmp) < 0)
			memswap(base, ++last, base, i, size);
	memswap(base, 0, base, last, size);
	rqsort(base, nmemb - last, size, cmp);
	rqsort((unsigned char *)base + (last + 1 * size), nmemb - last - 1,
	    size, cmp);
}

/* memswap: generic swap
 *
 * a is the element of the array abase to swap with array bbase's element b
 * size is the size of each element
 */
void
memswap(void *abase, size_t a, void *bbase, size_t b, size_t size)
{
	unsigned char *avec = &abase[a * size];
	unsigned char *bvec = &bbase[b * size];
	size_t i;
	unsigned char tmp;

	for (i = 0; i < size; i++) {
		if (avec[i] != bvec[i]) {
			tmp = avec[i];
			avec[i] = bvec[i];
			bvec[i] = tmp;
		}

}

/* ncmp: better Compar function
 * Makes functions usable with standard C's qsort() and bsearch() functions a
 * bit more natural.
 *
 * a is the element of the array abase to compare to array bbase's element b
 * size is the size of an element
 * cmp is a compare function, same as the one used with qsort() and bsearch()
 */
int
ncmp(const void *abase, size_t a, const void *bbase, size_t b, size_t size, Compar
    cmp)
{
	const unsigned char *const avec = abase;
	const unsigned char *const bvec = bbase;

	return (cmp(&avec[a * size], &bvec[b * size]));
}

/* stacknew: create a new generic stack for storing frames of size size
 *
 * Returns NULL with errno untouched on malloc failure.
 */
gstack *
stacknew(size_t size)
{
	gstack *sp;
	if ((sp = malloc(sizeof(*sp))) == NULL)
		return (NULL);

	sp->bufsize = size;
	sp->top = NULL;

	return (sp);
}

/* stackpush: push new value onto the stack
 *
 * Returns -1 with errno untouched on malloc failure.
 */
int
stackpush(gstack *sp, const void *base)
{
	gstackfram *fp, *tmpfp;
	assert(sp != NULL && base != NULL);
	if ((fp = malloc(sizeof(*fp))) == NULL)
		return (-1);
	if ((fp->buf = malloc(sp->bufsize)) == NULL) {
		free(fp);
		return (-1);
	}

	tmpfp = sp->top;
	sp->top = fp;
	fp->prev = tmpfp;

	memcpy(sp->top->buf, base, sp->bufsize);
	return (0);
}

/* stackpop: pop value from stack */
void *
stackpop(gstack *sp)
{
	void *buf;
	gstackfram *fp;
	if (sp == NULL || sp->top == NULL)
		return (NULL);

	fp = sp->top;
	sp->top = fp->prev;
	buf = fp->buf;

	free(fp);
	return (buf);
}

/* stackdel: free the stack and all the buffers and frames in it */
void
stackdel(gstack *sp)
{
	gstackfram *tfp;

	if (sp == NULL)
		return;
	while ((tfp = sp->top) != NULL) {
		sp->top = tfp->prev;
		free(tfp->buf);
		free(tfp);
	}
	free(sp);
}
