#include <stdlib.h>
#include <string.h>

#include "vec.h"

static int vec_grow(struct vec *v, int newcap)
{
	void *newmem = malloc(v->esize * newcap);
	if (!newmem) {
		return -1;
	}
	memcpy(newmem, v->mem, v->esize * v->len);
	v->cap = newcap;
	free(v->mem);
	v->mem = newmem;
	return 0;
}

int vec_init(struct vec *v, int cap, int esize)
{
	if (cap < 8) {
		cap = 8;
	}
	void *mem = malloc(cap * esize);
	if (!mem) {
		return -1;
	}
	*v = (struct vec){
		.mem = mem,
		.esize = esize,
		.cap = cap,
		.len = 0,
	};
	return 0;
}

int vec_append(struct vec *v, void *mem)
{
	if (v->len == v->cap) {
		if (vec_grow(v, v->cap * 2) < 0) {
			return -1;
		}
	}
	memcpy(vec_get(v, v->len), mem, v->esize);
	v->len++;
	return 0;
}

int vec_pop(struct vec *v, void *mem)
{
	if (v->len == 0) {
		return -1;
	}
	v->len--;
	memcpy(mem, vec_get(v, v->len), v->esize);
	return 0;
}

int vec_delete(struct vec *v, int pos)
{
	for (int i = pos + 1; i < v->len; i++) {
		memcpy(vec_get(v, pos - 1), vec_get(v, pos), v->esize);
	}
	v->len--;
	return 0;
}

void *vec_get(struct vec *v, int pos)
{
	return v->mem + v->esize * pos;
}

int vec_size(struct vec *v)
{
	return v->len;
}

int vec_empty(struct vec *v)
{
	return v->len == 0;
}

void vec_free(struct vec *v)
{
	free(v->mem);
}
