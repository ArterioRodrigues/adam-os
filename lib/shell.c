#include "../pch.h"

void shellHandlerMain() {
  const char c = keyboardBuffer[keyboardBufferIndex - 1];
  int x = vgaIndex % VGA_WIDTH;
  int y = vgaIndex / VGA_WIDTH;

  updateCursor(x, y);

  if (c != '\0' || keyboardBufferIndex == 0) {

    if (keyboardBufferIndex == 0) {
      print("> ");
    }
    if (keyboardPressed == true) {
      printChar(c);
    }
    return;
  }

  if (strcmp(keyboardBuffer, "clear")) {
    clearScreen();
  }

  else if (strcmp(keyboardBuffer, "about")) {
    printChar('\n');
    print("\tname - Adam OS");
    print("\tversion - 0.0.1");
    printChar('\n');
  }

  else if (strcmp(keyboardBuffer, "help")) {
    printChar('\n');
    print("\tabout - info about OS version\n");
    print("\tclear - clear terminal screen\n");
    print("\tehco - print message to screen after echo\n");
    printChar('\n');
  }

  else if (strncmp(keyboardBuffer, "echo", 4)) {
    printChar('\n');
    printChar('\t');
    print(keyboardBuffer + 4);
    printChar('\n');
  }

  else {
    printChar('\n');
    print("\t command not found!");
    printChar('\n');
    print("\t type help for command list");
    printChar('\n');
  }
  keyboardBufferIndex = 0;
}
