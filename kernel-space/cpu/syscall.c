#include "../pch.h"
extern void syscall_handler();
char syscall_buffer[256];

void handle_syscall_exit() {
    print("\nUser program exited\n");
    while (1)
        ;
}

void handle_syscall_read(registers_t *regs) {
    int fd = regs->ebx;
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;

    if (fd == 0) {
        asm volatile("sti"); 
        while (keyboard_buffer_index < (int)len)  

        asm volatile("cli");

        for (uint32_t i = 0; i < len; i++) {
            buf[i] = keyboard_buffer[i];
        }
        keyboard_buffer_index = 0; 
        regs->eax = len;
    }
}

void handle_syscall_write(registers_t *regs) {
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;
    for (uint32_t i = 0; i < len; i++) {
        print_char(buf[i]);
    }
    regs->eax = len;
}

void syscall_handler_main(registers_t *regs) {
    uint32_t syscall_num = regs->eax;

    switch (syscall_num) {
    case SYSCALL_EXIT:
        handle_syscall_exit();
        break;
    case SYSCALL_WRITE:
        handle_syscall_write(regs);
        break;
    case SYSCALL_READ:
        handle_syscall_read(regs);
        break;
    default:
        print("Unknown syscall\n");
        regs->eax = -1;
    }
}
void init_syscall() { idt_set_gate(0x80, (unsigned int)syscall_handler, KERNEL_CODE_SEGMENT, RING3_CALL); }
