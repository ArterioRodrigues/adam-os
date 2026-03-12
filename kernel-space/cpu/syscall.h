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

#define RING3_CALL 0xEE

void init_syscall();
void syscall_handler_main(registers_t *regs);

#endif
