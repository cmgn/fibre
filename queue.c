#include <string.h>
#include <stdlib.h>

#include "queue.h"

static void *queue_addr(struct queue *q, int pos)
{
	return q->mem + q->esize * pos;
}

static int queue_grow(struct queue *q, int newcap)
{
	struct queue newq = { 0 };
	void *mem = malloc(q->esize * newcap);
	if (!mem) {
		return -1;
	}
	void *tmp = malloc(q->esize);
	if (!tmp) {
		goto cleanup_mem;
	}
	queue_init(&newq, newcap, q->esize);
	while (!queue_empty(q)) {
		queue_poll(q, tmp);
		queue_add(&newq, tmp);
	}
	free(q->mem);
	free(tmp);
	memcpy(q, &newq, sizeof(*q));
	return 0;

cleanup_mem:
	free(mem);
	return -1;
}

int queue_init(struct queue *q, int cap, int esize)
{
	if (cap < 8) {
		cap = 8;
	}
	void *mem = malloc(esize * cap);
	if (!mem) {
		return -1;
	}
	*q = (struct queue){
		.mem = mem,
		.esize = esize,
		.len = 0,
		.cap = cap,
		.r = 0,
		.w = 0,
	};
	return 0;
}

int queue_add(struct queue *q, void *data)
{
	if (q->len == q->cap) {
		if (queue_grow(q, q->cap * 2) < 0) {
			return -1;
		}
	}
	q->len++;
	memcpy(queue_addr(q, q->w), data, q->esize);
	q->w = (q->w + 1) % q->cap;
	return 0;
}

int queue_poll(struct queue *q, void *data)
{
	if (q->len == 0) {
		return -1;
	}
	q->len--;
	memcpy(data, queue_addr(q, q->r), q->esize);
	q->r = (q->r + 1) % q->cap;
	return 0;
}

void *queue_peek(struct queue *q)
{
	if (q->len == 0) {
		return 0;
	}
	return queue_addr(q, q->r);
}

int queue_size(struct queue *q)
{
	return q->len;
}

int queue_empty(struct queue *q)
{
	return queue_size(q) == 0;
}

void queue_free(struct queue *q)
{
	free(q->mem);
}
