#ifndef WINDOW_H
#define WINDOW_H

#include "event.h"
#include "../../shared/config.h"
#include "../../shared/types.h"

typedef struct window {
    uint32_t window_id;
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
extern uint32_t window_id;
extern bool wm_dirty;

window_t *wm_create_window(int x, int y, uint32_t width, uint32_t height, char *title, int pid);
void wm_composite();
void update_window();
void update_focused_window(event_type_t type, uint8_t scancode, char c, uint8_t mouse_button, int mouse_x, int mouse_y);

window_t *get_window(uint32_t id);

uint32_t window_count();
void window_draw_string(window_t *window, create_text_t *text);

void window_put_pixel(window_t *window, int x, int y, uint8_t color);
void window_draw_rect(window_t *window, int x, int y, int w, int h, uint8_t color);
void window_draw_line(window_t *window, int x0, int y0, int x1, int y1, uint8_t color);
void window_draw_char(window_t *window, int x, int y, char c, uint8_t color);
void remove_window(uint32_t id);
void remove_window_process(uint32_t pid);
#endif
