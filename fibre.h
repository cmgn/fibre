#ifndef _FIBRE_H
#define _FIBRE_H

#include <sys/epoll.h>

#include "third_party/coro/coro.h"

struct fibre {
	struct coro c;
	struct epoll_event ev;
};

typedef void (*fibre_func)(void *arg);

extern int spawn(fibre_func func, void *arg);
extern void yield(struct epoll_event *ev);
extern int start(fibre_func entry, void *arg);

#endif
