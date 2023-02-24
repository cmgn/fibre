#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/ip.h>
#include <sys/socket.h>

#include "fibre.h"
#include "fibre_io.h"

#define LEN(X) (sizeof(X) / sizeof(*X))

void handler(void *arg)
{
	int client = (int)(uintptr_t)arg;
	char buf[8192];
	for (;;) {
		int nread = fibre_read(client, buf, LEN(buf));
		if (nread < 0) {
			perror("read");
			goto cleanup;
		}
		if (nread == 0) {
			break;
		}
		int nwritten = 0;
		while (nwritten < nread) {
			int delta = fibre_write(client, buf + nwritten,
						nread - nwritten);
			if (delta < 0) {
				perror("write");
				goto cleanup;
			}
			nwritten += delta;
		}
	}
cleanup:
	close(client);
}

void run(void *arg)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 },
		       sizeof(int)) < 0) {
		perror("setsockopt");
		goto cleanup_socket;
	}
	struct sockaddr_in addr = (struct sockaddr_in){
		.sin_family = AF_INET,
		.sin_addr = { INADDR_ANY },
		.sin_port = htons(5000),
	};
	if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		goto cleanup_socket;
	}
	if (listen(sock, 32) < 0) {
		perror("listen");
		goto cleanup_socket;
	}
	for (;;) {
		int client = fibre_accept(sock);
		if (client < 0) {
			perror("accept");
			goto cleanup_socket;
		}
		if (fibre_spawn(handler, (void *)(uintptr_t)client) < 0) {
			perror("fibre_spawn");
			goto cleanup_socket;
		}
	}
	return;

cleanup_socket:
	close(sock);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (fibre_start(run, 0) < 0) {
		return 1;
	}
	return 0;
}
