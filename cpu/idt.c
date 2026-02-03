#include "../pch.h"

struct idtEntry idt[IDT_ENTRIES];
struct idtPtr idtp;

unsigned char inb(unsigned short port) {
  unsigned char result;
  __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

void outb(unsigned short port, unsigned char data) {
  __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

void idtSetGate(unsigned char num, unsigned int base, unsigned short selector,
                unsigned char flags) {
  idt[num].baseLow = base & BITMASK_LOW;
  idt[num].baseHigh = (base >> 16) & BITMASK_HIGH;
  idt[num].selector = selector;
  idt[num].zero = 0;
  idt[num].flags = flags;
}

void remapPic() {
  unsigned char mask1 = inb(PIC1_DATA);
  unsigned char mask2 = inb(PIC2_DATA);

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
