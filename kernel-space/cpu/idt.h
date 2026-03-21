#ifndef IDT_H
#define IDT_H
#include "../kernel/config.h"
#include "../../shared/types.h"

extern void idt_load(unsigned int);

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    unsigned int base;
} __attribute__((packed));

typedef struct idt_ptr idt_ptr_t;
typedef struct idt_entry idt_entry_t;

extern idt_entry_t idt[IDT_ENTRIES];
extern idt_ptr_t idtp;

uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);
void idt_set_gate(uint8_t num, unsigned int base, uint16_t selector, uint8_t flags);
void remap_pic();
void init_idtp();
void load_idtp();

#endif
