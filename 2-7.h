#if !defined(H_2_7)
#define H_2_7
#include <stddef.h>

typedef struct List List;
struct List {
	void *datap;
	List *nextp;
};

/* Compar: the generic compare function used with the stdlib qsort() */
typedef int (Compar)(const void *, const void *);

List *listnew(List *listp, void *);
List *listgetnext(List *);
List *listsetnext(List *, List *);
List *listappend(List *, List *);
size_t listmergeat(List *, List *, size_t);
void *listgetdata(List *);
void *listsetdata(List *, void *);
void *listfinddata(List *, void *, Compar *);
size_t listlen(List *);
List *listreverse(List *);
void *listpopmemb(List *, size_t);


#endif /* !defined(H_2_7) */
