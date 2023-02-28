#ifndef _FIBRE_IO_H
#define _FIBRE_IO_H

#include <sys/types.h>

extern ssize_t fibre_write(int fd, const char *buf, unsigned long n);
extern ssize_t fibre_read(int fd, char *buf, unsigned long n);
extern int fibre_accept(int fd);
extern int fibre_sleep(long ms);

#endif
