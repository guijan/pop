#if !defined(H_MY_STRING)
#define H_MY_STRING
#include <stddef.h>

void *my_memcpy(void *, const void *, size_t);
void *my_memmove(void *, const void *, size_t);
int my_memcmp(const void *, const void *, size_t);
void *my_memchr(const void *, int, size_t);
void *my_memset(void *, int, size_t);
#endif
