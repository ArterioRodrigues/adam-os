#include "../pch.h"

volatile unsigned short *vga_buffer = (unsigned short *)VGA_ADDRESS;
int vga_index = 0;

void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & BYTE_MASK));
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & BYTE_MASK));
}

void print_char_color(char c, unsigned char color) {
    if (c == '\n') {
        vga_index = (vga_index / VGA_WIDTH + 1) * VGA_WIDTH;
    }

    else if (c == '\b') {
        if (vga_index > 0) {
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

    if (vga_index >= VGA_SIZE) {
        vga_index = VGA_WIDTH * (VGA_HEIGHT - 1);
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
