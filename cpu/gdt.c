#include "../pch.h"

gdt_entry_t gdt[6];
tss_entry_t tss_entry;
gdt_ptr_t gdt_ptr_struct;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[num].base_low = base & 0xFFFF;
    gdt[num].base_mid = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = limit & 0xFFFF;
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (flags << 4);
    gdt[num].access = access;
}

// Access byte breakdown:
//   Bit 7   (0x80): Present — must be 1 for valid segment
//   Bits 6-5(0x60): DPL — privilege level (0=kernel, 3=user)
//   Bit 4   (0x10): Descriptor type (1=code/data, 0=system)
//   Bit 3   (0x08): Executable (1=code segment, 0=data segment)
//   Bit 2   (0x04): Direction/Conforming
//   Bit 1   (0x02): Readable(code) / Writable(data)
//   Bit 0   (0x01): Accessed (CPU sets this, leave as 0)
//
//   0x9A = 1001 1010 = Present | DPL=0 | Code | Executable | Readable
//   0x92 = 1001 0010 = Present | DPL=0 | Data | Writable
//   0xFA = 1111 1010 = Present | DPL=3 | Code | Executable | Readable
//   0xF2 = 1111 0010 = Present | DPL=3 | Data | Writable
//   0x89 = 1000 1001 = Present | DPL=0 | System | 32-bit TSS Available
//
// Flags nibble (upper 4 bits of granularity byte):
//   Bit 3 (0x8): Granularity — 0=byte limit, 1=4KB page limit
//   Bit 2 (0x4): Size        — 0=16-bit, 1=32-bit protected mode
//   0xC = 1100 = 4KB granularity + 32-bit — used for all normal segments
//   0x0 = 0000 = byte granularity + 16-bit — used for TSS

void init_gdt() {
    gdt_set_gate(GDT_NULL, 0, 0, 0x00, 0x0);
    gdt_set_gate(GDT_KERNEL_CODE, 0, 0xFFFFF, 0x9A, 0xC);
    gdt_set_gate(GDT_KERNEL_DATA, 0, 0xFFFFF, 0x92, 0xC);
    gdt_set_gate(GDT_USER_CODE, 0, 0xFFFFF, 0xFA, 0xC);
    gdt_set_gate(GDT_USER_DATA, 0, 0xFFFFF, 0xF2, 0xC);
    gdt_set_gate(GDT_TSS, (uint32_t)&tss_entry, sizeof(tss_entry) - 1, 0x89, 0x0);

    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = KERNEL_DATA_SEGMENT;
    tss_entry.esp0 = KERNEL_STACK_ADDRESS;

    gdt_ptr_struct.limit = sizeof(gdt) - 1;
    gdt_ptr_struct.base = (uint32_t)&gdt;

    gdt_flush();
    tss_flush();
}

void set_kernel_stack(uint32_t stack) { tss_entry.esp0 = stack; }
