#include "../pch.h"

uint8_t mouse_cycle = 0;
uint8_t mouse_bytes[3];
uint8_t mouse_buttons = 0;
uint8_t prev_mouse_buttons = 0;
int mouse_x = 400;
int mouse_y = 300;
int prev_mouse_x = 400;
int prev_mouse_y = 300;
int drag_offset_x = 0;
int drag_offset_y = 0;
int dx = 0;
int dy = 0;
bool is_x_negative = false;
bool is_y_negative = false;

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;

    if (type == 0) {
        while (timeout--) {
            if (inb(PS2_STATUS) & 1)
                return;
        }
    } else {
        while (timeout--) {
            if (!(inb(PS2_STATUS) & 2))
                return;
        }
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(PS2_COMMAND, 0xD4);
    mouse_wait(1);
    outb(PS2_DATA, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(PS2_DATA);
}

void init_mouse() {
    mouse_wait(1);
    outb(PS2_COMMAND, 0xA8);

    mouse_wait(1);
    outb(PS2_COMMAND, 0x20);
    mouse_wait(0);
    uint8_t status = inb(PS2_DATA);

    status |= 2;
    mouse_wait(1);
    outb(PS2_COMMAND, 0x60);
    mouse_wait(1);
    outb(PS2_DATA, status);

    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();

    idt_set_gate(0x2C, (unsigned int)mouse_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);

    uint8_t mask = inb(PIC2_DATA);
    mask &= ~(1 << 4);
    outb(PIC2_DATA, mask);
}

void mouse_handler_main(registers_t *regs) {
    uint8_t input = mouse_read();

    if (mouse_cycle == 0) {
        mouse_bytes[0] = input;
        if (!(input & (1 << 3))) {
            outb(PIC2_COMMAND, PIC_EOI);
            outb(PIC1_COMMAND, PIC_EOI);
            return;
        }
    }

    else if (mouse_cycle == 1)
        mouse_bytes[1] = input;
    else {
        mouse_bytes[2] = input;
        if ((mouse_bytes[0] & (1 << 6)) || (mouse_bytes[0] & (1 << 7))) {
            outb(PIC2_COMMAND, PIC_EOI);
            outb(PIC1_COMMAND, PIC_EOI);
            mouse_cycle = 0;
            return;
        }

        is_x_negative = mouse_bytes[0] & (1 << 4) ? true : false;
        is_y_negative = mouse_bytes[0] & (1 << 5) ? true : false;

        dx = is_x_negative ? mouse_bytes[1] | 0xFFFFFF00 : mouse_bytes[1];
        dy = is_y_negative ? mouse_bytes[2] | 0xFFFFFF00 : mouse_bytes[2];

        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;
        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0)
            mouse_x = 0;
        if (mouse_x > screen_width - 1)
            mouse_x = screen_width - 1;
        if (mouse_y < 0)
            mouse_y = 0;
        if (mouse_y > screen_height - 1)
            mouse_y = screen_height - 1;

        prev_mouse_buttons = mouse_buttons;
        mouse_buttons = mouse_bytes[0] & 0x07;

        update_window();
        wm_composite();
    }

    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);

    mouse_cycle = (mouse_cycle + 1) % 3;
}
