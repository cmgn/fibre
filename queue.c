#include <string.h>
#include <stdlib.h>

#include "queue.h"

static void *fibre_queue_addr(struct fibre_queue *q, int pos)
{
	return q->mem + q->esize * pos;
}

static int fibre_queue_grow(struct fibre_queue *q, int newcap)
{
	struct fibre_queue newq = { 0 };
	void *mem = malloc(q->esize * newcap);
	if (!mem) {
		return -1;
	}
	void *tmp = malloc(q->esize);
	if (!tmp) {
		goto cleanup_mem;
	}
	fibre_queue_init(&newq, newcap, q->esize);
	while (!fibre_queue_empty(q)) {
		fibre_queue_poll(q, tmp);
		fibre_queue_add(&newq, tmp);
	}
	free(q->mem);
	free(tmp);
	memcpy(q, &newq, sizeof(*q));
	return 0;

cleanup_mem:
	free(mem);
	return -1;
}

int fibre_queue_init(struct fibre_queue *q, int cap, int esize)
{
	if (cap < 8) {
		cap = 8;
	}
	void *mem = malloc(esize * cap);
	if (!mem) {
		return -1;
	}
	*q = (struct fibre_queue){
		.mem = mem,
		.esize = esize,
		.len = 0,
		.cap = cap,
		.r = 0,
		.w = 0,
	};
	return 0;
}

int fibre_queue_add(struct fibre_queue *q, void *data)
{
	if (q->len == q->cap) {
		if (fibre_queue_grow(q, q->cap * 2) < 0) {
			return -1;
		}
	}
	q->len++;
	memcpy(fibre_queue_addr(q, q->w), data, q->esize);
	q->w = (q->w + 1) % q->cap;
	return 0;
}

int fibre_queue_poll(struct fibre_queue *q, void *data)
{
	if (q->len == 0) {
		return -1;
	}
	q->len--;
	memcpy(data, fibre_queue_addr(q, q->r), q->esize);
	q->r = (q->r + 1) % q->cap;
	return 0;
}

void *fibre_queue_peek(struct fibre_queue *q)
{
	if (q->len == 0) {
		return 0;
	}
	return fibre_queue_addr(q, q->r);
}

int fibre_queue_size(struct fibre_queue *q)
{
	return q->len;
}

int fibre_queue_empty(struct fibre_queue *q)
{
	return fibre_queue_size(q) == 0;
}

void fibre_queue_free(struct fibre_queue *q)
{
	free(q->mem);
}
