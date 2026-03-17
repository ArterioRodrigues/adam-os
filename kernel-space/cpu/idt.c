#include "../pch.h"

idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idtp;

uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void outw(uint16_t port, uint16_t data) { __asm__("out %%ax, %%dx" : : "a"(data), "d"(port)); }

uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void outb(uint16_t port, uint8_t data) { __asm__("out %%al, %%dx" : : "a"(data), "d"(port)); }

void idt_set_gate(uint8_t num, unsigned int base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & BITMASK_LOW;
    idt[num].base_high = (base >> 16) & BITMASK_HIGH;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void remap_pic() {
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, PIC_ICW1_INIT);
    outb(PIC2_COMMAND, PIC_ICW1_INIT);

    outb(PIC1_DATA, PIC1_OFFSET);
    outb(PIC2_DATA, PIC2_OFFSET);

    outb(PIC1_DATA, PIC1_CASCADE);
    outb(PIC2_DATA, PIC2_CASCADE);

    outb(PIC1_DATA, PIC_ICW4_8086);
    outb(PIC2_DATA, PIC_ICW4_8086);

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void init_idtp() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
}

void load_idtp() {
    idt_load((unsigned int)&idtp);

    asm volatile("sti");

    outb(PIC1_DATA, MASTER_MOUSE_TIMER_KB_MASK);
    outb(PIC2_DATA, SLAVE_MOUSE_MASK);
}
