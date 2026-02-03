#include "../pch.h"

char keyboardBuffer[256];
int keyboardBufferIndex = 0;
bool keyboardPressed = false;

char scancodeToAscii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
    0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

void keyboardHandlerMain() {
  unsigned char scancode = inb(KEYBOARD_DATA_PORT);
  char c = scancodeToAscii[scancode];

  if (scancode & SCANCODE_RELEASE_MASK) {
    keyboardPressed = false;
  }

  else {
    keyboardPressed = true;

    if (scancode == SCANCODE_ENTER) {
      keyboardBuffer[keyboardBufferIndex] = '\0';
      keyboardBufferIndex++;
    }

    else if (scancode == SCANCODE_BACKSPACE) {
      keyboardBuffer[keyboardBufferIndex] = '\b';
      keyboardBufferIndex++;
    }

    else if (scancode < sizeof(scancodeToAscii) && c != 0) {
      keyboardBuffer[keyboardBufferIndex] = c;
      keyboardBufferIndex++;
    }
  }

  outb(PIC1_COMMAND, PIC_EOI);
}
