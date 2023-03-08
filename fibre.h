#ifndef _FIBRE_H
#define _FIBRE_H

#include <sys/epoll.h>

#include "third_party/coro/coro.h"

struct fibre {
	struct coro c;
	struct epoll_event ev;
};

struct fibre_opts {
	int stack_size;
};

typedef void (*fibre_func)(void *arg);

extern int fibre_spawn(fibre_func func, void *arg);
extern void fibre_yield(struct epoll_event *ev);
extern int fibre_start(fibre_func entry, void *arg);
extern void fibre_set_opts(struct fibre_opts *opts);

#endif
