#include "../pch.h"

extern void syscall_handler();

void syscall_handler_main(registers_t *regs) {
    uint32_t syscall_num = regs->eax;

    switch (syscall_num) {
    case SYSCALL_WRITE: {
        char *buf = (char *)regs->ecx;
        uint32_t len = regs->edx;
        for (uint32_t i = 0; i < len; i++) {
            print_char(buf[i]);
        }
        regs->eax = len;
    } break;

    case SYSCALL_EXIT:
        print("\nUser program exited\n");
        while (1)
            ;
        break;

    default:
        print("Unknown syscall\n");
        regs->eax = -1;
    }
}

void init_syscall() { idt_set_gate(0x80, (unsigned int)syscall_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE); }
