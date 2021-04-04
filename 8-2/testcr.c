#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include "addcr.h"
#include "misc.h"
#include "removecr.h"

#define ACNT(n) (sizeof(n) / sizeof(*n))

enum {
	POLLBAD = POLLERR|POLLHUP|POLLNVAL
};

static int forkfuncs(int, int);
static int testpipe(int, int);
static size_t memseqlen(void *, unsigned char, size_t);

int
main(void)
{
	int inpipe[2] = {-1, -1};
	int outpipe[2] = {-1, -1};
	int rval = EXIT_FAILURE;
	int ret;

	if (pipe(inpipe) == -1)
		goto end;
	if (pipe(outpipe) == -1)
		goto end;
	if (forkfuncs(outpipe[1], inpipe[0]) == -1)
		goto end;

	if ((ret = testpipe(outpipe[1], inpipe[0])) == -1) {
		goto end;
	} else if (ret > 0) {
		if (printf("failed\n") < 0)
			goto end;
	}

	rval = EXIT_SUCCESS;
end:
	close(inpipe[0]);
	close(inpipe[1]);
	close(outpipe[0]);
	close(outpipe[1]);
	return (rval);
}

/* forkfuncs: fork the functions we'll test
 * The output of the functions goes to fildes[0] and their input is fildes[1].
 *
 * Returns -1 on error.
 */
static int
forkfuncs(int outputfd, int inputfd)
{
	int funcpipe[2];
	pid_t addcrpid, removecrpid;
	int rval = -1;
	if (pipe(funcpipe) == -1)
		return (-1);

	if ((addcrpid = fork()) == -1)
		goto end;
	else if (addcrpid == 0) {
		addcr(inputfd, funcpipe[1]);
		goto end;
	}
	if ((removecrpid = fork()) == -1)
		goto end;
	else if (removecrpid == 0) {
		removecr(funcpipe[0], outputfd);
		goto end;
	}

	rval = 0;
end:
	close(funcpipe[0]);
	close(funcpipe[1]);
	return (rval);
}

/* testpipe: test addcr and removecr
 * Returns -1 on error.
 * Returns 0 on test success.
 * Returns >0 on test error.
 */
static int
testpipe(int dstfd, int srcfd)
{
	void *buf;
	size_t buflen;
	size_t srcblksize;
	size_t dstblksize;
	size_t targ;
	size_t ntested;
	struct pollfd fds[2];
	int i;
	ssize_t n;
	int ret = -1;
	srcblksize = getfdblksize(srcfd);
	dstblksize = getfdblksize(dstfd);
	buflen = MAX(srcblksize, dstblksize);
	targ = dstblksize * 3;
	if ((buf = malloc(buflen)) == NULL)
		return (-1);
	memset(buf, '\n', buflen);

	fds[0].fd = srcfd;
	fds[0].events = POLLOUT;
	fds[1].fd = dstfd;
	fds[1].events = POLLIN;

	for (ntested = 0; ntested < targ;) {
		if (poll(fds, ACNT(fds), 1) <= 0)
			goto end;
		for (i = 0; i < (int)ACNT(fds); i++) {
			if (fds[i].revents & POLLOUT) {
				n = write(fds[i].fd, buf, srcblksize);
				if (n == -1)
					goto end;
			} else if (fds[i].revents & (POLLIN|POLLHUP)) {
				n = read(fds[i].fd, buf, dstblksize);
				if (n == -1)
					goto end;
				else if (n == 0)
					break;
				if (memseqlen(buf, '\n', n) != n) {
					ret = 1;
					goto end;
				}
				ntested += n;
			} else if (fds[i].revents & (POLLERR|POLLNVAL)) {
				goto end;
			}
		}
	}

	ret = 0;
end:
	free(buf);
	return (ret);
}

/* memseqlen: return the length of the sequence of c starting in _buf */
static size_t
memseqlen(void *_buf, unsigned char c, size_t buflen)
{
	char *buf = _buf;
	size_t i;
	for (i = 0; i < buflen; i++)
		if (buf[i] != c)
			break;
	return (i);
}
