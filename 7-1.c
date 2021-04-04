#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	NPAT = 1024,
	NSTART = 1024
};

int patlen[NPAT];			/* length of pattern */
int starting[UCHAR_MAX+1][UCHAR_MAX+1][NSTART];	/* pats starting with char */
int nstarting[UCHAR_MAX+1 * UCHAR_MAX+1]; /* number of such patterns */

int
isspam(char *mesg)
{
	int i, j, k;
	unsigned char c;

	for (j = 0; (c = mesg[j]) != '\0'; j++) {
		for (i = 0; i < nstarting[c]; i++) {
				k = starting[c][i];
				if (memcmp(mesg+j, pat[k], patlen[k]) == 0) {
					printf("spam: match for '%s'\n",
					    pat[k]);
					return (1);
				}
		}
	}
	return (0);
}
