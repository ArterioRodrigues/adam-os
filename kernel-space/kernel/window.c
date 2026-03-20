#include "window.h"
#include "../pch.h"
#include "event.h"
#include "scheduler.h"

#define PADDING 5 
#define HEADER_OFFSET 20
window_t *window_head_ptr = NULL;
bool wm_dirty = false;
uint32_t window_z_index = 1;
uint32_t window_id = 1;

window_t *wm_get_focused_window() {
    window_t *window = window_head_ptr;
    while (window) {
        if (window->is_focused)
            return window;

        window = window->next;
    }

    return NULL;
}

window_t *wm_create_window(int x, int y, uint32_t width, uint32_t height, char *title, int pid) {
    window_t *window = kmalloc(sizeof(window_t));

    window->x = x;
    window->y = y;

    window->width = width;
    window->height = height;
    window->pixel_buffer = kmalloc(width * (height + HEADER_OFFSET));

    memset(window->pixel_buffer, 0x0, width * (height + HEADER_OFFSET));
    strcpy(window->title, title);

    window->pid = pid;
    window->z_index = window_z_index++;
    window->window_id = window_id++;
    window->is_visible = true;
    window->is_focused = false;
    window->is_dragging = false;

    if (window_head_ptr == NULL)
        window_head_ptr = window;
    else {
        window_t *current = window_head_ptr;
        while (current->next)
            current = current->next;
        current->next = window;
    }

    window->next = NULL;
    return window;
}

void draw_window(window_t *window) {
    uint8_t focus_color = window->is_focused ? 0x0 : 0x99;

    vga_draw_rect(window->x - 1, window->y - 1, window->width + 2, window->height + HEADER_OFFSET + 2, focus_color);

    vga_blit(window->x, window->y, window->width, window->height + HEADER_OFFSET, window->pixel_buffer);

    vga_draw_rect(window->x, window->y, window->width, HEADER_OFFSET - 4, 0xF);
    vga_draw_rect(window->x, window->y, window->width, HEADER_OFFSET - 5, focus_color);
    vga_draw_string(window->x + PADDING, window->y + PADDING, window->title, 0xF);
    vga_draw_string(window->x + window->width - PADDING - 15, window->y + PADDING, "[X]", 0xF);
}

void wm_composite() {
    window_t *focus_window = NULL;
    vga_clear_screen(0x64);

    for (int i = 1; i < window_z_index; i++) {
        window_t *window = window_head_ptr;
        while (window->z_index != i) {
            window = window->next;
        }

        if (!window->is_visible || window->is_focused) {
            focus_window = window->is_focused ? window : NULL;
            continue;
        }

        draw_window(window);
    }

    if (focus_window) {
        draw_window(focus_window);
    }
    vga_draw_cursor(mouse_x, mouse_y, BLACK);
    vga_flip();
}

void update_window() {
    window_t *window = window_head_ptr;
    while (window) {
        if (window->is_dragging) {
            window->x = mouse_x - drag_offset_x;
            window->y = mouse_y - drag_offset_y;
        }

        if (!(mouse_buttons & 1))
            window->is_dragging = false;

        window = window->next;
    }

    if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y) {
        update_focused_window(EVENT_MOUSE_MOVE, 0, 0, mouse_buttons, mouse_x, mouse_y);
    }

    if (!((mouse_buttons & 1) && !(prev_mouse_buttons & 1)))
        return;

    window = window_head_ptr;
    window_t *focused_window = NULL;
    uint32_t z_index = 0;

    while (window) {
        if (!((prev_mouse_x >= window->x && prev_mouse_x < window->x + window->width) &&
              (prev_mouse_y >= window->y && prev_mouse_y < window->y + window->height))) {
            window->is_focused = window->is_focused ? true : false;
            window = window->next;
            continue;
        }

        if (mouse_buttons & 1 && (z_index <= window->z_index)) {
            z_index = window->z_index;
            focused_window = window;
        }

        window = window->next;
    }

    if (focused_window) {
        window = window_head_ptr;
        while (window) {
            if (window->z_index > focused_window->z_index)
                window->z_index = window->z_index - 1;

            window->is_focused = false;
            window = window->next;
        }

        focused_window->is_focused = true;
        focused_window->z_index = window_z_index - 1;

        if (prev_mouse_y < focused_window->y + 12 && prev_mouse_x > focused_window->x + focused_window->width - 30) {
            remove_window(focused_window->window_id);
            return;
        }

        if (prev_mouse_y < focused_window->y + 12) {
            drag_offset_x = prev_mouse_x - focused_window->x;
            drag_offset_y = prev_mouse_y - focused_window->y;
            focused_window->is_dragging = true;
        }
        update_focused_window(EVENT_MOUSE_CLICK, 0, 0, mouse_buttons, mouse_x - focused_window->x,
                              mouse_y - focused_window->y);
    }
}

void update_focused_window(event_type_t type, uint8_t scancode, char c, uint8_t mouse_button, int mouse_x,
                           int mouse_y) {
    window_t *window = wm_get_focused_window();
    if (!window)
        return;

    event_t event;
    event.type = type;

    switch (type) {
    case (EVENT_KEYPRESS):
        event.keyboard_scancode = scancode;
        event.c = c;
        break;

    case (EVENT_MOUSE_MOVE):
    case (EVENT_MOUSE_CLICK):
        event.mouse_button = mouse_button;
        event.mouse_x = mouse_x;
        event.mouse_y = mouse_y;
        break;
    }

    event_queue_push(&window->event_queue, event);
}

window_t *get_window(uint32_t id) {
    window_t *window = window_head_ptr;
    while (window) {
        if (window->window_id == id)
            return window;

        window = window->next;
    }

    return NULL;
}

void window_put_pixel(window_t *window, int x, int y, uint8_t color) {
    if (x >= window->width || x < 0 || y >= window->height || y < 0)
        return;

    y += HEADER_OFFSET;
    uint32_t index = (y * window->width) + x;
    window->pixel_buffer[index] = color;
}

void window_draw_rect(window_t *window, int x, int y, int w, int h, uint8_t color) {
    for (int i = x; i < x + w; i++) {
        for (int j = y; j < y + h; j++) {
            window_put_pixel(window, i, j, color);
        }
    }
}

void window_draw_char(window_t *window, int x, int y, char c, uint8_t color) {
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
    int size = strlen(text->str);
    for (int i = 0; i < size; i++) {
        window_draw_char(window, text->x + (i * 6), text->y, text->str[i], text->color);
    }
}
void remove_window(uint32_t id) {
    window_t *window = window_head_ptr;
    window_t *prev = NULL;

    while (window) {
        if (window->window_id == id)
            break;
        prev = window;
        window = window->next;
    }

    if (window == NULL)
        return;

    int pid = window->pid;
    if (prev == NULL)
        window_head_ptr = window->next;
    else
        prev->next = window->next;

    window_t *w = window_head_ptr;
    while (w) {
        if (w->z_index > window->z_index)
            w->z_index--;
        w = w->next;
    }

    window_z_index--;
    kfree(window->pixel_buffer);
    kfree(window);

}
