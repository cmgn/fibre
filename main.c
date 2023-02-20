#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fibre.h"

struct cmdargs {
	int argc;
	char **argv;
};

void run(struct cmdargs *arg)
{
	if (arg->argc != 2) {
		puts("expected 1 arg");
		exit(EXIT_FAILURE);
	}
	printf("Hello, %s\n", arg->argv[1]);
}

int main(int argc, char **argv)
{
	struct cmdargs args = { .argc = argc, .argv = argv };
	start((fibre_func)run, &args);
	return 0;
}
