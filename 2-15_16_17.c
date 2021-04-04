#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NHASH 512

enum {
	OPTMULT = 37, /* optimal number to multiply with in a string hash */
	DEFSIZE = BUFSIZ, /* default size of table */
	GROWBY = 1 /* tables will grow by 1 << GROWBY times when necessary */
};

/* nvhash is an unsigned type <= sizeof(size_t) and holds a hash */
typedef unsigned int nvhash;
typedef struct Nameval Nameval;
struct Nameval {
	const char *name;
	int value;
	Nameval *next;
};
typedef struct Nvtable {
	size_t nmemb; /* size of Nameval array */
	size_t lists; /* number of Namevals inside the array */
	Nameval **tab; /* Nameval array, the hash table */
} NvTable;

int nvgetval(Nameval *);
Nameval *nvnew(void *, const char *, int);
Nameval *nvlookup(NvTable **, const char *, int, Nameval *, int);
static NvTable *nvtnew(NvTable *);
static NvTable *growtable(NvTable *);
static nvhash namehash(NvTable *, const char *);
Nameval *nvappend(Nameval *, Nameval *);

/* nvgetval: get the value inside a Nameval */
int
nvgetval(Nameval *nvp)
{
	return (nvp->value);
}

/* nvnew: Create new Nameval */
Nameval *
nvnew(void *buf, const char *name, int value)
{
	Nameval *nvp = buf;
	if (nvp == NULL)
		if ((nvp = malloc(sizeof(*nvp))) == NULL)
			return (NULL);
	nvp->name = name;
	nvp->value = value;
	nvp->next = NULL;
	return (nvp);
}

/* nvlookup: look name up in Nameval hash table, or insert name
 * if create is true, add new Nameval entry inside buf containing name and
 * value. buf and/or *table can be NULL, in which case they'll be allocated.
 * Returns NULL if the entry is not found or allocation for insertion fails.
 */
Nameval *
nvlookup(NvTable **table, const char *name, int create, Nameval *buf, int value)
{
	nvhash hash;
	Nameval *p, *prev;
	NvTable *tp;
	if (*table == NULL)
		if ((*table = nvtnew(NULL)) == NULL)
			return (NULL);
	tp = *table;
	hash = namehash(*table, name);
	for (prev = p = tp->tab[hash]; p != NULL; prev = p, p = p->next)
		if (strcmp(name, p->name) == 0)
			return (p);
	if (create) {
		p = nvnew(buf, name, value);
		if (prev == NULL) {
			tp->tab[hash] = p;
			tp->lists++;
			if (tp->lists > tp->nmemb >> GROWBY)
				if ((tp = growtable(tp)) != NULL)
					*table = tp;
		} else {
			prev->next = p;
		}
		return (p);
	}

	return (NULL);
}

/* nvtnew: create new NvTable, allocate it if tp is NULL.
 * Return null on malloc error.
 */
static NvTable *
nvtnew(NvTable *tp)
{
	const size_t tablen = sizeof(*tp->tab) * DEFSIZE;
	if (tp == NULL)
		if ((tp = malloc(sizeof(*tp))) == NULL)
			return (NULL);
	if ((tp->tab = malloc(tablen)) == NULL)
		goto err;
	tp->nmemb = DEFSIZE;
	tp->lists = 0;
	memset(tp->tab, 0, tablen);
	return (tp);
err:
	free(tp);
	return (NULL);
}

/* growtable: grow NvTable by 1 << GROWBY
 * Returns NULL on realloc failure.
 */
static NvTable *
growtable(NvTable *table)
{
	size_t i;
	Nameval *listbkp, *tmplist;
	const size_t nmemb = table->nmemb << GROWBY;
	const size_t len = nmemb * sizeof(*table->tab);
	void *tmp;

	if ((tmp = realloc(table->tab, len)) == NULL)
		return (NULL);
	table->tab = tmp;
	listbkp = NULL;
	for (i = 0; i < table->nmemb; i++)
		listbkp = nvappend(listbkp, table->tab[i]);
	table->nmemb = nmemb;
	memset(table->tab, 0, len);
	for (; listbkp != NULL; listbkp = tmplist) {
		tmplist = listbkp->next;
		listbkp->next = NULL;
		nvlookup(&table, listbkp->name, 1, listbkp, listbkp->value);
	}
	return (table);
}

/* nvseqaccess: access next nameval in hash table
 * if prev is NULL, return first element. Otherwise return the element after
 * prev.
 * Returns NULL when there is no next element.
 */
Nameval *
nvseqaccess(NvTable *table, Nameval *prev)
{
	size_t i;
	if (prev == NULL)
		i = 0;
	else if (prev->next == NULL)
		i = namehash(table, prev->name);
	else
		return (prev->next);
	if (i >= table->nmemb)
		return (NULL);
	for (i++; table->tab[i] == NULL; i++)
		if (i >= table->nmemb)
			return (NULL);
	return (table->tab[i]);
}

/* namehash: hash name for tab */
static nvhash
namehash(NvTable *tab, const char *name)
{
	nvhash h;
	for (h = 0; *name != '\0'; name++)
		h *= h * OPTMULT + *name;
	return (h % tab->nmemb);
}

/* nvappend: append nameval lists, return the base list */
Nameval *
nvappend(Nameval *dst, Nameval *src)
{
	Nameval **target;
	for (target = &dst; *target != NULL; target = &(*target)->next)
		;
	*target = src;
	return (dst);
}
