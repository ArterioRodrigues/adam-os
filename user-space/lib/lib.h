#ifndef HELPERS_H
#define HELPERS_H

extern void sys_write(char *buf, int len);
extern char sys_read(int fd, char *buf, int len);
extern int sys_fork();
extern void sys_exit();
extern void sys_exec(char *buf);

void wait();
#endif
