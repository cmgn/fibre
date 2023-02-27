#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>

struct fibre_hashmap {
	uint8_t *flags;
	void *keys;
	void *vals;
	int keysize;
	int valsize;
	int cap;
	int len;
};

struct fibre_hashmap_iter {
	struct fibre_hashmap *h;
	int pos;
};

extern int fibre_hashmap_init(struct fibre_hashmap *h, int cap, int keysize,
			      int valsize);
extern int fibre_hashmap_insert(struct fibre_hashmap *h, void *key, void *val);
extern int fibre_hashmap_delete(struct fibre_hashmap *h, void *key);
extern void *fibre_hashmap_get(struct fibre_hashmap *h, void *key);
extern int fibre_hashmap_size(struct fibre_hashmap *h);
extern void fibre_hashmap_free(struct fibre_hashmap *h);

extern void fibre_hashmap_iter_init(struct fibre_hashmap_iter *it,
				    struct fibre_hashmap *h);
extern void *fibre_hashmap_iter_next(struct fibre_hashmap_iter *it);

#endif
