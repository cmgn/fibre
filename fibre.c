#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <sys/epoll.h>

#include "fibre.h"
#include "queue.h"
#include "hashmap.h"
#include "vec.h"

// Size of the stack for a fibre.
#define STACK_SIZE (128 * 1024)

#define CONTAINER_SIZE 32

// Enable detailed tracing.
//#define TRACE

#ifdef TRACE
#define LOG(MSG, ...)                                 \
	do {                                          \
		printf("%s:%d ", __FILE__, __LINE__); \
		printf((MSG), ##__VA_ARGS__);         \
		puts("");                             \
		fflush(stdout);                       \
	} while (0)
#else
#define LOG(MSG, ...)
#endif

#define LEN(X) (sizeof(X) / sizeof(*X))

// The currently active fibre.
static struct fibre *curr;

// Pointers to fibres who are ready to run.
static struct fibre_queue ready;

// A mapping between file descriptors and fibre_vecors of fibres waiting for them
// to be ready.
static struct fibre_hashmap fdwatchers;

// A mapping between file descriptors and the current event bitset.
static struct fibre_hashmap fdevents;

static int epollfd;

static void spawn_entry(struct coro *c, fibre_func func)
{
	void *arg = coro_yield(c, 0);
	coro_yield(c, 0);
	func(arg);
}

int fibre_spawn(fibre_func func, void *arg)
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
	if (fibre_queue_add(&ready, &f) < 0) {
		LOG("created and scheduled %p", f);
		goto cleanup_fibre;
	}
	return 0;

cleanup_fibre:
	free(f);
	return -1;
}

void fibre_yield(struct epoll_event *ev)
{
	coro_yield(&curr->c, ev);
}

static int add_watcher(int fd, struct fibre *f)
{
	struct fibre_vec *v = fibre_hashmap_get(&fdwatchers, &fd);
	if (!v) {
		LOG("first time seeing fd %d, initialising...", fd);
		struct fibre_vec tmpv = { 0 };
		if (fibre_vec_init(&tmpv, CONTAINER_SIZE,
				   sizeof(struct fibre *)) < 0) {
			return -1;
		}
		if (fibre_hashmap_insert(&fdwatchers, &fd, &tmpv) < 0) {
			fibre_vec_free(&tmpv);
			return -1;
		}
		v = fibre_hashmap_get(&fdwatchers, &fd);
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &f->ev) < 0) {
			if (errno != EEXIST) {
				LOG("failed to register %d with epoll: %s", fd,
				    strerror(errno));
				return -1;
			}
		}
		if (fibre_hashmap_insert(&fdevents, &fd, &f->ev.events) < 0) {
			return -1;
		}
	}
	uint32_t eset = *(uint32_t *)fibre_hashmap_get(&fdevents, &fd);
	if (!(eset & f->ev.events)) {
		eset |= f->ev.events;
		struct epoll_event ev = f->ev;
		ev.events = eset;
		if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
			return -1;
		}
		if (fibre_hashmap_insert(&fdevents, &fd, &eset) < 0) {
			return -1;
		}
	}
	LOG("adding %p to watchers of %d", f, fd);
	return fibre_vec_append(v, &f);
}

static int notify_watchers()
{
	struct epoll_event events[8192];
	int nfds;
	if ((nfds = epoll_wait(epollfd, events, LEN(events), 0)) < 0) {
		return -1;
	}
	LOG("%d fds ready", nfds);
	for (int i = 0; i < nfds; i++) {
		struct epoll_event *ev = &events[i];
		int fd = ev->data.fd;
		struct fibre_vec *watchers =
			fibre_hashmap_get(&fdwatchers, &fd);
		// TODO(cmgn): Work out why this happens.
		if (!watchers) {
			epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
			continue;
		}
		int i;
		LOG("finding suitable watcher for %d", fd);
		for (i = 0; i < fibre_vec_size(watchers); i++) {
			struct fibre *f =
				*(struct fibre **)fibre_vec_get(watchers, i);
			if (f->ev.events & ev->events) {
				break;
			}
		}
		if (i == fibre_vec_size(watchers)) {
			LOG("no suitable watcher for %d; unregistering event type",
			    fd);
			uint32_t eset =
				*(uint32_t *)fibre_hashmap_get(&fdevents, &fd);
			eset &= ~ev->events;
			ev->events = eset;
			if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, ev) < 0) {
				return -1;
			}
			fibre_hashmap_insert(&fdevents, &fd, &eset);
			continue;
		}
		struct fibre *f = *(struct fibre **)fibre_vec_get(watchers, i);
		LOG("found suitable watcher for %d: %p", fd, f);
		fibre_queue_add(&ready, &f);
		fibre_vec_delete(watchers, i);
		if (fibre_vec_empty(watchers)) {
			fibre_vec_free(watchers);
			fibre_hashmap_delete(&fdwatchers, &fd);
			fibre_hashmap_delete(&fdevents, &fd);
			epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
		}
	}
	return 0;
}

static int run_ready()
{
	int nready = fibre_queue_size(&ready);
	LOG("%d fibres ready", nready);
	for (int i = 0; i < nready; i++) {
		struct fibre *f;
		fibre_queue_poll(&ready, &f);
		curr = f;
		LOG("resuming %p", f);
		struct epoll_event *ev = coro_resume(&f->c);
		if (coro_done(&f->c)) {
			LOG("%p finished; freeing", f);
			coro_free(&f->c);
			free(f);
			continue;
		}
		if (!ev) {
			LOG("%p fibre_yielded with no epoll event; adding to ready",
			    f);
			if (fibre_queue_add(&ready, &f) < 0) {
				return -1;
			}
			continue;
		}
		LOG("%p fibre_yielded with an epoll event", f);
		memcpy(&f->ev, ev, sizeof(*ev));
		if (add_watcher(ev->data.fd, f) < 0) {
			return -1;
		}
	}
	return 0;
}

int fibre_start(fibre_func entry, void *arg)
{
	int status = 0;
	if (fibre_queue_init(&ready, CONTAINER_SIZE, sizeof(struct fibre *)) <
	    0) {
		status = -1;
		goto failure;
	}
	if (fibre_hashmap_init(&fdwatchers, CONTAINER_SIZE, sizeof(int),
			       sizeof(struct fibre_queue)) < 0) {
		status = -1;
		goto cleanup_fibre_queue;
	}
	if (fibre_hashmap_init(&fdevents, CONTAINER_SIZE, sizeof(int),
			       sizeof(uint32_t)) < 0) {
		status = -1;
		goto cleanup_fdwatchers;
	}
	if ((epollfd = epoll_create1(0)) < 0) {
		status = -1;
		goto cleanup_fdevents;
	}
	if (fibre_spawn(entry, arg) < 0) {
		status = -1;
		goto cleanup_epoll;
	}
	for (;;) {
		if (fibre_queue_empty(&ready)) {
			struct epoll_event ev;
			epoll_wait(epollfd, &ev, 1, -1);
		}
		if (run_ready() < 0) {
			status = -1;
			goto cleanup_epoll;
		}
		if (notify_watchers() < 0) {
			status = -1;
			goto cleanup_epoll;
		}
	}
cleanup_epoll:
	close(epollfd);
cleanup_fdevents:
	fibre_hashmap_free(&fdevents);
cleanup_fdwatchers:; // Semi-colon to allow declaration.
	struct fibre_hashmap_iter it;
	fibre_hashmap_iter_init(&it, &fdwatchers);
	struct fibre_vec *v;
	while ((v = fibre_hashmap_iter_next(&it))) {
		fibre_vec_free(v);
	}
	fibre_hashmap_free(&fdwatchers);
cleanup_fibre_queue:
	fibre_queue_free(&ready);
failure:
	return status;
}
