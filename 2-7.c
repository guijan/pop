#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "2-7.h"

/* Datafunc: The first argument is List's data, the second is freeform
 * Used with listapply() to run a function on the entire list.
 *
 * listapply() keeps iterating over the list as long as it returns > 0.
 * listapply() stops and passes through a <= 0 return value.
 */
typedef int (Datafunc)(void *, void *);

/* Datafree: free List data */
typedef void (Datafree)(void *);

/* listnew: new generic list
 *
 * If listp is NULL, allocate it. Otherwise use the provided buffer.
 */
List *
listnew(List *listp, void *datap)
{
	if (listp == NULL)
		if ((listp = malloc(sizeof(*listp))) == NULL)
			return (NULL);

	listp->nextp = NULL;
	listp->datap = datap;
	return (listp);
}

/* listgetnext: get next member of list */
List *
listgetnext(List *listp)
{
	return (listp == NULL ? NULL : listp->nextp);
}

/* listsetnext: set next member of list
 *
 * Returns the old next member of the list, which could be NULL, or NULL
 * if the destination list is NULL
 */
List *
listsetnext(List *dst, List *src)
{
	List *retp;
	if (dst == NULL)
		return (NULL);

	retp = listgetnext(dst);
	dst->nextp = src;
	return (retp);
}

/* listappend: append src to dst, return list's new last member */
List *
listappend(List *dst, List *src)
{
	while (listgetnext(dst) != NULL)
		dst = listgetnext(dst);
	listsetnext(dst, src);
	while (listgetnext(dst) != NULL)
		dst = listgetnext(dst);

	return (dst);
}

/* listmergeat: merge src at dst member pos
 *
 * Returns the position src was merged at.
 */
size_t
listmergeat(List *dst, List *src, size_t pos)
{
	size_t i;
	List *nextp;

	for (i = 0; i < pos; i++) {
		if ((nextp = listgetnext(dst)) == NULL)
			break;
		dst = nextp;
	}
	nextp = listsetnext(dst, src);
	while (listgetnext(dst) != NULL)
		dst = listgetnext(dst);
	listsetnext(dst, nextp);
	return (i);
}

/* listgetdata: get data of listp */
void *
listgetdata(List *listp)
{
	return (listp == NULL ? NULL : listp->datap);
}

/* listsetdata: set data of listp, return old data */
void *
listsetdata(List *listp, void *datap)
{
	void *retp;
	if (listp == NULL)
		return (NULL);

	retp = listgetdata(listp);
	listp->datap = datap;
	return (retp);
}

/* listfinddata: find data in listp that matches datap according to cmp
 *
 * Returns NULL if not found.
 */
void *
listfinddata(List *listp, void *datap, Compar *cmp)
{
	void *retp;

	for (; listp != NULL; listp = listgetnext(listp))
		if (cmp((retp = listgetdata(listp)), datap) == 0)
			return (retp);

	return (NULL);
}

/* listlen: return the length of the list */
size_t
listlen(List *listp)
{
	size_t i;

	for (i = 0; listp != NULL; i++)
		listp = listgetnext(listp);

	return (i);
}

/* listreverse: reverse list and return the new first member */
List *
listreverse(List *listp)
{
	List *prevp;
	List *nextp;

	for (prevp = NULL; listp != NULL; listp = nextp) {
		nextp = listsetnext(listp, prevp);
		prevp = listp;
	}

	return (prevp);
}

/* listpopmemb: delete 1-indexed member of list, return the data it held
 *
 * Returns NULL if the data or the list are NULL, or if the member doesn't
 * exist.
 * The behavior is undefined if pos is 0.
 */
void *
listpopmemb(List *listp, size_t pos)
{
	size_t i;
	List *prevp;
	void *retp;

	if (pos == 0)
		return (NULL);
	for (i = 0; i < pos; i++) {
		prevp = listp;
		listp = listgetnext(listp);
		if (listp == NULL)
			return (NULL);
	}
	retp = listgetdata(prevp);
	/*
	 * Using memcpy instead of assigning pointers allows changing the
	 * start of the list.
	 */
	memcpy(prevp, listp, sizeof(*prevp));
	free(listp);
	return (retp);
}
