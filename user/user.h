#ifndef USER_H
#define USER_H

extern void jump_usermode();
extern void sys_write(char *buf, unsigned int length);
extern void sys_exit();

void test_user_function();
#endif
