#include "../pch.h"

uint8_t ansi_to_terminal[20] = {
    BLACK,          // #30
    RED,            // #31
    GREEN,          // #32
    BROWN,          // #33
    BLUE,           // #34
    MAGENTA,        // #35
    CYAN,           // #36
    LIGHT_GRAY,     // #37
    DARK_GRAY,      // #38
    BRIGHT_RED,     // #39
    BRIGHT_GREEN,   // #40
    YELLOW,         // #41
    BRIGHT_BLUE,    // #42
    BRIGHT_MAGENTA, // #43
    BRIGHT_CYAN,    // #44
    WHITE           // #45
};
terminal_t *terminal = NULL;
int cursor_index = 0;

void render_terminal() {
    window_t *window = terminal->window;
    memset(window->pixel_buffer, 0, window->width * window->height);
    for (int i = 0; i < TERMINAL_ROWS; i++) {
        for (int j = 0; j < TERMINAL_COLS; j++) {
            int index = (i * TERMINAL_COLS) + j;
            char ch = terminal->buffer[index] & 0xFF;
            uint8_t color = (terminal->buffer[index] >> 8) & 0xFF;
            if (ch > 0)
                window_draw_char(window, j * TERMINAL_FONT_W, (i * TERMINAL_FONT_H), ch, color);
        }
    }
    wm_composite();
}

void terminal_print_char_color(char c, unsigned char color) {
    uint32_t cursor_index = (TERMINAL_COLS * terminal->cursor_row) + terminal->cursor_column;

    if (cursor_index >= TERMINAL_SIZE) {
        for (int i = 0; i < TERMINAL_SIZE; i++) {
            if (i > TERMINAL_SIZE - TERMINAL_COLS)
                terminal->buffer[i] = (WHITE << TERMINAL_COLOR_SHIFT) | ' ';
            else
                terminal->buffer[i] = terminal->buffer[i + TERMINAL_COLS];
        }
        cursor_index = TERMINAL_SIZE - TERMINAL_COLS;
    }

    if (c == '\n')
        cursor_index = (cursor_index / TERMINAL_COLS + 1) * TERMINAL_COLS;

    else if (c == '\b') {
        if (cursor_index > 0 && cursor_index > terminal->cursor_floor) {
            cursor_index--;
            terminal->buffer[cursor_index] = (color << TERMINAL_COLOR_SHIFT) | ' ';
        }
    }

    else if (c == '\t') {
        int spacesToAdd = TAB_SIZE - (cursor_index % TAB_SIZE);

        for (int i = 0; i < spacesToAdd; i++) {
            terminal->buffer[cursor_index] = (color << TERMINAL_COLOR_SHIFT) | ' ';
            cursor_index++;
        }
    }

    else {
        terminal->buffer[cursor_index] = (color << TERMINAL_COLOR_SHIFT) | c;
        cursor_index++;
    }

    terminal->cursor_row = cursor_index / TERMINAL_COLS;
    terminal->cursor_column = cursor_index % TERMINAL_COLS;
    render_terminal();
}

void terminal_print_color(const char *str, unsigned char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_print_char_color(str[i], color);
    }
}
void clear_terminal() {
    terminal->cursor_column = 0;
    terminal->cursor_row = 0;
    terminal->cursor_floor = 0;
    window_t *window = terminal->window;
    memset(window->pixel_buffer, 0, window->width * window->height);
    memset(terminal->buffer, 0, TERMINAL_ROWS * TERMINAL_COLS * sizeof(uint16_t));
}

void init_terminal() {
    terminal = kmalloc(sizeof(terminal_t));
    terminal->buffer = kmalloc(TERMINAL_COLS * TERMINAL_ROWS * sizeof(uint16_t));
    terminal->window = wm_create_window(0, 0, TERMINAL_WIN_W, TERMINAL_WIN_H, "TERMINAL", 0);
    terminal->cursor_column = 0;
    terminal->cursor_row = 0;
    terminal->cursor_floor = 0;
}

void terminal_print_at(const char *str, int x, int y, unsigned char color) {
    uint32_t saved_row = terminal->cursor_row;
    uint32_t saved_col = terminal->cursor_column;

    terminal->cursor_row = y;
    terminal->cursor_column = x;

    for (int i = 0; str[i] != '\0'; i++) {
        uint32_t index = (terminal->cursor_row * TERMINAL_COLS) + terminal->cursor_column;
        terminal->buffer[index] = (color << TERMINAL_COLOR_SHIFT) | str[i];
        terminal->cursor_column++;
    }

    terminal->cursor_row = saved_row;
    terminal->cursor_column = saved_col;
}

void terminal_print_many(const char *first, ...) {
    va_list args;
    va_start(args, first);
    terminal_print_color(first, WHITE);
    const char *str;
    while ((str = va_arg(args, const char *)) != NULL) {
        terminal_print_color(str, WHITE);
    }
    va_end(args);
}
