#include "window.h"
#include "../pch.h"
#include "event.h"

#define PADDING 4
window_t *window_head_ptr = NULL;
uint32_t window_z_index = 1;

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
    window->pixel_buffer = kmalloc(width * height);

    memset(window->pixel_buffer, window_z_index, width * height);
    strcpy(window->title, title);

    window->pid = pid;
    window->z_index = window_z_index;
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
    window_z_index++;
    return window;
}

void draw_window(window_t *window) {
    uint8_t focus_color = window->is_focused ? 0x0 : 0x99;

    vga_draw_rect(window->x - 1, window->y - 1, window->width + 2, window->height + 2, focus_color);

    vga_blit(window->x, window->y, window->width, window->height, window->pixel_buffer);

    vga_draw_rect(window->x, window->y, window->width, 12, focus_color);
    vga_draw_string(window->x + PADDING, window->y + PADDING, window->title, 0xF);
    vga_draw_string(window->x + window->width - PADDING - 12, window->y + PADDING, "[X]", 0xF);
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
    vga_draw_cursor(mouse_x, mouse_y, 0xF);
    vga_flip();
}

void update_window() {
    window_t *window = window_head_ptr;
    window_t *focused_window = NULL;
    uint32_t z_index = 0;

    while (window) {
        if (!((prev_mouse_x >= window->x && prev_mouse_x < window->x + window->width) &&
              (prev_mouse_y >= window->y && prev_mouse_y < window->y + window->height))) {
            window->is_focused = window->is_focused ? true : false;
            window = window->next;
            continue;
        }

        if (prev_mouse_buttons & 1 && (z_index <= window->z_index)) {
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
        if (prev_mouse_y < focused_window->y + 12) {
            drag_offset_x = prev_mouse_x - focused_window->x;
            drag_offset_y = prev_mouse_y - focused_window->y;
            focused_window->is_dragging = true;
        }
        if ((mouse_buttons & 1) && !(prev_mouse_buttons & 1))
            update_focused_window(EVENT_MOUSE_CLICK, 0, 0, mouse_buttons, mouse_x - focused_window->x,
                                  mouse_y - focused_window->y);
        if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y)
            update_focused_window(EVENT_MOUSE_MOVE, 0, 0, mouse_buttons, mouse_x - focused_window->x,
                                  mouse_y - focused_window->y);
    }

    window = window_head_ptr;
    while (window) {
        if (window->is_dragging) {
            window->x = mouse_x - drag_offset_x;
            window->y = mouse_y - drag_offset_y;
        }

        if (!(mouse_buttons & 1))
            window->is_dragging = false;

        window = window->next;
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
