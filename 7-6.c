#include <stdio.h>
#include <stdlib.h>

#include "7-2.c" /* ugly include I know */

static long nruns = 1000000000;

static void calibrate_nruns(void); /* setup */
static int bench_add(void);
static int printresults(const char *, long);
static int bench_sub(void);
static int bench_mul(void);
static int bench_div(void);
static int bench_fadd(void);
static int bench_fsub(void);
static int bench_fmul(void);
static int bench_fdiv(void);
static int bench_func(void);
static void nullfunc(void);
static int bench_loop(void);
static int bench_arrind(void);

int
main(void)
{
	calibrate_nruns(); /* setup */

	if (bench_add() == -1)
		goto err;
	if (bench_sub() == -1)
		goto err;
	if (bench_mul() == -1)
		goto err;
	if (bench_div() == -1)
		goto err;
	if (bench_fadd() == -1)
		goto err;
	if (bench_fsub() == -1)
		goto err;
	if (bench_fmul() == -1)
		goto err;
	if (bench_fdiv() == -1)
		goto err;
	if (bench_func() == -1)
		goto err;
	if (bench_loop() == -1)
		goto err;
	if (bench_arrind() == -1)
		goto err;

	return (EXIT_SUCCESS);
err:
	return (EXIT_FAILURE);
}

/* calibrate_nruns: the project should do additions for around 1 second */
static void
calibrate_nruns(void)
{

}

static int
bench_add(void)
{
	struct timings tim;
	long i;

	benchsetup(&tim);
	for (i = 0; i < nruns; i++)
		64 + 46;
	return (printresults("addition", benchget(&tim)));
}

static int
printresults(const char *test, long time)
{
	if (printf("%-16s: %ld.%ld\n", test, time / BENCH_TPS, time % BENCH_TPS)
	    < 0)
		return (-1);
	return (0);
}

static int
bench_sub(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for (i = 0; i < nruns; i++)
		64 - 46;
	time = benchget(&tim);
	return (printresults("subtraction", time));
}

static int
bench_mul(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for (i = 0; i < nruns; i++)
		64 * 46;
	time = benchget(&tim);
	return (printresults("multiplication", time));
}

static int
bench_div(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for (i = 0; i < nruns; i++)
		64 / 46;
	time = benchget(&tim);
	return (printresults("division", time));
}

static int
bench_fadd(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		64.0 + 46.0;
	time = benchget(&tim);
	return (printresults("float addition", time));
}

static int
bench_fsub(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		64.0 - 46.0;
	time = benchget(&tim);
	return (printresults("float subtraction", time));
}

static int
bench_fmul(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		64.0 * 46.0;
	time = benchget(&tim);
	return (printresults("float multiplication", time));
}

static int
bench_fdiv(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		64.0 / 46.0;
	time = benchget(&tim);
	return (printresults("float division", time));
}

static int
bench_func(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		nullfunc();
	time = benchget(&tim);
	return (printresults("function call", time));
}

static void
nullfunc(void)
{

}

static int
bench_loop(void)
{
	struct timings tim;
	long i;
	long time;
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		;
	time = benchget(&tim);
	return (printresults("loop", time));
}

static int
bench_arrind(void)
{
	struct timings tim;
	long i;
	long time;
	int arr[32];
	benchsetup(&tim);
	for  (i = 0; i < nruns; i++)
		arr[8];
	time = benchget(&tim);
	return (printresults("array indexing", time));
}
