#ifndef KEYBOARD_H
#define KEYBOARD_H

extern char keyboardBuffer[256];
extern int keyboardBufferIndex;
extern int keyboardPressed;

void keyboardHandlerMain();

#endif
