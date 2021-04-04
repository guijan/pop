#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Btree Btree;
struct Btree {
	void *datap;
	Btree *leftp;
	Btree *rightp;
};

/* Compar: the comparison function you'd use with qsort() from the stdlib */
typedef int (Compar)(const void *, const void *);
/* ApplyFunc: function to apply to whole tree with btapply()
 * Makes btapply stop and return if it returns true.
 * Will receive the tree's node as its first argument
 */
typedef int (ApplyFunc)(Btree *, void *);

struct btmemsprint_arg {
	void *base;
	size_t size;
	size_t i;
};

Btree *btnew(Btree *, void *);
Btree *btadd(Btree *, Btree *, Compar *);
void *btgetdata(const Btree *);
void *btsetdata(Btree *, void *);
Btree *btlookup(const Btree *, const void *, Compar *);
int btapply(Btree *, ApplyFunc *, void *);
int btfree(Btree *, void *);
int btmemsprint(Btree *, void *);
void btsort(void *, size_t, size_t, Compar *);

/* btnew: new binary tree node holding datap
 * If treep is NULL, allocate it, otherwise use the buffer it points to.
 *
 * Returns NULL on allocation error.
 */
Btree *
btnew(Btree *treep, void *datap)
{
	if (treep == NULL)
		if ((treep = malloc(sizeof(*treep))) == NULL)
			return (NULL);
	treep->datap = datap;
	treep->leftp = NULL;
	treep->rightp = NULL;
	return (treep);
}

/* btadd: add btree node src to dst, return root node */
Btree *
btadd(Btree *dst, Btree *src, Compar *cmp)
{
	int ret;

	if (dst == NULL)
		return (src);
	ret = cmp(btgetdata(dst), btgetdata(src));
	if (ret < 0)
		dst->leftp = btadd(dst->leftp, src, cmp);
	else if (ret > 0)
		dst->rightp = btadd(dst->rightp, src, cmp);

	return (dst);
}

/* btgetdata: get data in binary tree node */
void *
btgetdata(const Btree *treep)
{
	return (treep == NULL ? NULL : treep->datap);
}

/* btsetdata: set data in binary tree node, return old data */
void *
btsetdata(Btree *treep, void *datap)
{
	void *oldp;
	if (treep == NULL)
		return (NULL);
	oldp = btgetdata(treep);
	treep->datap = datap;
	return (oldp);
}

/* btlookup: return binary tree node whose data compares true to datap
 *
 * Returns NULL if not found.
 */
Btree *
btlookup(const Btree *treep, const void *datap, Compar *cmp)
{
	int ret;

	if (treep == NULL)
		return (NULL);

	ret = cmp(btgetdata(treep), datap);
	if (ret < 0)
		treep = treep->leftp;
	else if (ret == 0)
		return ((Btree *)treep);
	else
		treep = treep->rightp;

	return (btlookup(treep, datap, cmp));
}

/* btapply: apply function to entire tree
 * Stops and returns the function's return value if the function returns true.
 */
int
btapply(Btree *treep, ApplyFunc *fn, void *fnarg)
{
	int ret = 0;
	Btree *leftp;
	Btree *rightp;
	if (treep == NULL)
		goto end;
	leftp = treep->leftp;
	rightp = treep->rightp;
	if ((ret = btapply(leftp, fn, fnarg)))
		goto end;
	if ((ret = fn(treep, fnarg)))
		goto end;
	if ((ret = btapply(rightp, fn, fnarg)))
		goto end;
end:
	return (ret);
}

/* btfree: free binary tree using btapply */
int
btfree(Btree *treep, void *arg /* unused */)
{
	free(btgetdata(treep));
	free(treep);
	return (0);
}
/* btmemsprint: ApplyFunc for sorting an array */
int
btmemsprint(Btree *treep, void *_args)
{
	const void *const datap = btgetdata(treep);
	struct btmemsprint_arg *args = _args;

	memcpy((char *)args->base + args->i * args->size, datap, args->size);
	args->i++;
	return (0);
}

void
btsort(void *_base, size_t nmemb, size_t size, Compar *cmp)
{
	unsigned char *const base = _base;
	size_t i;
	Btree *treep;
	Btree *tp;
	void *datap;
	struct btmemsprint_arg memparg;

	treep = btnew(NULL, base);
	assert(treep != NULL);
	for (i = 1; i < nmemb; i++) {
		datap = malloc(size);
		assert(datap != NULL);
		memcpy(datap, base + i * size, size);
		tp = btnew(NULL, datap);
		assert(tp != NULL);
		btadd(treep, tp, cmp);
	}
	memparg.base = _base;
	memparg.size = size;
	memparg.i = 0;
	btapply(treep, btmemsprint, &memparg);
	btapply(treep, btfree, NULL);
}
