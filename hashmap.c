#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

#define HASHMAP_USED 1

#define GROWTH_FACTOR 2
// TODO(cmgn): Make this configurable.
#define GROWTH_THRESHOLD 0.8

static void *fibre_hashmap_key_addr(struct fibre_hashmap *h, int pos)
{
	return h->keys + h->keysize * pos;
}

static void *fibre_hashmap_val_addr(struct fibre_hashmap *h, int pos)
{
	return h->vals + h->valsize * pos;
}

static int fibre_hashmap_pos_used(struct fibre_hashmap *h, int pos)
{
	return h->flags[pos] & HASHMAP_USED;
}

static unsigned long hash(unsigned char *a, int len)
{
	unsigned long hash = 5381;
	for (int i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + a[i];
	}
	return hash;
}

static int fibre_hashmap_grow(struct fibre_hashmap *h, int newcap)
{
	// We don't use realloc because we cannot recover if one of them fails.
	void *newkeys = malloc(h->keysize * newcap);
	if (!newkeys) {
		return -1;
	}
	void *newvals = malloc(h->valsize * newcap);
	if (!newvals) {
		goto cleanup_keys;
	}
	// Be careful here: we use calloc() because the flags have to be zeroed.
	void *newflags = calloc(1, sizeof(*h->flags) * newcap);
	if (!newflags) {
		goto cleanup_vals;
	}
	void *oldkeys = h->keys;
	void *oldvals = h->vals;
	uint8_t *oldflags = h->flags;
	int oldcap = h->cap;
	h->keys = newkeys;
	h->vals = newvals;
	h->flags = newflags;
	h->len = 0;
	h->cap = newcap;
	for (int i = 0; i < oldcap; i++) {
		if (oldflags[i] & HASHMAP_USED) {
			fibre_hashmap_insert(h, oldkeys + h->keysize * i,
					     oldvals + h->valsize * i);
		}
	}
	free(oldkeys);
	free(oldvals);
	free(oldflags);
	return 0;

cleanup_vals:
	free(newvals);
cleanup_keys:
	free(newkeys);
	return -1;
}

static int fibre_hashmap_maybe_grow(struct fibre_hashmap *h)
{
	double usage = (double)h->len / (double)h->cap;
	if (usage >= GROWTH_THRESHOLD) {
		return fibre_hashmap_grow(h, h->cap * GROWTH_FACTOR);
	}
	return 0;
}

static int fibre_hashmap_find(struct fibre_hashmap *h, void *key)
{
	int fibre_startpos = hash(key, h->keysize) % h->cap;
	for (int i = 0; i < h->cap; i++) {
		int pos = (fibre_startpos + i) % h->cap;
		if (!fibre_hashmap_pos_used(h, pos)) {
			return pos;
		}
		if (memcmp(fibre_hashmap_key_addr(h, pos), key, h->keysize) ==
		    0) {
			return pos;
		}
	}
	return -1;
}

int fibre_hashmap_init(struct fibre_hashmap *h, int cap, int keysize,
		       int valsize)
{
	if (cap < 8) {
		cap = 8;
	}
	*h = (struct fibre_hashmap){
		.flags = 0,
		.keys = 0,
		.vals = 0,
		.keysize = keysize,
		.valsize = valsize,
		.cap = 0,
		.len = 0,
	};
	return fibre_hashmap_grow(h, cap);
}

int fibre_hashmap_insert(struct fibre_hashmap *h, void *key, void *val)
{
	int pos = fibre_hashmap_find(h, key);
	if (pos < 0) {
		return -1;
	}
	memcpy(fibre_hashmap_val_addr(h, pos), val, h->valsize);
	if (!fibre_hashmap_pos_used(h, pos)) {
		h->len++;
		h->flags[pos] |= HASHMAP_USED;
		memcpy(fibre_hashmap_key_addr(h, pos), key, h->keysize);
	}
	return fibre_hashmap_maybe_grow(h);
}

void *fibre_hashmap_get(struct fibre_hashmap *h, void *key)
{
	int pos = fibre_hashmap_find(h, key);
	if (pos < 0 || !fibre_hashmap_pos_used(h, pos)) {
		return 0;
	}
	return fibre_hashmap_val_addr(h, pos);
}

int fibre_hashmap_delete(struct fibre_hashmap *h, void *key)
{
	int pos = fibre_hashmap_find(h, key);
	if (pos < 0 || !fibre_hashmap_pos_used(h, pos)) {
		return -1;
	}
	h->len--;
	h->flags[pos] &= ~HASHMAP_USED;
	return 0;
}

int fibre_hashmap_size(struct fibre_hashmap *h)
{
	return h->len;
}

void fibre_hashmap_free(struct fibre_hashmap *h)
{
	free(h->flags);
	free(h->keys);
	free(h->vals);
}

void fibre_fibre_hashmap_iter_init(struct fibre_hashmap_iter *it,
				   struct fibre_hashmap *h)
{
	*it = (struct fibre_hashmap_iter){
		.h = h,
		.pos = 0,
	};
}

void *fibre_fibre_hashmap_iter_next(struct fibre_hashmap_iter *it)
{
	while (it->pos < it->h->cap &&
	       !fibre_hashmap_pos_used(it->h, it->pos)) {
		it->pos++;
	}
	if (it->pos >= it->h->cap) {
		return 0;
	}
	void *addr = fibre_hashmap_key_addr(it->h, it->pos);
	it->pos++;
	return addr;
}
