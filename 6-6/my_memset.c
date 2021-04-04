#include <stdlib.h>

/* my memset: purposefully different memset */
void *
my_memset(void *b, int c, size_t len)
{
	unsigned char *p = b;
	while (len--)
		p[len] = c;
	return (b);
}
