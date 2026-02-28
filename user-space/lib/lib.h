#ifndef HELPERS_H
#define HELPERS_H
#include "types.h"

extern void sys_exit();
extern void sys_exec(char *buf);
extern void sys_write(int fd, char *buf, int len);

extern int sys_fork();
extern int sys_close(int fd);
extern int sys_open(int fd, char *buf, int len);
extern int sys_read(int fd, char *buf, int len);

static void wait() {
    int i = 0;
    int j = 0;
    while (i < 10000) {
        while (j < 10000)
            j++;
        i++;
        j = 0;
    }
}

#endif
