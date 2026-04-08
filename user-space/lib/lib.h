#ifndef HELPERS_H
#define HELPERS_H
#include "../../shared/abi-types.h"
#include "../../shared/math.h"
#include "../../shared/mem.h"
#include "../../shared/string.h"
#include "../../shared/types.h"
#include "../../shared/config.h"

extern void sys_exit();
extern void sys_exec(char *filname, char *buf);
extern void sys_write(int fd, char *buf, int len);

extern int sys_fork();
extern int sys_close(int fd);
extern int sys_open(char *filename);
extern int sys_read(int fd, char *buf, int len);
extern int sys_ps(ps_entry_t *buf, int max);
extern void sys_kill(int pid);
extern void sys_waitpid(int pid);
extern int sys_create(char *name, char *data, int len);
extern int sys_poll(int fd);
extern void sys_sleep(int ticks);
extern int sys_uptime();
extern int sys_create_window(create_window_t *window);
extern void sys_create_rect(create_rect_t *rect);
extern void sys_create_text(create_text_t *text);
extern int sys_get_event(uint32_t id, event_t *event);
extern void sys_destroy_window(uint32_t id);
extern void sys_flush();
extern uint32_t sys_getpid();
extern uint32_t sys_getppid();
extern void *sys_sbrk(uint32_t offset);

void print(const char *s);
#endif
