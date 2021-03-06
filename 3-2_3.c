#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
	/* How many strings are in struct strtable by default. */
	STRTABLEBUFNMEMB = 10240,
	/* It is assumed that a word will never be larger than MAXWORDLEN. */
	MAXWORDLEN = 100,
	/* Optimal factor for ASCII string hash function. */
	OPTMULT = 37,
	/* Number of word prefixes in Markov chain */
	NPREF = 2,
	/* How many are in struct mkrvtable by default. */
	MRKVTABLEBUFNMEMB = 10240
};

/* strt_hash: an unsigned type with a size <= size_t */
typedef size_t strt_hash;
/* mrkv_hash: an unsigned type with a size <= size_t */
typedef size_t mrkv_hash;

struct strtable {
	/* Number of members in tab. */
	size_t		nmemb;
	/* Number of members that fit in tab's buffer. */
	size_t		bufnmemb;
	struct strlist	**tab;
};

struct strlist {
	/* Always an unique pointer which can be passed to free(), thus equal
	 * strings already in the list are always behind equal pointers. */
	const char	*str;
	struct strlist	*next;
};

struct mrkvtable {
	/* Number of members in tab. */
	size_t 			nmemb;
	/* Number of members that fit in tab's buffer. */
	size_t 			bufnmemb;
	/* Number of word prefixes in Markov chain */
	size_t			npref;
	struct mrkvstate	**tab;
};

struct mrkvstate {
	const char		*pref[NPREF];
	struct mrkvsuffix	*suf;
	struct mrkvstate	*next;
};

struct mrkvsuffix {
	const char 		*word;
	struct mrkvsuffix 	*next;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int cook_args(size_t, char *[]);
static int do_work(struct mrkvtable *, struct strtable *, const char **, size_t,
    const char *[]);
static int body_work(FILE *, struct mrkvtable *, struct strtable *,
    const char **);
static int generate(struct mrkvtable *, struct strtable *, const char **);
static void mrkv_prefixrand(const struct mrkvtable *, const char **);
static const char *mrkv_sufrand(const struct mrkvstate *);
static const char *readword(FILE *);
static int skipspace(FILE *);

static struct strlist *strt_lookup(struct strtable *, const char *, int);
static int strt_addstr(struct strtable *, const char **);
static struct strtable *strt_new(struct strtable *);
static void strt_free(struct strtable *);
static struct strlist *strl_new(struct strlist *, const char *);
static void strl_free(struct strlist *);
static strt_hash strt_hashstr(const struct strtable *, const char *);

static struct mrkvtable *mrkv_tablenew(void);
static void mrkv_tablefree(struct mrkvtable *);
static struct mrkvsuffix *mrkv_suffixnew(const char *);
static int mrkv_sufadd(struct mrkvstate *, const char *);
static void mrkv_suffixfree(struct mrkvsuffix *);
static struct mrkvstate *mrkv_statenew(const char *[]);
static void mrkv_statefree(struct mrkvstate *);
static mrkv_hash mrkv_hashstate(const struct mrkvtable *, const char *[]);
static struct mrkvstate *mrkv_lookup(struct mrkvtable *, const char *[], int);
static int mrkv_prefcmp(const struct mrkvtable *, const char *[],
    const char *[]);

long cflag;

/* This program reads words from stdin and/or files specified as arguments, adds
 * them to a markov chain, and prints out that many words
 */
int
main(int argc, char *argv[])
{
	int c;
	const char *errstr;
	while ((c = getopt(argc, argv, "c:")) != -1) {
		switch (c) {
		case 'c':
			cflag = strtonum(optarg, 0, LONG_MAX, &errstr);
			if (errstr != NULL)
				goto err;
			break;
		default:
			abort();
		}
	}
	argc -= optind;
	argv += optind;
	if (cook_args(argc, argv) == -1)
		goto err;
	return (EXIT_SUCCESS);
err:
	perror(NULL);
	return (EXIT_FAILURE);
}

/* cook_args: do the bulk of the program's work.
 * Actually just an allocator for the next function.
 * Reads count filenames off of the files array.
 *
 * Returns -1 on fopen or malloc error.
 */
static int
cook_args(size_t count, char *files[])
{
	struct strtable *strtab = NULL;
	struct mrkvtable *mrkvtab = NULL;
	const char **pref = NULL;
	char *dash = "-";
	int ret = -1;
	if ((strtab = strt_new(NULL)) == NULL)
		goto end;
	if ((mrkvtab = mrkv_tablenew()) == NULL)
		goto end;
	if ((pref = calloc(mrkvtab->npref, sizeof(*pref))) == NULL)
		goto end;
	if (count == 0) {
		count = 1;
		files = &dash;
	}
	if (do_work(mrkvtab, strtab, pref, count, (const char **)files) == -1)
		goto end;
	ret = 0;
end:
	strt_free(strtab);
	mrkv_tablefree(mrkvtab);
	free(pref);
	return (ret);
}

/* do_work: do the bulk of the program's work
 * Builds up mrkvtab and strtab data structures, but does not run them
 */
static int
do_work(struct mrkvtable *mrkvtab, struct strtable *strtab, const char **pref,
    size_t count, const char *files[])
{
	size_t i;
	FILE *input = NULL;
	int ret = -1;
	for (i = 0; i < count; i++) {
		if ((input = fopen(files[i], "rb")) == NULL)
			goto end;
		if (body_work(input, mrkvtab, strtab, pref) == -1)
			goto end;
		fclose(input);
	}
	if (generate(mrkvtab, strtab, pref) == -1)
		goto end;
	ret = 0;
end:
	return (ret);
}

/* body_work: do the bulk of do_work's work
 *
 * Returns -1 on error.
 */
static int
body_work(FILE *input, struct mrkvtable *mrkvtab, struct strtable *strtab,
    const char **pref)
{
	const char *w;
	struct mrkvstate *state;
	while ((w = readword(input)) != NULL) {
		if (strt_addstr(strtab, &w) == -1)
			goto end;
		if (*pref != NULL) {
			if ((state = mrkv_lookup(mrkvtab, pref, 1)) == NULL)
				goto end;
			if (mrkv_sufadd(state, w) == -1)
				goto end;
		}
		memmove(pref, pref + 1, (mrkvtab->npref - 1) * sizeof(*pref));
		pref[mrkvtab->npref - 1] = w;
	}

end:
	free((void *)w);
	return (feof(input) ? 0 : -1);
}

static int
generate(struct mrkvtable *mrkvtab, struct strtable *strtab, const char **pref)
{
	size_t i;
	int ret = -1;
	struct mrkvstate *state;
	const size_t lastpref = mrkvtab->npref - 1;

	mrkv_prefixrand(mrkvtab, pref);
	for (i = 0; i < mrkvtab->npref; i++)
		if (printf("%s\n", pref[i]) < 0)
			goto end;

	for (/* i from previous loop */; i < cflag; i++) {
		state = mrkv_lookup(mrkvtab, pref, 0);
		memmove(pref, pref + 1, lastpref * sizeof(*pref));
		pref[lastpref] = mrkv_sufrand(state);
		if (printf("%s\n", pref[lastpref]) < 0)
			goto end;
	}
	ret = 0;
end:
	return (ret);
}

/* mrkv_prefixrand: set pref to a random prefix from tab */
static void
mrkv_prefixrand(const struct mrkvtable *tab, const char **pref)
{
	size_t i;
	struct mrkvstate *sp, *retsp;
	arc4random_buf(&i, sizeof(i));

	while(tab->tab[i %= tab->bufnmemb] == NULL)
		i++;
	sp = tab->tab[i];
	for (i = 0, retsp = sp; sp->next != NULL; sp = sp->next)
		if (arc4random() % ++i == 0)
			retsp = sp;
	memcpy(pref, retsp->pref, sizeof(*pref) * tab->npref);
}

/* mrkv_sufrand get random string from state's suffix list */
static const char *
mrkv_sufrand(const struct mrkvstate *state)
{
	size_t nfound;
	const struct mrkvsuffix *suf, *tsp;
	nfound = 0;
	for (suf = tsp = state->suf; tsp->next != NULL; tsp = tsp->next)
		if (arc4random() % ++nfound == 0)
			suf = tsp;
	return (suf->word);
}
/* readword: read word from FILE stream, skipping a whitespace prefix
 * The returned pointer to the word can be free()d.
 *
 * Return NULL if EOF with no data read.
 * Return NULL on error, with some data lost.
 */
static const char *
readword(FILE *input)
{
	char *str;
	const size_t bufsz = MAXWORDLEN;
	int c;
	size_t i;
	if ((str = malloc(bufsz)) == NULL)
		goto err;
	if (skipspace(input) != 0)
		goto err;
	for (i = 0; i < bufsz - 1; i++) {
		if ((c = fgetc(input)) == EOF) {
			if (ferror(input))
				goto err;
			else if (feof(input))
				break;
		}
		if (isspace(c)) {
			if (ungetc(c, input) == EOF)
				goto err;
			break;
		}
		str[i] = c;
	}
	str[i] = '\0';
	return (str);
err:
	free(str);
	return (NULL);
}

/* skipspace: skip whitespace in input
 * Return last character read if it can't be ungetc'd
 * Return 0 on success
 * Return EOF on error or EOF
 */
static int
skipspace(FILE *input)
{
	int c;
	while ((c = fgetc(input)) != EOF)
		if (!isspace(c))
			return (ungetc(c, input) == c ? 0 : c);
	return (EOF);
}

/* strt_lookup: look up str in table
 * If create is true, create the hash table entry if it's not found.
 * str becomes property of this function if a new list is created. A new list
 * was created if the returned list's str member is equal to the str argument of
 * this function in a pointer comparison.
 * Returns NULL if the entry is not found.
 * Returns NULL if create is true and creation fails. */
static struct strlist *
strt_lookup(struct strtable *table, const char *str, int create)
{
	struct strlist *listp;
	strt_hash hash;
	hash = strt_hashstr(table, str);

	for (listp = table->tab[hash]; listp != NULL; listp = listp->next)
		if (strcmp(str, listp->str) == 0)
			goto end;
	if (create) {
		if ((listp = strl_new(NULL, str)) == NULL)
			goto end;
		listp->next = table->tab[hash];
		table->tab[hash] = listp;
		table->nmemb++;
	}
end:
	return (listp);
}

/* strt_addstr: add single string to strtab
 * The pointer to the string may change, the string is the strtab's property
 * unless there's an error.
 *
 * Returns -1 on error.
 */
static int
strt_addstr(struct strtable *strtab, const char **str)
{
	struct strlist *sp;
	int ret = -1;
	if ((sp = strt_lookup(strtab, *str, 1)) == NULL)
		goto end;
	if (sp->str != *str) {
		free((void *)*str);
		*str = sp->str;
	}
	ret = 0;
end:
	return (ret);
}

/* strt_new: allocate a struct strtable.
 * If table isn't null, use the buffer it points to, otherwise allocate it.
 * The buffer's contents are undefined on failure. */
static struct strtable *
strt_new(struct strtable *table)
{
	struct strtable *alloc = NULL;
	if (table == NULL)
		if ((alloc = table = malloc(sizeof(*table))) == NULL)
			return (NULL);
	table->nmemb = 0;
	table->bufnmemb = STRTABLEBUFNMEMB;
	if ((table->tab = calloc(table->bufnmemb, sizeof(*table->tab))) == NULL)
		goto err;
	return (table);
err:
	free(table->tab);
	if (alloc != NULL)
		free(table);
	return (NULL);
}

/* strt_free: free table and all of its contents
 * Also accepts a NULL pointer.
 */
static void
strt_free(struct strtable *table)
{
	size_t i;
	if (table == NULL)
		return;
	for (i = 0; i < table->bufnmemb; i++)
		strl_free(table->tab[i]);
	free(table->tab);
	free(table);
}

/* strl_new: allocate a struct strlist holding str.
 * If list isn't null, use the buffer it points to, otherwise allocate it.
 * The buffer's contents are undefined on failure. */
static struct strlist *
strl_new(struct strlist *list, const char *str)
{
	if (list == NULL)
		if ((list = malloc(sizeof(*list))) == NULL)
			return (NULL);
	list->str = str;
	list->next = NULL;
	return (list);
}

/* strl_free: free list and all of its contents
 * Also accepts a NULL pointer.
 */
static void
strl_free(struct strlist *list)
{
	struct strlist *next;
	while (list != NULL) {
		next = list->next;
		free((void *)list->str);
		free(list);
		list = next;
	}
}

/* strt_hashstr: hash str for table */
static strt_hash
strt_hashstr(const struct strtable *table, const char *str)
{
	strt_hash hash;
	for (hash = 0; *str != '\0'; str++)
		hash *= hash * (unsigned int)OPTMULT + *str;
	return (hash % table->bufnmemb);
}

/* mrkv_tablenew: malloc new mrkvtable */
static struct mrkvtable *
mrkv_tablenew(void)
{
	struct mrkvtable *table;
	if ((table = malloc(sizeof(*table))) == NULL)
		goto err;
	table->bufnmemb = MRKVTABLEBUFNMEMB;
	table->nmemb = 0;
	table->npref = NPREF;
	if ((table->tab = calloc(table->bufnmemb, sizeof(*table->tab))) == NULL)
		goto err;

	return (table);
err:
	free(table);
	return (NULL);
}

/* mrkv_tablefree: free mrkv_table
 * table can be NULL
 */
static void
mrkv_tablefree(struct mrkvtable *table)
{
	struct mrkvstate **tabp;
	size_t i;
	if (table == NULL)
		return;
	for (i = 0, tabp = table->tab; i < table->nmemb; i++, tabp++)
		mrkv_statefree(*tabp);
	free(table->tab);
	free(table);
}

/* mrkv_suffixnew: new suffix for markov chain holding word */
static struct mrkvsuffix *
mrkv_suffixnew(const char *word)
{
	struct mrkvsuffix *suf;
	if ((suf = malloc(sizeof(*suf))) == NULL)
		return (NULL);
	suf->word = word;
	suf->next = NULL;
	return (suf);
}

/* mrkv_sufadd: add word to state's list of suffixes
 *
 * Returns -1 on error.
 */
static int
mrkv_sufadd(struct mrkvstate *state, const char *word)
{
	struct mrkvsuffix *suffix;
	if ((suffix = mrkv_suffixnew(word)) == NULL)
		return (-1);
	suffix->next = state->suf;
	state->suf = suffix;
	return (0);
}

/* mrkv_suffixfree: free markov suffix
 * suf can be NULL.
 */
static void
mrkv_suffixfree(struct mrkvsuffix *suf)
{
	struct mrkvsuffix *next;
	while (suf != NULL) {
		next = suf->next;
		free(suf);
		suf = next;
	}
}

/* mrkv_statenew: new markov state */
static struct mrkvstate *
mrkv_statenew(const char *pref[])
{
	struct mrkvstate *state;
	if ((state = malloc((sizeof(*state)))) == NULL)
		return (NULL);
	memcpy(state->pref, pref, sizeof(state->pref));
	state->next = NULL;
	state->suf = NULL;
	return (state);
}

/* mrkv_statefree: free struct mrkvstate
 * state can be NULL
 */
static void
mrkv_statefree(struct mrkvstate *state)
{
	struct mrkvstate *next;
	while (state != NULL) {
		next = state->next;
		mrkv_suffixfree(state->suf);
		free(state);
		state = next;
	}
}

/* mrkv_hashstate: hash markov prefix */
static mrkv_hash
mrkv_hashstate(const struct mrkvtable *tab, const char *pref[])
{
	size_t i;
	const char *sp;
	mrkv_hash hash = 0;
	for (i = 0; i < tab->npref; i++)
		for (sp = pref[i]; *sp != '\0'; sp++)
			hash *= hash * (unsigned int)OPTMULT + *sp;
	return (hash % tab->bufnmemb);
}

/* mrkv_lookup: lookup prefix in mrkvtable
 * If create is true, create state.
 *
 * Returns NULL if the state is not found.
 * Returns NULL if create is true and fails
 */
static struct mrkvstate *
mrkv_lookup(struct mrkvtable *tab, const char *prefix[], int create)
{
	mrkv_hash hash;
	struct mrkvstate *sp;
	struct mrkvstate *state;
	hash = mrkv_hashstate(tab, prefix);

	for (sp = tab->tab[hash]; sp != NULL; sp = sp->next)
		if (mrkv_prefcmp(tab, prefix, sp->pref) == 0)
			goto end;

	if (create) {
		if ((state = mrkv_statenew(prefix)) == NULL)
			goto end;
		state->next = tab->tab[hash];
		tab->tab[hash] = state;
		return (state);
	}
end:
	return (sp);
}

/* mrkv_prefcmp: strcmp an entire struct mrkvtable prefix */
static int
mrkv_prefcmp(const struct mrkvtable *tab, const char *apref[],
    const char *bpref[])
{
	int ret;
	int i;
	for (i = 0; i < tab->npref; i++)
		if ((ret = strcmp(apref[i], bpref[i]) != 0))
			return (ret);
	return (0);
}
