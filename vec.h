#ifndef _VEC_H
#define _VEC_H

struct fibre_vec {
	void *mem;
	int esize;
	int cap;
	int len;
};

extern int fibre_vec_init(struct fibre_vec *v, int cap, int esize);
extern int fibre_vec_append(struct fibre_vec *v, void *data);
extern int fibre_vec_pop(struct fibre_vec *v, void *data);
extern int fibre_vec_delete(struct fibre_vec *v, int pos);
extern void *fibre_vec_get(struct fibre_vec *v, int pos);
extern int fibre_vec_size(struct fibre_vec *v);
extern int fibre_vec_empty(struct fibre_vec *v);
extern void fibre_vec_free(struct fibre_vec *v);

#endif
