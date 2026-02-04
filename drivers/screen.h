#ifndef SCREEN_H
#define SCREEN_H

extern volatile unsigned short *vgaBuffer;
extern int vgaIndex;

void updateCursor(int x, int y);
void printChar(char c);
void print(const char *str);
void clearScreen();

#endif
