#if !defined(H_MISC)
#define H_MISC

#include <sys/types.h>

ssize_t nwrite(int, const void *, size_t);
size_t getfdblksize(int);

#endif
