#include "../cpu/idt.h"
#include "../drivers/screen.h"
#include "config.h"

extern void keyboardHandler();
extern void idtLoad();

void kernelMain() {
  clearScreen();
  print("Welcome to my OS!\n");
  print("Kernel loaded successfully!\n");
  print("This is written in C!\n");

  idtp.limit = (sizeof(struct idtEntry) * IDT_ENTRIES) - 1;
  idtp.base = (unsigned int)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idtSetGate(i, 0, 0, 0);
  }

  remapPic();
  idtSetGate(IRQ_KEYBOARD, (unsigned int)keyboardHandler, KERNEL_CODE_SEGMENT,
             IDT_FLAG_INTERRUPT_GATE);
  idtLoad((unsigned int)&idtp);

  __asm__("sti");

  outb(PIC1_DATA, KEYBOARD_IRQ_MASK);

  while (1)
    ;
}
