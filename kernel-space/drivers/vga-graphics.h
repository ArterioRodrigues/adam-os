#ifndef VGA_H
#define VGA_H
#include "../../shared/types.h"

#define CURSOR_H 18
#define CURSOR_W 16

extern volatile uint8_t *vga_graphics_buffer;
extern uint8_t *back_buffer;
extern uint32_t font_row;
extern uint32_t font_col;
extern uint32_t screen_width;
extern uint32_t screen_height;
extern uint32_t screen_pitch;
extern uint32_t screen_size;
extern uint32_t dirty_y_min, dirty_y_max;


void mark_dirty(int y_top, int y_bottom);
void vga_put_pixel(int x, int y, uint8_t color);
void vga_clear_screen(uint8_t color);
void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_rect_outline(int x, int y, int w, int h, uint8_t outline_color, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, char *c, uint8_t color);
void vga_flip();
void vga_draw_cursor(int x, int y, uint8_t color);
void vga_blit(int x, int y, uint32_t width, uint32_t height, uint8_t *buffer);
void init_vga();
#endif
