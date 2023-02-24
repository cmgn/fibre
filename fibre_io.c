#include <unistd.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

#include "fibre.h"
#include "fibre_io.h"

int fibre_write(int fd, const char *buf, unsigned long n)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLOUT;
	fibre_yield(&ev);
	return write(fd, buf, n);
}

int fibre_read(int fd, char *buf, unsigned long n)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	fibre_yield(&ev);
	return read(fd, buf, n);
}

int fibre_accept(int fd)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	fibre_yield(&ev);
	return accept(fd, 0, 0);
}

int fibre_sleep(long ms)
{
	int status = 0;
	int fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (fd < 0) {
		status = -1;
		goto failed;
	}
	struct itimerspec t = { 0 };
	t.it_value.tv_sec = ms / 1000;
	t.it_value.tv_nsec = 1000 * 1000 * (ms % 1000);
	if (timerfd_settime(fd, 0, &t, 0) < 0) {
		status = -1;
		goto cleanup_fd;
	}
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	fibre_yield(&ev);
cleanup_fd:
	close(fd);
failed:
	return status;
}
