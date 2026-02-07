#ifndef KEYBOARD_H
#define KEYBOARD_H

extern char keyboard_buffer[256];
extern int keyboard_buffer_index;
extern unsigned char keyboard_pressed;
extern void keyboard_handler();

void keyboard_handler_main();
void init_keyboard();
#endif
