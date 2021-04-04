#if !defined(H_CSVLIB)
#define H_CSVLIB

#include <stddef.h>

enum { /* settings for csv_setopt */
	CSV_SEP = 1 << 0,
};

/* This struct must NOT be touched by the user. */
struct csvstate;

/* All functions cannot receive null pointers unless otherwise stated. */
struct csvstate *csv_init(struct csvstate *);
void 		 csv_destroy(struct csvstate *);
size_t		 csv_nfield(struct csvstate *);
char 		*csv_getfield(struct csvstate *, size_t);

#endif /* !defined(H_CSVLIB) */
