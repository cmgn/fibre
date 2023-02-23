#ifndef _QUEUE_H
#define _QUEUE_H

struct queue {
	unsigned char *mem;
	int esize;
	int len;
	int cap;
	int r;
	int w;
};

extern int queue_init(struct queue *q, int cap, int esize);
extern int queue_add(struct queue *q, void *data);
extern int queue_poll(struct queue *q, void *out);
extern void *queue_peek(struct queue *q);
extern int queue_size(struct queue *q);
extern int queue_empty(struct queue *q);
extern void queue_free(struct queue *q);

#endif
