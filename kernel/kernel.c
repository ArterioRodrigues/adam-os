#include "../pch.h"
#include "config.h"
#include "kmalloc.h"
#include "scheduler.h"

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

    for (uint32_t i = HEAP_START; i <= HEAP_END; i += PAGE_SIZE)
        map_page(page_directory, i, i, PAGE_FLAG_KERNEL);

    map_page(page_directory, VGA_ADDRESS, VGA_ADDRESS, PAGE_FLAG_KERNEL);

    uint32_t user_func_frame = allocate_frame();
    uint32_t user_stack_frame = allocate_frame();

    memcpy((void *)user_func_frame, fn, PAGE_SIZE);
    map_page(page_directory, USER_FUNC_VADDR, user_func_frame, PAGE_FLAG_USER);
    map_page(page_directory, USER_STACK_VADDR, user_stack_frame, PAGE_FLAG_USER);

    map_page(page_directory, user_func_frame, user_func_frame, PAGE_FLAG_USER);
    map_page(page_directory, user_stack_frame, user_stack_frame, PAGE_FLAG_USER);
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
    init_shell();

    init_exception();
    init_syscall();
    load_idtp();

    init_heap();
    init_ramfs();
    init_frames();

    for (uint32_t i = KERNEL_START; i <= KERNEL_END; i += PAGE_SIZE) {
        uint32_t frame = allocate_frame();
    }

    page_directory_t *process1_page_directory = create_kernel_page_directory((void *)process1);
    page_directory_t *process2_page_directory = create_kernel_page_directory((void *)process2);
    page_directory_t *process3_page_directory = create_kernel_page_directory((void *)process3);
    page_directory_t *process4_page_directory = create_kernel_page_directory((void *)process4);
    page_directory_t *process5_page_directory = create_kernel_page_directory((void *)process5);
    page_directory_t *process6_page_directory = create_kernel_page_directory((void *)process6);
    page_directory_t *process7_page_directory = create_kernel_page_directory((void *)process7);
    page_directory_t *process8_page_directory = create_kernel_page_directory((void *)process8);
    page_directory_t *process9_page_directory = create_kernel_page_directory((void *)process9);
    page_directory_t *process10_page_directory = create_kernel_page_directory((void *)process10);

    registers_t process1_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process2_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process3_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process4_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process5_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process6_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process7_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process8_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process9_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);
    registers_t process10_reg = make_initial_registers(USER_FUNC_VADDR, USER_STACK_VADDR);

    pcb_t *process1 = create_process_control_block(process1_page_directory, process1_reg, 0, NULL);
    pcb_t *process2 = create_process_control_block(process2_page_directory, process2_reg, 0, NULL);
    pcb_t *process3 = create_process_control_block(process3_page_directory, process3_reg, 0, NULL);
    pcb_t *process4 = create_process_control_block(process4_page_directory, process4_reg, 0, NULL);
    pcb_t *process5 = create_process_control_block(process5_page_directory, process4_reg, 0, NULL);
    pcb_t *process6 = create_process_control_block(process6_page_directory, process4_reg, 0, NULL);
    pcb_t *process7 = create_process_control_block(process7_page_directory, process1_reg, 0, NULL);
    pcb_t *process8 = create_process_control_block(process8_page_directory, process2_reg, 0, NULL);
    pcb_t *process9 = create_process_control_block(process9_page_directory, process3_reg, 0, NULL);
    pcb_t *process10 = create_process_control_block(process10_page_directory, process4_reg, 0, NULL);

    init_scheduler(process1);
    scheduler_enqueue(process2);
    scheduler_enqueue(process3);
    scheduler_enqueue(process4);
    scheduler_enqueue(process5);
    scheduler_enqueue(process6);
    scheduler_enqueue(process7);
    scheduler_enqueue(process8);
    scheduler_enqueue(process9);
    scheduler_enqueue(process10);

    start_scheduler();

    while (1)
        ;
}
