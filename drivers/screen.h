#ifndef SCREEN_H
#define SCREEN_H

extern volatile unsigned short *vga_buffer;
extern int vga_index;

void update_cursor(int x, int y);
void print_char(char c);
void print(const char *str);
void clear_screen();

#endif
