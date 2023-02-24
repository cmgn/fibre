#ifndef _VEC_H
#define _VEC_H

struct vec {
	void *mem;
	int esize;
	int cap;
	int len;
};

extern int vec_init(struct vec *v, int cap, int esize);
extern int vec_append(struct vec *v, void *data);
extern int vec_pop(struct vec *v, void *data);
extern int vec_delete(struct vec *v, int pos);
extern void *vec_get(struct vec *v, int pos);
extern int vec_size(struct vec *v);
extern int vec_empty(struct vec *v);
extern void vec_free(struct vec *v);

#endif
