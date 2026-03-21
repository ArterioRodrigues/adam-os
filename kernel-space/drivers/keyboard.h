#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../kernel/config.h"
#include "../../shared/types.h"

#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LSHIFT_REL 0xAA
#define SCANCODE_RSHIFT_REL 0xB6
#define SCANCODE_END 0x4F

extern bool keyboard_pressed;
extern void keyboard_handler();

void keyboard_handler_main(registers_t *regs);
void init_keyboard();
#endif
