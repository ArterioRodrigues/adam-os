#ifndef GTD_H
#define GDT_H
#include "../kernel/types.h"

#define GDT_NULL        0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE   3
#define GDT_USER_DATA   4
#define GDT_TSS         5

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss_entry_struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));
typedef struct tss_entry_struct tss_entry_t;

extern struct gdt_entry gdt[6];
extern tss_entry_t tss_entry;
extern struct gdt_ptr gdt_ptr_struct;
extern void gdt_flush();
extern void tss_flush();

void init_gdt();
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void set_kernel_stack(uint32_t stack);
#endif
