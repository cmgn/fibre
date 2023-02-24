#ifndef _FIBRE_IO_H
#define _FIBRE_IO_H

extern int fibre_write(int fd, const char *buf, unsigned long n);
extern int fibre_read(int fd, char *buf, unsigned long n);
extern int fibre_accept(int fd);

#endif
