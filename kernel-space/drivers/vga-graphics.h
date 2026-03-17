#ifndef VGA_H
#define VGA_H
#include "../kernel/types.h"

extern volatile uint8_t *vga_graphics_buffer; 
void vga_put_pixel(int x, int y, uint8_t color);
void vga_clear_screen(uint8_t color);
void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_rect_outline(int x, int y, int w, int h, uint8_t outline_color, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, char *c, uint8_t color);
void vga_flip();
#endif
