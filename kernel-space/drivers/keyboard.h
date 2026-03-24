#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../kernel/config.h"
#include "../../shared/types.h"

#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LSHIFT_REL 0xAA
#define SCANCODE_RSHIFT_REL 0xB6
#define SCANCODE_END 0x4F
#define SCANCODE_LCTRL 0x1D
#define SCANCODE_LCTRL_REL 0x9D
#define SCANCODE_C 0x2E

extern bool keyboard_pressed;
extern void keyboard_handler();

void keyboard_handler_main(registers_t *regs);
void init_keyboard();
#endif
