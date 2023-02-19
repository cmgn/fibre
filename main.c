#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "third_party/coro/coro.h"

#define STACK_SIZE (8 * 1024 * 1024)

struct task {
	struct task *next;
	struct coro c;
};

struct sched {
	struct task *head;
	struct task *tail;
};

void sched_schedule(struct sched *s, struct task *t)
{
	if (!s->head) {
		s->head = t;
		s->tail = t;
		return;
	}
	s->tail->next = t;
	s->tail = t;
}

struct task *sched_get_task(struct sched *s)
{
	if (!s->head) {
		return 0;
	}
	struct  task *t = s->head;
	s->head = s->head->next;
	if (!s->head) {
		s->tail = 0;
	}
	t->next = 0;
	return t;
}

void sched_run(struct sched *s)
{
	while (s->head) {
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
	spawn(streamof, (void *)5);
	run();
	return 0;
}
