#include <unistd.h>

#include <sys/epoll.h>
#include <sys/socket.h>

#include "fibre.h"
#include "fibre_io.h"

int fibre_write(int fd, const char *buf, unsigned long n)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLOUT;
	yield(&ev);
	return write(fd, buf, n);
}

int fibre_read(int fd, char *buf, unsigned long n)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	yield(&ev);
	return read(fd, buf, n);
}

int fibre_accept(int fd)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	yield(&ev);
	return accept(fd, 0, 0);
}
