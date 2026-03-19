#ifndef SYSCALL_H
#define SYSCALL_H
#include "../kernel/types.h"

#define SYSCALL_EXIT 1
#define SYSCALL_FORK 2
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_OPEN 5
#define SYSCALL_CLOSE 6
#define SYSCALL_PS 7
#define SYSCALL_EXEC 11
#define SYSCALL_KILL 12
#define SYSCALL_WAIT 13
#define SYSCALL_CREATE 14
#define SYSCALL_POLL 15
#define SYSCALL_SLEEP 16
#define SYSCALL_UPTIME 17
#define SYSCALL_CREATE_WINDOW 18
#define SYSCALL_DRAW_RECT 19
#define SYSCALL_DRAW_TEXT 20
#define SYSCALL_DRAW_EVENT 21
#define SYSCALL_DESTROY_WINDOW 22

void init_syscall();
void syscall_handler_main(registers_t *regs);

#endif
