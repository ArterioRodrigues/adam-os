#include "../pch.h"

volatile unsigned short *vgaBuffer = (unsigned short *)VGA_ADDRESS;
int vgaIndex = 0;

void updateCursor(int x, int y) {
  uint16_t pos = y * VGA_WIDTH + x;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void printChar(char c) {
  if (c == '\n') {
    vgaIndex = (vgaIndex / VGA_WIDTH + 1) * VGA_WIDTH;
  }

  else if (c == '\b') {
    if (vgaIndex > 0) {
      vgaIndex--;
      vgaBuffer[vgaIndex] = (COLOR << VGA_COLOR_SHIFT) | ' ';
    }
  }

  else if (c == '\t') {
    int spacesToAdd = TAB_SIZE - (vgaIndex % TAB_SIZE);

    for (int i = 0; i < spacesToAdd; i++) {
      vgaBuffer[vgaIndex] = (COLOR << VGA_COLOR_SHIFT) | ' ';
      vgaIndex++;
    }
  }

  else {
    vgaBuffer[vgaIndex] = (COLOR << VGA_COLOR_SHIFT) | c;
    vgaIndex++;
  }

  if (vgaIndex >= VGA_SIZE) {
    vgaIndex = VGA_WIDTH * (VGA_HEIGHT - 1);
  }
}

void print(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    printChar(str[i]);
  }
}

void clearScreen() {
  for (int i = 0; i < VGA_SIZE; i++) {
    vgaBuffer[i] = (COLOR << VGA_COLOR_SHIFT) | ' ';
  }
  vgaIndex = 0;
}
