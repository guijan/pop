#include <sys/param.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include "misc.h"

/* nwrite: reentrant nwrite, return value != nbytes on error. */
ssize_t
nwrite(int fd, const void *_buf, size_t nbytes)
{
	const char *buf = _buf;
	size_t off;
	ssize_t nw;

	for (off = 0; off < nbytes; off += nw) {
		nw = write(fd, buf + off, nbytes - off);
		if (nw == 0 || nw == -1)
			break;
	}
	return (off);
}

/* getfdblksize: get optimal i/o size for fd */
size_t
getfdblksize(int fd)
{
	struct stat sb;

	if (fstat(fd, &sb) == -1 || sb.st_blksize <= 0)
		return (BUFSIZ);
	return (sb.st_blksize);
}
