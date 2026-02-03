#ifndef SCREEN_H
#define SCREEN_H

extern volatile unsigned short *vgaBuffer;
extern int vgaIndex;

void printChar(char c);
void print(const char *str);
void clearScreen();

#endif
