#include "../pch.h"

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

    for (uint32_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        uint32_t frame = allocate_frame();
    }

    page_directory = create_page_directory();

    for (uint32_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        map_page(page_directory, i, i, 0x3);
    }

    uint32_t user_func_frame = allocate_frame();
    uint32_t user_stack_frame = allocate_frame();

    memcpy((void *)user_func_frame, (void *)test_user_function, PAGE_SIZE);

    map_page(page_directory, USER_FUNC_VADDR, user_func_frame, 0x7);
    map_page(page_directory, USER_STACK_VADDR, user_stack_frame, 0x7);

    load_page_directory(page_directory);
    enable_paging();

    jump_usermode();

    while (1)
        ;
}
