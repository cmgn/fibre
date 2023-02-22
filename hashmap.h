#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>

struct hashmap {
	uint8_t *flags;
	void *keys;
	void *vals;
	int keysize;
	int valsize;
	int cap;
	int len;
};

struct hashmap_iter {
	struct hashmap *h;
	int pos;
};

extern int hashmap_init(struct hashmap *h, int cap, int keysize, int valsize);
extern int hashmap_insert(struct hashmap *h, void *key, void *val);
extern int hashmap_delete(struct hashmap *h, void *key);
extern int hashmap_get(struct hashmap *h, void *key, void *out);
extern int hashmap_size(struct hashmap *h);

extern void hashmap_iter_init(struct hashmap_iter *it, struct hashmap *h);
extern int hashmap_iter_next(struct hashmap_iter *it, void *out);

#endif
