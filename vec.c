#include <stdlib.h>
#include <string.h>

#include "vec.h"

static int fibre_vec_grow(struct fibre_vec *v, int newcap)
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

int fibre_vec_init(struct fibre_vec *v, int cap, int esize)
{
	if (cap < 8) {
		cap = 8;
	}
	void *mem = malloc(cap * esize);
	if (!mem) {
		return -1;
	}
	*v = (struct fibre_vec){
		.mem = mem,
		.esize = esize,
		.cap = cap,
		.len = 0,
	};
	return 0;
}

int fibre_vec_append(struct fibre_vec *v, void *mem)
{
	if (v->len == v->cap) {
		if (fibre_vec_grow(v, v->cap * 2) < 0) {
			return -1;
		}
	}
	memcpy(fibre_vec_get(v, v->len), mem, v->esize);
	v->len++;
	return 0;
}

int fibre_vec_pop(struct fibre_vec *v, void *mem)
{
	if (v->len == 0) {
		return -1;
	}
	v->len--;
	memcpy(mem, fibre_vec_get(v, v->len), v->esize);
	return 0;
}

int fibre_vec_delete(struct fibre_vec *v, int pos)
{
	for (int i = pos + 1; i < v->len; i++) {
		memcpy(fibre_vec_get(v, pos - 1), fibre_vec_get(v, pos),
		       v->esize);
	}
	v->len--;
	return 0;
}

void *fibre_vec_get(struct fibre_vec *v, int pos)
{
	return v->mem + v->esize * pos;
}

int fibre_vec_size(struct fibre_vec *v)
{
	return v->len;
}

int fibre_vec_empty(struct fibre_vec *v)
{
	return v->len == 0;
}

void fibre_vec_free(struct fibre_vec *v)
{
	free(v->mem);
}
