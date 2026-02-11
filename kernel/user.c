#include "../pch.h"

extern void jump_usermode();

void test_user_program() {

    char msg[] = "Hello from user space!\n";

    asm volatile("mov $4, %%eax;"
                 "mov %0, %%ecx;"
                 "mov %1, %%edx;"
                 "int $0x80"
                 :
                 : "r"(msg), "r"(sizeof(msg))
                 : "eax", "ecx", "edx");

    asm volatile("mov $1, %%eax;"
                 "int $0x80"
                 :
                 :
                 : "eax");

    while (1)
        ;
}

void enter_userspace() {
    uint32_t user_stack = 0x200000;

    uint32_t stack_frame = allocate_frame();
    map_page(page_directory, user_stack - PAGE_SIZE, stack_frame, 0x7);

    uint32_t code_frame = allocate_frame();
    map_page(page_directory, 0x400000, code_frame, 0x7);

    memcpy((void *)0x400000, (void *)test_user_program, 0x1000);


    asm volatile("cli;"
                 "mov $0x23, %%ax;"
                 "mov %%ax, %%ds;"
                 "mov %%ax, %%es;"
                 "mov %%ax, %%fs;"
                 "mov %%ax, %%gs;"

                 "mov %%esp, %%eax;"
                 "push $0x23;"
                 "push %0;"
                 "pushf;"
                 "pop %%eax;"
                 "or $0x200, %%eax;"
                 "push %%eax;"
                 "push $0x1B;"
                 "push %1;"
                 "iret"
                 :
                 : "r"(user_stack), "r"(0x400000)
                 : "eax");
}
