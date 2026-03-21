#include "../pch.h"

volatile uint8_t *vga_graphics_buffer;
uint8_t *back_buffer = NULL;

uint32_t font_row = 8;
uint32_t font_col = 8;
uint32_t font_spacing = 9;

uint32_t screen_width;
uint32_t screen_height;

uint32_t screen_pitch;
uint32_t screen_size;
uint32_t dirty_y_min;
uint32_t dirty_y_max;

void mark_dirty(int y_top, int y_bottom) {
    if (y_top < 0)
        y_top = 0;
    if (y_bottom >= (int)screen_height)
        y_bottom = screen_height - 1;

    if (y_top < dirty_y_min)
        dirty_y_min = y_top;
    if (y_bottom > dirty_y_max)
        dirty_y_max = y_bottom;
}

void vga_put_pixel(int x, int y, uint8_t color) {
    mark_dirty(y, y);
    if (x >= screen_width || x < 0 || y >= screen_height || y < 0)
        return;
    uint32_t index = (y * screen_pitch) + x;
    back_buffer[index] = color;
}

void vga_clear_screen(uint8_t color) {
    mark_dirty(0, screen_height - 1);
    memset((void *)back_buffer, color, screen_size);
}

void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    mark_dirty(y, y + h - 1);
    for (int i = x; i < x + w; i++) {
        for (int j = y; j < y + h; j++) {
            vga_put_pixel(i, j, color);
        }
    }
}

void vga_draw_rect_outline(int x, int y, int w, int h, uint8_t outline_color, uint8_t color) {
    vga_draw_rect(x, y, w, h, outline_color);
    vga_draw_rect(x + 1, y + 1, w - 2, h - 2, color);
}

void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int error = dx - dy;

    while (!(x0 == x1 && y0 == y1)) {
        vga_put_pixel(x0, y0, color);
        int error2 = 2 * error;

        if (error2 > -dy) {
            error -= dy;
            x0 += sx;
        }
        if (error2 < dx) {
            error += dx;
            y0 += sy;
        }
    }
}

void vga_draw_char(int x, int y, char c, uint8_t color) {
    mark_dirty(y, y + font_row);
    uint8_t *glyph = font8x8[c - 32];

    for (int row = 0; row < font_row; row++) {
        uint8_t bits = glyph[row];

        for (int col = 0; col < font_col; col++) {
            if (bits & (0x80 >> col))
                vga_put_pixel(x + col, y + row, color);
        }
    }
}

void vga_draw_string(int x, int y, char *c, uint8_t color) {
    int size = strlen(c);
    for (int i = 0; i < size; i++) {
        vga_draw_char(x + (i * font_spacing ) , y, c[i], color);
    }
}

void vga_flip() {
    if (dirty_y_min > dirty_y_max)
        return;

    uint32_t offset = dirty_y_min * screen_pitch;
    uint32_t size = (dirty_y_max - dirty_y_min + 1) * screen_pitch;
    memcpy((void *)(vga_graphics_buffer + offset), back_buffer + offset, size);

    dirty_y_min = screen_height;
    dirty_y_max = 0;
}
void vga_draw_cursor(int x, int y, uint8_t color) {
    uint8_t *outline, *fill;
    if (current_cursor == CURSOR_POINTER) {
        outline = pointer_outline;
        fill = pointer_fill;
    } else {
        outline = cursor_outline;
        fill = cursor_fill;
    }

    for (int row = 0; row < 18; row++) {
        uint16_t o = (outline[row * 2] << 8) | outline[row * 2 + 1];
        uint16_t f = (fill[row * 2] << 8) | fill[row * 2 + 1];
        for (int col = 0; col < 16; col++) {
            uint16_t bit = 0x8000 >> col;
            if (o & bit)
                vga_put_pixel(x + col, y + row, 0xF);
            if (f & bit)
                vga_put_pixel(x + col, y + row, color);
        }
    }
}
void init_vga() {
    screen_width = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x12);
    screen_height = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x14);
    screen_pitch = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x10);
    dirty_y_min = screen_height;
    dirty_y_max = 0;

    vga_graphics_buffer = (volatile uint8_t *)(*(uint32_t *)(VBE_MODE_INFO_ADDR + 0x28));

    screen_size = screen_pitch * screen_height;
    back_buffer = kmalloc(screen_size);
    memset(back_buffer, 0, screen_size);
}
void vga_blit(int x, int y, uint32_t width, uint32_t height, uint8_t *buffer) {
    mark_dirty(y, y + height - 1);
    for (uint32_t row = 0; row < height; row++) {
        int dest_y = y + row;
        if (dest_y < 0 || dest_y >= (int)screen_height)
            continue;

        int src_x = 0, dst_x = x;
        int copy_width = width;

        if (dst_x < 0) {
            src_x = -dst_x;
            copy_width += dst_x;
            dst_x = 0;
        }
        if (dst_x + copy_width > (int)screen_width)
            copy_width = screen_width - dst_x;
        if (copy_width <= 0)
            continue;

        memcpy(back_buffer + dest_y * screen_pitch + dst_x, buffer + row * width + src_x, copy_width);
    }
}
