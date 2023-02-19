#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "third_party/coro/coro.h"

#define STACK_SIZE (8 * 1024 * 1024)
#define MAX_TASKS 8192

struct task {
	struct coro c;
};

struct sched {
	struct task **tasks;
	int cap;
	int r;
	int w;
	int size;
};

int sched_schedule(struct sched *s, struct task *t)
{
	if (s->size == s->cap) {
		return -1;
	}
	s->tasks[s->w] = t;
	s->w = (s->w + 1) % s->cap;
	s->size++;
	return 0;
}

struct task *sched_get_task(struct sched *s)
{
	if (s->size == 0) {
		return 0;
	}
	struct task *t = s->tasks[s->r];
	s->r = (s->r + 1) % s->cap;
	s->size--;
	return t;
}

void sched_run(struct sched *s)
{
	while (s->size) {
		struct task *t = sched_get_task(s);
		coro_resume(&t->c);
		if (!coro_done(&t->c)) {
			sched_schedule(s, t);
		} else {
			coro_free(&t->c);
			free(t);
		}
	}
}

struct sched global_sched = { 0 };

void spawn(coro_func fn, void *arg)
{
	struct task *t = calloc(1, sizeof(*t));
	if (!t) {
		perror("malloc");
		exit(1);
	}
	void *stack = calloc(1, STACK_SIZE);
	if (!stack) {
		perror("malloc");
		exit(1);
	}
	coro_init(&t->c, stack, STACK_SIZE, fn, arg);
	sched_schedule(&global_sched, t);
}

void streamof(struct coro *c, void *arg) {
	printf("%ld\n", (int64_t)arg);
	spawn(streamof, (void *)((int64_t)arg) + 1);
}

void run() {
	sched_run(&global_sched);
}

int main()
{
	global_sched.tasks = calloc(1, sizeof(struct task *) * MAX_TASKS);
	if (!global_sched.tasks) {
		perror("malloc");
		return 1;
	}
	global_sched.cap = MAX_TASKS;
	spawn(streamof, (void *)5);
	run();
	return 0;
}
