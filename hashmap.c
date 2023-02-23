#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

#define HASHMAP_USED 1

#define GROWTH_FACTOR 2
// TODO(cmgn): Make this configurable.
#define GROWTH_THRESHOLD 0.8

static void *hashmap_key_addr(struct hashmap *h, int pos)
{
	return h->keys + h->keysize * pos;
}

static void *hashmap_val_addr(struct hashmap *h, int pos)
{
	return h->vals + h->valsize * pos;
}

static int hashmap_pos_used(struct hashmap *h, int pos)
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

static int hashmap_grow(struct hashmap *h, int newcap)
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
			hashmap_insert(h, oldkeys + h->keysize * i,
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

static int hashmap_maybe_grow(struct hashmap *h)
{
	double usage = (double)h->len / (double)h->cap;
	if (usage >= GROWTH_THRESHOLD) {
		return hashmap_grow(h, h->cap * GROWTH_FACTOR);
	}
	return 0;
}

static int hashmap_find(struct hashmap *h, void *key)
{
	int startpos = hash(key, h->keysize) % h->cap;
	for (int i = 0; i < h->cap; i++) {
		int pos = (startpos + i) % h->cap;
		if (!hashmap_pos_used(h, pos)) {
			return pos;
		}
		if (memcmp(hashmap_key_addr(h, pos), key, h->keysize) == 0) {
			return pos;
		}
	}
	return -1;
}

int hashmap_init(struct hashmap *h, int cap, int keysize, int valsize)
{
	if (cap < 8) {
		cap = 8;
	}
	*h = (struct hashmap){
		.flags = 0,
		.keys = 0,
		.vals = 0,
		.keysize = keysize,
		.valsize = valsize,
		.cap = 0,
		.len = 0,
	};
	return hashmap_grow(h, cap);
}

int hashmap_insert(struct hashmap *h, void *key, void *val)
{
	int pos = hashmap_find(h, key);
	if (pos < 0) {
		return -1;
	}
	memcpy(hashmap_val_addr(h, pos), val, h->valsize);
	if (!hashmap_pos_used(h, pos)) {
		h->len++;
		h->flags[pos] |= HASHMAP_USED;
		memcpy(hashmap_key_addr(h, pos), key, h->keysize);
	}
	return hashmap_maybe_grow(h);
}

void *hashmap_get(struct hashmap *h, void *key)
{
	int pos = hashmap_find(h, key);
	if (pos < 0 || !hashmap_pos_used(h, pos)) {
		return 0;
	}
	return hashmap_val_addr(h, pos);
}

int hashmap_delete(struct hashmap *h, void *key)
{
	int pos = hashmap_find(h, key);
	if (pos < 0 || !hashmap_pos_used(h, pos)) {
		return -1;
	}
	h->len--;
	h->flags[pos] &= ~HASHMAP_USED;
	return 0;
}

int hashmap_size(struct hashmap *h)
{
	return h->len;
}

void hashmap_free(struct hashmap *h)
{
	free(h->flags);
	free(h->keys);
	free(h->vals);
}

void hashmap_iter_init(struct hashmap_iter *it, struct hashmap *h)
{
	*it = (struct hashmap_iter){
		.h = h,
		.pos = 0,
	};
}

void *hashmap_iter_next(struct hashmap_iter *it)
{
	while (it->pos < it->h->cap && !hashmap_pos_used(it->h, it->pos)) {
		it->pos++;
	}
	if (it->pos >= it->h->cap) {
		return 0;
	}
	void *addr = hashmap_key_addr(it->h, it->pos);
	it->pos++;
	return addr;
}
