#ifndef IDT_H
#define IDT_H
#include "../kernel/config.h"

struct idtEntry {
  unsigned short baseLow;
  unsigned short selector;
  unsigned char zero;
  unsigned char flags;
  unsigned short baseHigh;
} __attribute__((packed));

struct idtPtr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

extern struct idtEntry idt[IDT_ENTRIES];
extern struct idtPtr idtp;

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void idtSetGate(unsigned char num, unsigned int base, unsigned short selector,
                unsigned char flags);
void remapPic();

#endif
