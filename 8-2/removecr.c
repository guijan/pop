#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

#include "removecr.h"
#include "misc.h"

static size_t removecrbuf(void *, size_t);

/* removecr: remove spurious carriage returns of srcfd and output to dstfd.
 * dstfd and srcfd can safely be the same file, but not the same fd.
 * dstfd must be writable, srcfd must be readable.
 * S_IFMT(dstfd) and S_IFMT(SRCFD) can be any.
 * Doesn't touch the offset.
 *
 * Returns -1 on error.
 */
int
removecr(int dstfd, int srcfd)
{
	size_t rdblksize;
	size_t wrblksize;
	size_t buflen;
	unsigned char *buf;
	ssize_t nr;
	ssize_t nw;
	off_t off;
	int rval = -1;

	assert(dstfd != srcfd);
	rdblksize = getfdblksize(srcfd);
	wrblksize = getfdblksize(dstfd);
	buflen = MAX(rdblksize, wrblksize);
	if ((buf = malloc(buflen)) == NULL)
		return (-1);

	for (;;) {
		nr = read(srcfd, buf, buflen);
		if (nr == 0)
			break;
		else if (nr == -1)
			goto end;

		nw = removecrbuf(buf, nr);
		if ((nwrite(dstfd, buf, nw)) != nw)
			goto end;
	}
	if ((off = lseek(dstfd, 0, SEEK_CUR)) == -1) {
		if (errno != ESPIPE)
			goto end;
	} else {
		/* It's okay if the fd can't be truncated */
		if (ftruncate(dstfd, off) == -1 && errno != EINVAL)
			goto end;
	}
	rval = 0;
end:
	free(buf);
	return (rval);
}

/* removecrbuf: remove carriage returns from buffer
 * Returns the new length of the buffer's bytes.
 */
static size_t
removecrbuf(void *_buf, size_t buflen)
{
	unsigned char *buf = _buf;
	size_t i;

	i = 0;
	if (buflen == 1 && buf[i] == '\r')
		return (0);
	while (i < buflen) {
		if (buf[i] == '\r')
			memmove(buf + i, buf + 1 + i, (buflen -= 1) - i);
		else
			i++;
	}
	return (i);
}
