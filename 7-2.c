#include <time.h>
#include <stdlib.h>

struct timings {
	clock_t clocks;
};

enum {
	/* How many times per second the benchmark clock ticks */
	BENCH_TPS = CLOCKS_PER_SEC
};

/* benchsetup: set up benchmark.
 * if tp is NULL, allocate and return it. Otherwise the argument is always
 * returned.
 */
struct timings *
benchsetup(struct timings *tp)
{
	if (tp == NULL)
		if ((tp = malloc(sizeof(*tp))) == NULL)
			return (NULL);
	tp->clocks = clock();
	return (tp);
}

/* benchget: get time between set up and benchget */
long
benchget(const struct timings *tp)
{
	return (clock() - tp->clocks);
}
