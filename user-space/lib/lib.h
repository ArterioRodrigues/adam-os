#ifndef HELPERS_H
#define HELPERS_H
#include "types.h"

extern void sys_exit();
extern void sys_exec(char *buf);
extern void sys_write(int fd, char *buf, int len);

extern int sys_fork();
extern int sys_close(int fd);
extern int sys_open(char *filename);
extern int sys_read(int fd, char *buf, int len);
extern int sys_ps(ps_entry_t *buf, int max);
extern void sys_kill(int pid);

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
