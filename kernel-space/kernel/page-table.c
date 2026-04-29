#include "../pch.h"

static void identity_map_in(page_directory_t *pd, uint32_t phys_addr) {
    uint32_t page_directory_index = phys_addr >> 22;
    uint32_t page_table_index = (phys_addr >> 12) & 0x3FF;
    page_table_t *page_table = (page_table_t *)(pd->entries[page_directory_index] & 0xFFFFF000);
    page_table->entries[page_table_index] = phys_addr | PAGE_FLAG_KERNEL;
}

page_directory_t *create_page_directory() {
    uint32_t memory = allocate_frame();
    page_directory_t *page_directory = (page_directory_t *)memory;

    for (int i = 0; i < 1024; i++) {
        page_directory->entries[i] = 0;
    }

    map_page(page_directory, memory, memory, PAGE_FLAG_KERNEL);
    return page_directory;
}

/*
 * TODO: FIRGURE OUT HOW TO REMOVE THE PAGE_ENABLED FUNC
 * Problem: map_page table needs to create a page_table to map
 * but we can access that page table because it's not mapped in memory
 * so we get a page fault, a way around this is to turn off paging
 * and access it directly but this is kinda ify should be a better way
 * if we do this and a program get's access to kernel it can see other
 * programs while paging is off
*/
static inline int paging_enabled(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000) != 0;
}

page_table_t *create_page_table() {
    uint32_t memory = allocate_frame();
    page_table_t *page_table = (page_table_t *)memory;

    if (paging_enabled()) {
        asm volatile("cli");
        disable_paging();

        for (int i = 0; i < 1024; i++)
            page_table->entries[i] = 0;

        enable_paging();
        asm volatile("sti");

        return page_table;
    }

    for (int i = 0; i < 1024; i++)
        page_table->entries[i] = 0;

    return page_table;
}

void copy_page_directory(page_directory_t *dest, page_directory_t *source) {
    for (int i = 0; i < 1024; i++) {
        if (!(source->entries[i] & 1))
            continue;

        uint32_t vaddr = i << 22;

        if (vaddr < USER_FUNC_VADDR) {
            dest->entries[i] = source->entries[i];
            continue;
        }

        page_table_t *source_pt = (page_table_t *)(source->entries[i] & 0xFFFFF000);

        page_table_t *dest_page_table = (page_table_t *)allocate_frame();
        identity_map_in(source, (uint32_t)dest_page_table);

        for (int j = 0; j < 1024; j++) {
            if (source_pt->entries[j] & 1) {
                uint32_t src_frame = source_pt->entries[j] & 0xFFFFF000;

                uint32_t dest_frame = allocate_frame();

                identity_map_in(source, (uint32_t)dest_frame);
                identity_map_in(source, (uint32_t)src_frame);

                memcpy((void *)dest_frame, (void *)src_frame, PAGE_SIZE);
                uint32_t flags = source_pt->entries[j] & 0xFFF;

                dest_page_table->entries[j] = dest_frame | flags;
            }
        }

        dest->entries[i] = (uint32_t)dest_page_table | (source->entries[i] & 0xFFF);
    }
}

void clear_page_directory(page_directory_t *page_directory) {
    for (int i = 0; i < 1024; i++) {
        if (!(page_directory->entries[i] & 1))
            continue;

        uint32_t vaddr = i << 22;
        if (vaddr < USER_FUNC_VADDR)
            continue;

        if (!(page_directory->entries[i] & 0x4))
            continue;

        page_table_t *page_table = (page_table_t *)(page_directory->entries[i] & 0xFFFFF000);
        for (int j = 0; j < 1024; j++) {
            if (page_table->entries[j] & 1) {
                if (!(page_table->entries[j] & 0x4))
                    continue;
                uint32_t frame = page_table->entries[j] & 0xFFFFF000;
                free_frame(frame);
            }
        }
        free_frame((uint32_t)page_table);
    }
}
void update_page_directory(page_directory_t *page_directory, void *fn, uint32_t size, registers_t *regs) {
    uint32_t frame = allocate_frame();
    uint32_t user_func_frame = frame;

    map_page(page_directory, user_func_frame, user_func_frame, PAGE_FLAG_USER);
    map_page(page_directory, USER_FUNC_VADDR, user_func_frame, PAGE_FLAG_USER);
    memcpy((void *)user_func_frame, fn, min(size, PAGE_SIZE));

    for (int i = 1; i < ceil(size, PAGE_SIZE); i++) {
        frame = allocate_frame();
        map_page(page_directory, frame, frame, PAGE_FLAG_USER);
        map_page(page_directory, USER_FUNC_VADDR + PAGE_SIZE * i, frame, PAGE_FLAG_USER);

        uint32_t offset = i * PAGE_SIZE;
        uint32_t remaining = size - offset;
        memcpy((void *)frame, fn + offset, min(remaining, PAGE_SIZE));
    }

    uint32_t user_stack_frame = allocate_frame();
    map_page(page_directory, user_stack_frame, user_stack_frame, PAGE_FLAG_USER);
    map_page(page_directory, USER_STACK_VADDR, user_stack_frame, PAGE_FLAG_USER);

    regs->eip = USER_FUNC_VADDR;
    regs->useresp = USER_STACK_VADDR + 0x1000 - 4;
    regs->esp = USER_STACK_VADDR + 0x1000 - 4;

    uint32_t eflags;
    asm volatile("pushf; pop %0" : "=r"(eflags));

    regs->cs = (3 * 8) | 3;
    regs->ss = (4 * 8) | 3;
    regs->eflags = eflags | 0x200;

    load_page_directory(page_directory);
}
/*
 *
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
        identity_map_in(page_directory, (uint32_t)page_table);
    }
    page_table->entries[page_table_index] = physical_address | flags | 1;
}
