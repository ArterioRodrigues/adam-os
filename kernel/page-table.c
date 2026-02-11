#include "../pch.h"

page_directory_t *page_directory;

page_directory_t *create_page_directory() {
    uint32_t memory = allocate_frame();
    page_directory_t *page_directory = (page_directory_t *)memory;

    for (int i = 0; i < 1024; i++) {
        page_directory->entries[i] = 0;
    }

    return page_directory;
}

page_table_t *create_page_table() {
    uint32_t memory = allocate_frame();
    page_table_t *page_table = (page_table_t *)memory;

    for (int i = 0; i < 1024; i++) {
        page_table->entries[i] = 0;
    }

    return page_table;
}

/*
 * map_page - Maps a virtual address to a physical address
 *
 * Virtual Address Breakdown (32-bit):
 * Bits 31-22 (10 bits): Page Directory Index (0-1023)
 * Bits 21-12 (10 bits): Page Table Index (0-1023)
 * Bits 11-0  (12 bits): Offset within page (0-4095)
 *
 * Example Translation:
 * Virtual 0x00401234 -> Directory[1], Table[1], Offset 0x234
 * If Table[1] points to physical 0x00500000, then:
 * Virtual 0x00401234 translates to Physical 0x00500234
 *
 * Page Entry Flags (bits 0-11 of directory/table entries):
 * Bit 0 (0x1): Present - Page is in memory (must be set or page fault occurs)
 * Bit 1 (0x2): Read/Write - If set, page is writable. If clear, page is read-only
 * Bit 2 (0x4): User/Supervisor - If set, user mode can access. If clear, only kernel
 * Bit 3 (0x8): Write-Through - If set, write-through caching. If clear, write-back
 * Bit 4 (0x10): Cache Disable - If set, page will not be cached
 * Bit 5 (0x20): Accessed - Set by CPU when page is accessed (read or write)
 * Bit 6 (0x40): Dirty - Set by CPU when page is written to (page tables only)
 * Bit 7 (0x80): Page Size - If set, 4MB page. If clear, 4KB page (directory only)
 * Bit 8 (0x100): Global - If set, prevents TLB flush on CR3 reload (requires CR4.PGE)
 * Bits 9-11: Available for OS use
 *
 * Common Flag Combinations:
 * 0x0: Not present, will cause page fault
 * 0x1: Present, read-only, kernel only
 * 0x3: Present, read-write, kernel only (0x1 | 0x2)
 * 0x5: Present, read-only, user accessible (0x1 | 0x4)
 * 0x7: Present, read-write, user accessible (0x1 | 0x2 | 0x4)
 *
 * Usage Example:
 * uint32_t frame = allocate_frame();
 * map_page(page_directory, 0x40000000, frame, 0x7);
 * Now virtual 0x40000000 maps to physical frame with user read-write access
 */
void map_page(page_directory_t *page_directory, uint32_t virtual_address, uint32_t physical_address, uint32_t flags) {

    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_table_index = (virtual_address >> 12) & 0x3FF;

    page_table_t *page_table;

    if (page_directory->entries[page_directory_index] & 1) {
        page_table = (page_table_t *)(page_directory->entries[page_directory_index] & 0xFFFFF000);
    } else {
        page_table = create_page_table();
        page_directory->entries[page_directory_index] = ((uint32_t)page_table) | flags | 1;
    }
    page_table->entries[page_table_index] = physical_address | flags | 1;
}

void load_page_directory(page_directory_t *page_directory) { asm volatile("mov %0, %%cr3" ::"r"(page_directory)); }

void enable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" ::"r"(cr0));
}
