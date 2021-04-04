#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

enum {
	VALSIZE = MAX(sizeof(size_t), sizeof(void *)),
};

enum pc {
	PC_INT,
	PC_FLOAT,
};

struct valtree {
	size_t		 cnt;	/* How many times has val been found? */
	char		 val[VALSIZE]; /* unique byte sequence */
	struct valtree	*next;
	struct valtree  *prev;
};

struct valmeta {
	/* size of value inside root->val and its children */
	size_t		 vallen;
	struct valtree	*root;
};

typedef int (vmfunc)(struct valtree *, void *);

int freq(FILE *, size_t, int);
static int printcounts(struct valtree *, void *);
static struct valtree *valmeta_lookup(const struct valmeta *, const void *,
    int);
static struct valmeta *valmeta_new(size_t typesize);
static struct valtree *valtree_lookup(const struct valmeta *,
    struct valtree *, const void *, int);
static struct valtree *valtree_new(const struct valmeta *, const void *);
static int valmeta_apply(struct valmeta *, vmfunc *, void *);
static int valtree_apply(struct valtree *, vmfunc *, void *);

/* freq: get frequency of unique byte inputs from fp */
int
freq(FILE *fp, size_t size, int pctype)
{
	struct valmeta *vmp;
	unsigned char *buf = NULL;
	size_t n;
	size_t left;
	int ret = -1;
	if ((vmp = valmeta_new(size)) == NULL)
		goto end;
	if ((buf = malloc(size)) == NULL)
		goto end;

	for (;;) {
		n = fread(buf, size, 1, fp);
		if (n == 0) {
			if (feof(fp))
				break;
			if (ferror(fp))
				goto end;
		}
		if ((left = size - n)) /* If space left in the buffer. */
			memset(&buf[n], 0, left % size);
		if (valmeta_lookup(vmp, buf, 1) == NULL)
			goto end;
	}

	if (valmeta_apply(vmp, &printcounts, &pctype))
		goto end;
	ret = 0;
end:
	free(vmp);
	free(buf);
	return (ret);
}

static int
printcounts(struct valtree *vtp, void *arg)
{
	enum pc *type = arg;
	int ret;
	union {
		int inttype;
		float floattype;
	} num;

	if (*type == PC_INT) {
		memcpy(&num.inttype, vtp->val, sizeof(num.inttype));
		ret = printf("count for int %d: %lu\n", num.inttype,
		    (unsigned long)vtp->cnt);
	}
	else if (*type == PC_FLOAT) {
		memcpy(&num.floattype, vtp->val, sizeof(num.floattype));
		ret = printf("count for float %f: %lu\n",
		    num.floattype, (unsigned long)vtp->cnt);
	} else
		abort();


	return (ret < 0 ? -1 : 0);
}

static struct valtree *
valmeta_lookup(const struct valmeta *vmp, const void *val, int create)
{
	return (valtree_lookup(vmp, vmp->root, val, create));
}

/* valtree_lookup: find value in valtree
 * If value is already in the tree, the node holding it is returned and cnt is
 * incremented.
 * If create is set, create new node holding value, and return NULL if
 * allocation fails.
 * If create is not set, return NULL if the value is not found.
 */
static struct valtree *
valtree_lookup(const struct valmeta *vmp, struct valtree *vtp, const void *val,
    int create)
{
	int ret;
	if (vtp == NULL) {
		if (create)
			return (valtree_new(vmp, val));
		return (NULL);
	}
	ret = memcmp(val, vtp->val, vmp->vallen);
	if (ret < 0) {
		vtp->prev = valtree_lookup(vmp, vtp->prev, val, create);
		return (vtp->prev);
	} else if (ret == 0) {
		vtp->cnt++;
		return (vtp);
	} else {
		vtp->next = valtree_lookup(vmp, vtp->next, val, create);
		return (vtp->next);
	}
}

static struct valmeta *
valmeta_new(size_t typesize)
{
	struct valmeta *vmp;
	if ((vmp = calloc(sizeof(*vmp), 1)) == NULL)
		return (NULL);
	vmp->vallen = typesize;
	return (vmp);
}

static struct valtree *
valtree_new(const struct valmeta *vmp, const void *val)
{
	struct valtree *vtp;
	if ((vtp = calloc(sizeof(*vtp), 1)) == NULL)
		return (NULL);
	memcpy(vtp->val, val, vmp->vallen);
	vtp->cnt = 1;
	return (vtp);
}

static int
valmeta_apply(struct valmeta *vmp, vmfunc *func, void *arg)
{
	return (valtree_apply(vmp->root, func, arg));
}

static int
valtree_apply(struct valtree *vtp, vmfunc *func, void *arg)
{
	int ret = -1;
	if (vtp == NULL)
		goto end;
	if ((ret = valtree_apply(vtp->prev, func, arg)))
		goto end;
	if ((ret = func(vtp, arg)))
		goto end;
	if ((ret = valtree_apply(vtp->next, func, arg)))
		goto end;

	ret = 0;
end:
	return (ret);
}
