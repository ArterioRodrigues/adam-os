#ifndef WINDOW_H
#define WINDOW_H
#include "config.h"
#include "event.h"
#include "types.h"

typedef struct window {
    int x;
    int y;

    uint32_t width;
    uint32_t height;

    uint8_t *pixel_buffer;

    char title[32];
    uint32_t pid;
    uint32_t z_index;

    bool is_visible;
    bool is_focused;
    bool is_dragging;

    struct window *next;
    event_queue_t event_queue;

} window_t;

extern window_t *window_head_ptr;
extern uint32_t window_z_index;

window_t *wm_create_window(int x, int y, uint32_t width, uint32_t height, char *title, int pid);
void wm_composite();
void update_window();
void update_focused_window(event_type_t type, uint8_t scancode, char c, uint8_t mouse_button, int mouse_x, int mouse_y);
#endif
