#include "../pch.h"

volatile uint8_t *vga_graphics_buffer;
uint8_t *back_buffer = NULL;

uint32_t font_row = 4;
uint32_t font_col = 4;

uint32_t screen_width;
uint32_t screen_height;
uint32_t screen_pitch;
uint32_t screen_size;

void vga_put_pixel(int x, int y, uint8_t color) {
    if (x >= screen_width || x < 0 || y >= screen_height || y < 0)
        return;
    uint32_t index = (y * screen_pitch) + x;
    vga_graphics_buffer[index] = color;
}

void vga_clear_screen(uint8_t color) { memset((void *)vga_graphics_buffer, color, screen_size); }

void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
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
    uint8_t *glyph = font4x4[c - 32];
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
        vga_draw_char(x + (i * font_row), y, c[i], color);
    }
}

void vga_flip() { rep_memcpy((uint8_t *)vga_graphics_buffer, (uint8_t *)vga_graphics_buffer, screen_size); }

void vga_draw_cursor(int x, int y, uint8_t color) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++)
            if (mouse_cursor[row] & (0x80 >> col))
                vga_put_pixel(x + col, y + row, color);
    }
}

void init_vga() {
    screen_width = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x12);
    screen_height = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x14);
    screen_pitch = *(uint16_t *)(VBE_MODE_INFO_ADDR + 0x10);
    vga_graphics_buffer = (volatile uint8_t *)(*(uint32_t *)(VBE_MODE_INFO_ADDR + 0x28));
    screen_size = screen_pitch * screen_height;
    back_buffer = kmalloc(screen_size);
    memset(back_buffer, 0, screen_size);
}

void vga_blit(int x, int y, uint32_t width, uint32_t height, uint8_t *buffer) {
    uint32_t size = width * height;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++)
            vga_put_pixel(x + i, y + j, buffer[(j * width) + i]);
    }
}
