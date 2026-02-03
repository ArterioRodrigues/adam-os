#include "keyboard.h"
#include "../cpu/idt.h"
#include "../kernel/config.h"
#include "screen.h"

char scancodeToAscii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
    0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

void keyboardHandlerMain() {
  unsigned char scancode = inb(KEYBOARD_DATA_PORT);

  if (scancode & SCANCODE_RELEASE_MASK) {
  } else {
    if (scancode == SCANCODE_ENTER)
      printChar('\n');
    else if (scancode == SCANCODE_BACKSPACE)
      printChar('\b');
    else if (scancode < sizeof(scancodeToAscii)) {
      char c = scancodeToAscii[scancode];
      if (c != 0)
        printChar(c);
    }
  }

  outb(PIC1_COMMAND, PIC_EOI);
}
