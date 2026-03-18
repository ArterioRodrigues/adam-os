#ifndef MOUSE_H
#define MOUSE_H
#include "../kernel/types.h"

extern int mouse_x;
extern int mouse_y;
extern int prev_mouse_x;
extern int prev_mouse_y;
extern int dx;
extern int dy;
extern int drag_offset_x;
extern int drag_offset_y;
extern uint8_t mouse_buttons;
extern uint8_t prev_mouse_buttons;

extern void mouse_handler();
void init_mouse();

#endif
