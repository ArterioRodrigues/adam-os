#ifndef HELPERS_H
#define HELPERS_H

extern void sys_write(char *buf, int len);
extern char sys_read(int fd, char *buf, int len);

void wait();
#endif
