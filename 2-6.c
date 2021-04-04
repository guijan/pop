#include <stdlib.h>
#include <string.h>

typedef struct Nameval Nameval;
struct Nameval {
	char *name;
	int value;
};

struct NVtab {
	int nval;		/* Current number of values. */
	int max;		/* Allocated number of values. */
	Nameval *nameval;	/* Array of name-value pairs */
} nvtab;

enum {
	NVINIT = 1,
	NVGROW = 2
};

/* addname: add new name and value to nvtab */
int
addname(Nameval newname)
{
	Nameval *nvp;
	size_t i;

	if (nvtab.nameval == NULL) { /* first time */
		nvtab.nameval =
		    (Nameval *) malloc(NVINIT * sizeof(Nameval));
		if (nvtab.nameval == NULL)
			return (-1);
		nvtab.max = NVINIT;
		nvtab.nval = 0;
	} else if (nvtab.nval >= nvtab.max) { /* grow */
		nvp = (Nameval *) realloc(nvtab.nameval,
		    (NVGROW * nvtab.max) * sizeof(Nameval));
		if (nvp == NULL)
			return (-1);
		nvtab.max *= NVGROW;
		nvtab.nameval = nvp;
	}

	for (i = 0; i < nvtab.nval; i++) {
		if (nvtab.nameval[i].name == NULL) {
			nvtab.nameval[i] = newname;
			break;
		}
	}
	if (nvtab.nval == i)
		nvtab.nameval[nvtab.nval] = newname;
	return (nvtab.nval++);
}

/* delname: remove first matching nameval from nvtab */
int
delname(char *name)
{
	size_t i;

	for (i = 0; i < nvtab.nval; i++)
		if (nvtab.nameval[i].name != NULL &&
		    strcmp(nvtab.nameval[i].name, name) == 0) {
			nvtab.nameval[i].name = NULL;
			nvtab.nval--;
			return (1);
		}
	return (0);
}
