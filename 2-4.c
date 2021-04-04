#include <stddef.h>

static void memswap(void *, size_t, void *, size_t, size_t);
void badsort(void *, size_t, size_t, int (*)(const void *, const void *));
/* badsort: purposefully terrible sort
 *
 * Same interface as the stdlib qsort()
 * O(pow(2,n))
 */
void
badsort(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
	size_t i, j;
	size_t smallest;
	unsigned char *vec = base;

	for (i = 0; i < nmemb; i++) {
		smallest = i;
		for (j = 0; j < nmemb; j++) {
			if (compar(&vec[j], &vec[smallest]) < 0)
				smallest = j;
		}
		memswap(base, i, base, smallest, size);
	}
}

/* memswap: generic swap
 *
 * a is the element of the array abase to swap with array bbase's element b.
 * size is the size of each element.
 * The arrays may overlap.
 */
static void
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
}
