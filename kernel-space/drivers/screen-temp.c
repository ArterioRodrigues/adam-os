#include "../pch.h"

uint8_t ansi_to_vga[20]= {
    BLACK,                // #30
    RED,                  // #31
    GREEN,                // #32
    BROWN,                // #33
    BLUE,                 // #34
    MAGENTA,              // #35
    CYAN,                 // #36
    LIGHT_GRAY,           // #37
    DARK_GRAY,            // #38
    BRIGHT_RED,           // #39
    BRIGHT_GREEN,         // #40
    YELLOW,               // #41
    BRIGHT_BLUE,          // #42
    BRIGHT_MAGENTA,       // #43
    BRIGHT_CYAN,          // #44
    WHITE                 // #45
};

volatile unsigned short *vga_buffer = (unsigned short *)VGA_ADDRESS;
int vga_index = 0;
int vga_cursor_floor = 0;

void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & BYTE_MASK));
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & BYTE_MASK));
}

void print_at(const char *str, int x, int y, unsigned char color) {
    int saved = vga_index;

    vga_index = y * VGA_WIDTH + x;

    for (int i = 0; str[i] != '\0'; i++) {
        vga_buffer[vga_index] = (color << VGA_COLOR_SHIFT) | str[i];
        vga_index++;
    }

    vga_index = saved;
}

void print_char_color(char c, unsigned char color) {
    if (vga_index >= VGA_SIZE) {
        for (int i = 0; i < VGA_SIZE; i++) {
            if (i > VGA_SIZE - VGA_WIDTH)
                vga_buffer[i] = (WHITE << VGA_COLOR_SHIFT) | ' ';
            else
                vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        vga_index = VGA_SIZE - VGA_WIDTH;
    }

    if (c == '\n')
        vga_index = (vga_index / VGA_WIDTH + 1) * VGA_WIDTH;

    else if (c == '\b') {
        if (vga_index > 0 && vga_index > vga_cursor_floor) {
            vga_index--;
            vga_buffer[vga_index] = (color << VGA_COLOR_SHIFT) | ' ';
        }
    }

    else if (c == '\t') {
        int spacesToAdd = TAB_SIZE - (vga_index % TAB_SIZE);

        for (int i = 0; i < spacesToAdd; i++) {
            vga_buffer[vga_index] = (color << VGA_COLOR_SHIFT) | ' ';
            vga_index++;
        }
    }

    else {
        vga_buffer[vga_index] = (color << VGA_COLOR_SHIFT) | c;
        vga_index++;
    }

}

void print_color(const char *str, unsigned char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char_color(str[i], color);
    }
}

void clear_screen() {
    for (int i = 0; i < VGA_SIZE; i++) {
        vga_buffer[i] = (WHITE << VGA_COLOR_SHIFT) | ' ';
    }
    vga_index = 0;
}

void print_many(const char *first, ...) {
    va_list args;
    va_start(args, first);

    print(first);

    const char *str;
    while ((str = va_arg(args, const char *)) != NULL) {
        print(str);
    }

    va_end(args);
}
