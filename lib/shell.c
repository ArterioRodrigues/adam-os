#include "../pch.h"

void shellHandlerMain() {
  const char c = keyboardBuffer[keyboardBufferIndex - 1];

  if (c != '\0' || keyboardBufferIndex == 0) {

    if (keyboardBufferIndex == 0) {
      print("> ");
    }
    if (keyboardPressed == true) {
      printChar(c);
    }
    return;
  }

  struct string clearCommand = createString("clear");
  struct string helpCommand = createString("help");
  struct string echoCommand = createString("echo");
  struct string aboutCommand = createString("about");

  struct string keyboardBufferString = createString(keyboardBuffer);

  if (cmpString(clearCommand, keyboardBufferString)) {
    clearScreen();
  } else if (cmpString(echoCommand, keyboardBufferString)) {
    printChar('\n');
    print(keyboard
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
