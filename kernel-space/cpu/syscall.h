#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_EXIT 1
#define SYSCALL_FORK 2 
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_EXEC 11
#define RING3_CALL 0xEE

void init_syscall();

#endif
