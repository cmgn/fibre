#include "queue.h"

#include <string.h>

static void *queue_addr(struct queue *q, int pos)
{
	return q->mem + q->esize * pos;
}

void queue_init(struct queue *q, void *mem, int cap, int esize)
{
	*q = (struct queue){
		.mem = mem,
		.esize = esize,
		.len = 0,
		.cap = cap,
		.r = 0,
		.w = 0,
	};
}

int queue_add(struct queue *q, void *data)
{
	if (q->len == q->cap) {
		return -1;
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

int queue_peek(struct queue *q, void *data)
{
	if (q->len == 0) {
		return -1;
	}
	memcpy(data, queue_addr(q, q->r), q->esize);
	return 0;
}

int queue_size(struct queue *q)
{
	return q->len;
}

int queue_empty(struct queue *q)
{
	return queue_size(q) == 0;
}
