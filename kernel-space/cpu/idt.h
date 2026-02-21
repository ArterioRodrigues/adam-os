#ifndef IDT_H
#define IDT_H
#include "../kernel/config.h"

extern void idt_load(unsigned int);

struct idt_entry {
    unsigned short base_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

typedef struct idt_ptr idt_ptr_t;
typedef struct idt_entry idt_entry_t;

extern idt_entry_t idt[IDT_ENTRIES];
extern idt_ptr_t idtp;

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector, unsigned char flags);
void remap_pic();
void init_idtp();
void load_idtp();

#endif
