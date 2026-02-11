#include "../pch.h"

void kernel_main() {
    clear_screen();
    print("=== ADAM OS ===\n");

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

    page_directory = create_page_directory();

    for (uint32_t i = 0; i < 0x100000; i += PAGE_SIZE) {
        map_page(page_directory, i, i, 0x3);
    }

    for (uint32_t i = HEAP_START; i < HEAP_END; i += PAGE_SIZE) {
        uint32_t frame = allocate_frame();
        map_page(page_directory, i, frame, 0x3);
    }

    load_page_directory(page_directory);
    enable_paging();

    print("Paging enabled!\n");
    print("Entering user space...\n");

    enter_userspace(); 

    while (1)
        ;
}
