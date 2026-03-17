#include "../pch.h"

void mouse_handler_main(registers_t *regs) {
    char *buf = "MOUSE";
    vga_draw_string(0, 0, buf, 0xf);
    vga_draw_char(10, 10, 'A', 0xf);
    vga_flip();
}

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
