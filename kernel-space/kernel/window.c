#include "../pch.h"

#define PADDING 5
#define HEADER_OFFSET 20
#define MAX_WINDOWS 32
#define CLOSE_BTN_W 30
#define TITLE_BAR_H 12

window_t *window_head_ptr = NULL;
bool wm_dirty = false;
uint32_t window_z_index = 1;
uint32_t window_id = 1;

static inline bool point_in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

static uint32_t window_count() {
    uint32_t n = 0;
    for (window_t *w = window_head_ptr; w; w = w->next)
        n++;
    return n;
}

window_t *wm_get_focused_window() {
    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->is_focused)
            return w;
    }
    return NULL;
}

window_t *get_window(uint32_t id) {
    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->window_id == id)
            return w;
    }
    return NULL;
}

window_t *wm_create_window(int x, int y, uint32_t width, uint32_t height, char *title, int pid) {
    window_t *window = kmalloc(sizeof(window_t));
    memset(window, 0, sizeof(window_t));

    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;

    uint32_t buf_size = width * (height + HEADER_OFFSET);
    window->pixel_buffer = kmalloc(buf_size);
    memset(window->pixel_buffer, 0, buf_size);

    strcpy(window->title, title);
    window->pid = pid;
    window->z_index = window_z_index++;
    window->window_id = window_id++;
    window->is_visible = true;

    window->next = NULL;
    if (!window_head_ptr) {
        window_head_ptr = window;
    } else {
        window_t *tail = window_head_ptr;
        while (tail->next)
            tail = tail->next;
        tail->next = window;
    }

    return window;
}

void remove_window(uint32_t id) {
    window_t *prev = NULL;
    window_t *window = window_head_ptr;

    while (window) {
        if (window->window_id == id)
            break;
        prev = window;
        window = window->next;
    }

    if (!window)
        return;

    if (prev)
        prev->next = window->next;
    else
        window_head_ptr = window->next;

    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->z_index > window->z_index)
            w->z_index--;
    }
    window_z_index--;

    kfree(window->pixel_buffer);
    kfree(window);
}

static void draw_window(window_t *window) {
    uint8_t border_color = window->is_focused ? 0x0 : 0x99;
    int total_h = window->height + HEADER_OFFSET;

    vga_draw_rect(window->x - 1, window->y - 1, window->width + 2, total_h + 2, border_color);

    vga_blit(window->x, window->y, window->width, total_h, window->pixel_buffer);

    vga_draw_rect(window->x, window->y, window->width, HEADER_OFFSET - 5, border_color);
    vga_draw_string(window->x + PADDING, window->y + PADDING, window->title, 0xF);
    vga_draw_string(window->x + window->width - PADDING - 15, window->y + PADDING, "[X]", 0xF);
}

void wm_composite() {
    vga_clear_screen(0x64);

    uint32_t count = window_count();
    if (count == 0)
        goto cursor;

    window_t *sorted[MAX_WINDOWS];
    memset(sorted, 0, sizeof(sorted));

    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->z_index > 0 && w->z_index <= MAX_WINDOWS)
            sorted[w->z_index - 1] = w;
    }

    window_t *focused = NULL;
    for (uint32_t i = 0; i < count; i++) {
        window_t *w = sorted[i];
        if (!w || !w->is_visible)
            continue;
        if (w->is_focused) {
            focused = w;
            continue;
        }
        draw_window(w);
    }

    if (focused)
        draw_window(focused);

cursor:
    vga_draw_cursor(mouse_x, mouse_y, BLACK);
    vga_flip();
}

void window_put_pixel(window_t *window, int x, int y, uint8_t color) {
    if ((uint32_t)x >= window->width || x < 0 || (uint32_t)y >= window->height || y < 0)
        return;
    window->pixel_buffer[((y + HEADER_OFFSET) * window->width) + x] = color;
}

void window_draw_rect(window_t *window, int x, int y, int w, int h, uint8_t color) {
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > (int)window->width) ? (int)window->width : x + w;
    int y1 = (y + h > (int)window->height) ? (int)window->height : y + h;

    int clipped_w = x1 - x0;
    if (clipped_w <= 0 || y0 >= y1)
        return;

    for (int row = y0; row < y1; row++) {
        uint32_t offset = ((row + HEADER_OFFSET) * window->width) + x0;
        memset(&window->pixel_buffer[offset], color, clipped_w);
    }
}

void window_draw_char(window_t *window, int x, int y, char c, uint8_t color) {
    if (c < 32)
        return;
    uint8_t *glyph = font5x7[c - 32];
    for (int row = 0; row < 7; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 5; col++) {
            if (bits & (0x80 >> col))
                window_put_pixel(window, x + col, y + row, color);
        }
    }
}

void window_draw_string(window_t *window, create_text_t *text) {
    for (int i = 0; text->str[i]; i++)
        window_draw_char(window, text->x + (i * 6), text->y, text->str[i], text->color);
}

void update_focused_window(event_type_t type, uint8_t scancode, char c, uint8_t mouse_button, int mx, int my) {
    window_t *window = wm_get_focused_window();
    if (!window)
        return;

    event_t event;
    event.type = type;

    switch (type) {
    case EVENT_KEYPRESS:
        event.keyboard_scancode = scancode;
        event.c = c;
        break;
    case EVENT_MOUSE_MOVE:
    case EVENT_MOUSE_CLICK:
        event.mouse_button = mouse_button;
        event.mouse_x = mx;
        event.mouse_y = my;
        break;
    default:
        break;
    }

    event_queue_push(&window->event_queue, event);
}

void update_window() {
    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->is_dragging) {
            if (!(mouse_buttons & 1)) {
                w->is_dragging = false;
            } else {
                w->x = mouse_x - drag_offset_x;
                w->y = mouse_y - drag_offset_y;
            }
        }
    }

    if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y)
        update_focused_window(EVENT_MOUSE_MOVE, 0, 0, mouse_buttons, mouse_x, mouse_y);

    if (!((mouse_buttons & 1) && !(prev_mouse_buttons & 1)))
        return;

    window_t *clicked = NULL;
    uint32_t top_z = 0;

    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (!point_in_rect(prev_mouse_x, prev_mouse_y, w->x, w->y, w->width, w->height + HEADER_OFFSET))
            continue;
        if (w->z_index >= top_z) {
            top_z = w->z_index;
            clicked = w;
        }
    }

    if (!clicked)
        return;

    for (window_t *w = window_head_ptr; w; w = w->next) {
        if (w->z_index > clicked->z_index)
            w->z_index--;
        w->is_focused = false;
    }
    clicked->is_focused = true;
    clicked->z_index = window_z_index - 1;

    if (prev_mouse_y < clicked->y + TITLE_BAR_H && prev_mouse_x > clicked->x + (int)clicked->width - CLOSE_BTN_W) {
        remove_window(clicked->window_id);
        return;
    }

    if (prev_mouse_y < clicked->y + TITLE_BAR_H) {
        drag_offset_x = prev_mouse_x - clicked->x;
        drag_offset_y = prev_mouse_y - clicked->y;
        clicked->is_dragging = true;
    }

    update_focused_window(EVENT_MOUSE_CLICK, 0, 0, mouse_buttons, mouse_x - clicked->x, mouse_y - clicked->y);
}
