#include <stdio.h>

/* csv_create: create a CSV out of an array of strings
 * A new record can be marked with a NULL pointer in the array. */
int
csv_create(FILE *out, char **arr, size_t arrlen, char sep)
{
	size_t i;
	int ret = -1;
	for (i = 0; i < arrlen; i++) {
		if (arr[i] == NULL) {
			if (putc(sep, out) == EOF)
				goto end;
			continue;
		}
		if (fprintf(out, "%s%c", arr[i], sep) < 0)
			goto end;
	}

	ret = (0);
end:
	if (putc(sep, out) == EOF)
		ret = -1;
	return (ret);
}
