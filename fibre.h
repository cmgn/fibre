#ifndef _FIBRE_H
#define _FIBRE_H

#include "third_party/coro/coro.h"

struct fibre {
	struct coro c;
};

typedef void (*fibre_func)(void *arg);

extern void spawn(fibre_func func, void *arg);
extern void yield();
extern void start(fibre_func entry, void *arg);

#endif
