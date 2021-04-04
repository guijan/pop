#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>

#include "misc.h"

static int addcrbuf(unsigned char **, size_t *, ssize_t *);

int
addcr(int dstfd, int srcfd)
{
	size_t rdblksize;
	size_t wrblksize;
	size_t buflen;
	unsigned char *buf;
	ssize_t nr;
	ssize_t rdlen;
	ssize_t wrlen;
	off_t off;
	int rval = -1;

	assert(dstfd != srcfd);
	rdblksize = getfdblksize(srcfd);
	wrblksize = getfdblksize(dstfd);
	rdlen = MAX(rdblksize, wrblksize);
	buflen = rdlen * 2;
	if ((buf = malloc(buflen)) == NULL)
		return (-1);

	for (;;) {
		nr = read(srcfd, buf, rdlen);
		if (nr == 0)
			break;
		else if (nr == -1)
			goto end;
		wrlen = nr;
		if (addcrbuf(&buf, &buflen, &wrlen) == -1)
			goto end;
		if (nwrite(dstfd, buf, wrlen) != wrlen)
			goto end;
	}

	if ((off = lseek(dstfd, 0, SEEK_CUR)) == -1) {
		/* It's okay if the fd can't be truncated. */
		if (errno != ESPIPE)
			goto end;
	} else {
		if (ftruncate(dstfd, off) == -1)
			goto end;
	}


	rval = 0;
end:
	free(buf);
	return (rval);
}

/* addcrbuf: add carriage returns to _buf
 * _ndef is the number of bytes currently defined in the buffer, _buflen is the
 * buffer's size.
 * The function uses its arguments in a read-modify-write manner. */
static int
addcrbuf(unsigned char **_buf, size_t *_buflen, ssize_t *_ndef)
{
	size_t i;
	unsigned char *buf = *_buf;
	size_t buflen = *_buflen;
	size_t ndef = *_ndef;
	void *tp;
	int ret = -1;

	for (i = 0; i < ndef; i++) {
		if (buf[i] != '\n')
			continue;

		ndef++;
		if (ndef > buflen) {
			buflen *= 2;
			if ((tp = realloc(buf, buflen)) == NULL)
				goto end;
			buf = tp;
		}
		memmove(buf + i + 1, buf + i, buflen - ndef - i - 1);
		i++;
	}
	ret = 0;
end:
	*_buf = buf;
	*_buflen = buflen;
	*_ndef = ndef;
	return (ret);
}
