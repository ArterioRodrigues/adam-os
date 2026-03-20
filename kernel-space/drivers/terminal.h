#ifndef TERMINAL_H
#define TERMINAL_H

#include "../kernel/window.h"

#define TERMINAL_FONT_W 7 
#define TERMINAL_FONT_H 9 
#define TERMINAL_WIN_W 360
#define TERMINAL_WIN_H 300
#define TERMINAL_COLS (TERMINAL_WIN_W / TERMINAL_FONT_W)
#define TERMINAL_ROWS ((TERMINAL_WIN_H) / TERMINAL_FONT_H)
#define TERMINAL_SIZE (TERMINAL_COLS * TERMINAL_ROWS)
#define TERMINAL_COLOR_SHIFT 8

typedef struct {
    window_t *window;
    uint32_t cursor_column;
    uint32_t cursor_row;
    uint32_t cursor_floor;
    uint16_t *buffer;
} terminal_t;

extern uint8_t ansi_to_terminal[20];

extern terminal_t *terminal;

void render_terminal();
void init_terminal();
void clear_terminal();
void terminal_print_char_color(char c, unsigned char color);
void terminal_print_color(const char *str, unsigned char color);
void terminal_print_at(const char *str, int x, int y, unsigned char color);
void terminal_print_many(const char *first, ...);

#define print(str, ...) terminal_print_color(str, WHITE)
#define print_char(c, ...) terminal_print_char_color(c, WHITE)
#define printf(...) terminal_print_many(__VA_ARGS__, NULL)

#endif
