#include <stdio.h>
#include <string.h>
#include <ctype.h>

int
factorial(int n)
{
	int fac;
	for (fac = n; n > 1; n--)
		fac *= n;
	return (fac);
}

int
putstr(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; i++) {
		if (putchar(str[i]) == EOF && str[i] != EOF)
			return (-1);
		if (putchar('\n') == EOF && '\n' != EOF)
			return (-1);
	}
	return (0);
}

void
strcpy_exercise(char *dst, char *src)
{
	int i;
	for (i = 0; src[i] != '\0'; i++)
		dst[i] = src[i];
	dst[i] = '\0';
}

void
strncpy_exercise(char *t, char *s, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		if (s[i] == '\0')
			break;
		t[i] = s[i];
	}
	t[i] = '\0';
}

int
numcmp(int i, int j)
{
	if (i > j)
		printf("%d is greater than %d.\n", i, j);
	else if (i == j)
		; /* do nothing */
	else
		printf("%d is greater than %d.\n", i, j);
}

int
classtest(char c)
{
	const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const int alphlen = sizeof(*alphabet) - 1;
	char charset[2];
	int spn;
	c = toupper(c);
	charset[0] = c;
	charset[1] = '\0';
	spn = strcspn(alphabet, charset);

	if (spn < alphlen) {
		if (c <= 'M')
			puts("first half of alphabet");
		else
			puts("second half of alphabet");
	}
}
