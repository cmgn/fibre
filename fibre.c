#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fibre.h"
#include "queue.h"

#define STACK_SIZE (8 * 1024 * 1024)
#define MAX_FIBRES 8192

struct fibre *curr;
struct queue ready;

static void *allocate(uint64_t n)
{
	void *p = malloc(n);
	if (!p) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	return p;
}

static void spawn_entry(struct coro *c, fibre_func func)
{
	void *arg = coro_yield(c, 0);
	coro_yield(c, 0);
	func(arg);
}

void spawn(fibre_func func, void *arg)
{
	struct fibre *f = (struct fibre *)allocate(sizeof(*f));
	memset(f, 0, sizeof(*f));
	void *stack = allocate(STACK_SIZE);
	coro_init(&f->c, stack, STACK_SIZE, (coro_func)spawn_entry, func);
	coro_resume(&f->c);
	coro_yield(&f->c, arg);
	queue_add(&ready, &f);
}

void yield()
{
	coro_yield(&curr->c, 0);
}

void start(fibre_func entry, void *arg)
{
	void *mem = allocate(sizeof(struct fibre *) * MAX_FIBRES);
	queue_init(&ready, mem, MAX_FIBRES, sizeof(struct fibre *));
	spawn(entry, arg);
	while (!queue_empty(&ready)) {
		struct fibre *f;
		queue_poll(&ready, &f);
		curr = f;
		coro_resume(&f->c);
		if (!coro_done(&f->c)) {
			queue_add(&ready, &f);
		} else {
			coro_free(&f->c);
			free(f);
		}
	}
}
