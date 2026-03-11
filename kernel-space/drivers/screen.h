#ifndef SCREEN_H
#define SCREEN_H
#include "../kernel/types.h"

extern volatile unsigned short *vga_buffer;
extern int vga_index;
extern int vga_cursor_floor;

extern uint8_t ansi_to_vga[20];

void update_cursor(int x, int y);
void print_char_color(char c, unsigned char color);
void print_color(const char *str, unsigned char color);
void clear_screen();
void print_many(const char *first, ...);
void print_at(const char *str, int x, int y, unsigned char color);

#define print(str, ...) print_color(str, WHITE)
#define print_char(c, ...) print_char_color(c, WHITE)

#define printf(...) print_many(__VA_ARGS__, NULL)
#endif
