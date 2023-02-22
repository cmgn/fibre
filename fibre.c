#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fibre.h"
#include "queue.h"

#define STACK_SIZE (8 * 1024 * 1024)
#define MAX_FIBRES 8192

struct fibre *curr;
struct queue ready;

static void spawn_entry(struct coro *c, fibre_func func)
{
	void *arg = coro_yield(c, 0);
	coro_yield(c, 0);
	func(arg);
}

int spawn(fibre_func func, void *arg)
{
	struct fibre *f = (struct fibre *)malloc(sizeof(*f));
	if (!f) {
		return -1;
	}
	memset(f, 0, sizeof(*f));
	void *stack = malloc(STACK_SIZE);
	if (!stack) {
		goto cleanup_fibre;
	}
	coro_init(&f->c, stack, STACK_SIZE, (coro_func)spawn_entry, func);
	coro_resume(&f->c);
	coro_yield(&f->c, arg);
	queue_add(&ready, &f);
	return 0;

cleanup_fibre:
	free(f);
	return -1;
}

void yield()
{
	coro_yield(&curr->c, 0);
}

int start(fibre_func entry, void *arg)
{
	int status = 0;
	void *mem = malloc(sizeof(struct fibre *) * MAX_FIBRES);
	if (!mem) {
		status = -1;
		goto cleanup;
	}
	queue_init(&ready, mem, MAX_FIBRES, sizeof(struct fibre *));
	if (spawn(entry, arg) < 0) {
		status = -1;
		goto cleanup;
	}
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
cleanup:
	free(mem);
	return status;
}
