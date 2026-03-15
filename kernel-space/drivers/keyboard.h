#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../kernel/config.h"
#include "../kernel/types.h"


extern bool keyboard_pressed;
extern void keyboard_handler();

void keyboard_handler_main(registers_t *regs);
void init_keyboard();
#endif
