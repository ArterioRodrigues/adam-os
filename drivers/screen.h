#ifndef SCREEN_H
#define SCREEN_H


extern volatile unsigned short *vga_buffer;
extern int vga_index;

void update_cursor(int x, int y);
void print_char_color(char c, unsigned char color);
void print_color(const char *str, unsigned char color);
void clear_screen();

#define print(str, ...) print_color(str, WHITE)
#define print_char(c, ...) print_char_color(c, WHITE)

#endif
