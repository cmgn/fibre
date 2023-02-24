#ifndef _QUEUE_H
#define _QUEUE_H

struct fibre_queue {
	unsigned char *mem;
	int esize;
	int len;
	int cap;
	int r;
	int w;
};

extern int fibre_queue_init(struct fibre_queue *q, int cap, int esize);
extern int fibre_queue_add(struct fibre_queue *q, void *data);
extern int fibre_queue_poll(struct fibre_queue *q, void *out);
extern void *fibre_queue_peek(struct fibre_queue *q);
extern int fibre_queue_size(struct fibre_queue *q);
extern int fibre_queue_empty(struct fibre_queue *q);
extern void fibre_queue_free(struct fibre_queue *q);

#endif
