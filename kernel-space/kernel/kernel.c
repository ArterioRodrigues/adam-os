#include "../pch.h"

void init_shell() {
    uint32_t count = _binary_shell_bin_end - _binary_shell_bin_start;
    ramfs_make_file("/", "shell");
    ramfs_write("/shell", (char *)_binary_shell_bin_start, count);
    ramfs_ls("/");
}
 
registers_t make_initial_registers(uint32_t entry_vaddr, uint32_t stack_vaddr) {
    registers_t regs;
    memset(&regs, 0, sizeof(registers_t));

    uint32_t eflags;
    asm volatile("pushf; pop %0" : "=r"(eflags));

    regs.eip = entry_vaddr;
    regs.cs = (3 * 8) | 3;
    regs.eflags = eflags | 0x200;
    regs.useresp = stack_vaddr + 0x1000 - 4;
    regs.ss = (4 * 8) | 3;
    regs.ds = regs.es = regs.fs = regs.gs = (4 * 8) | 3;

    return regs;
}

page_directory_t *create_kernel_page_directory(void *fn) {
    page_directory_t *page_directory = create_page_directory();

    for (uint32_t i = KERNEL_START; i <= KERNEL_END; i += PAGE_SIZE)
        map_page(page_directory, i, i, PAGE_FLAG_KERNEL);

    map_page(page_directory, VGA_ADDRESS, VGA_ADDRESS, PAGE_FLAG_KERNEL);

    uint32_t user_func_frame = allocate_frame();
    uint32_t user_stack_frame = allocate_frame();

    memcpy((void *)user_func_frame, fn, PAGE_SIZE);
    map_page(page_directory, USER_FUNC_VADDR, user_func_frame, PAGE_FLAG_USER);
    map_page(page_directory, USER_STACK_VADDR, user_stack_frame, PAGE_FLAG_USER);

    return page_directory;
}

void kernel_main() {
    clear_screen();

    init_gdt();
    set_kernel_stack(KERNEL_STACK_ADDRESS);
    init_idtp();
    remap_pic();

    init_keyboard();
    init_timer();

    init_exception();
    init_syscall();
    load_idtp();

    init_heap();
    init_ramfs();
    init_frames();

    init_shell();

    for (uint32_t i = KERNEL_START; i <= KERNEL_END; i += PAGE_SIZE) {
        uint32_t frame = allocate_frame();
    }

    page_directory_t *idle_page_table = create_kernel_page_directory((void *)_binary_idle_bin_start);
    registers_t idle_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    pcb_t *idle = create_process_control_block(idle_page_table, idle_reg, 0, NULL);

    page_directory_t *main_page_directory = create_kernel_page_directory((void *)_binary_main_bin_start);
    registers_t main_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    pcb_t *main = create_process_control_block(main_page_directory, main_reg, 0, NULL);

    init_scheduler(idle);
    scheduler_enqueue(main);
    start_scheduler();

    while (1)
        ;
}
