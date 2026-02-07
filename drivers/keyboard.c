#include "../pch.h"

char keyboard_buffer[256];
int keyboard_buffer_index = 0;
bool keyboard_pressed = false;

char scancode_to_ascii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,
    0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', 0,    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   0,    0,   ' '};

void keyboard_handler_main() {
  unsigned char scancode = inb(KEYBOARD_DATA_PORT);
  char c = scancode_to_ascii[scancode];

  if (scancode & SCANCODE_RELEASE_MASK) {
    keyboard_pressed = false;
  }

  else {
    keyboard_pressed = true;

    if (scancode == SCANCODE_ENTER) {
      keyboard_buffer[keyboard_buffer_index] = '\0';
      keyboard_buffer_index++;
    }

    else if (scancode == SCANCODE_BACKSPACE) {
      keyboard_buffer[keyboard_buffer_index] = '\b';
      keyboard_buffer_index++;
    }

    else if (scancode < sizeof(scancode_to_ascii) && c != 0) {
      keyboard_buffer[keyboard_buffer_index] = c;
      keyboard_buffer_index++;
    }
  }

  outb(PIC1_COMMAND, PIC_EOI);
}

void init_keyboard() {
  idt_set_gate(IRQ_KEYBOARD, (unsigned int)keyboard_handler,
               KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
}
