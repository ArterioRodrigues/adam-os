#include "../pch.h"

const char *exception_messages[] = {
    "Division By Zero",            // 0
    "Debug",                       // 1
    "Non Maskable Interrupt",      // 2
    "Breakpoint",                  // 3
    "Overflow",                    // 4
    "Out of Bounds",               // 5
    "Invalid Opcode",              // 6
    "No Coprocessor",              // 7
    "Double Fault",                // 8
    "Coprocessor Segment Overrun", // 9
    "Bad TSS",                     // 10
    "Segment Not Present",         // 11
    "Stack Fault",                 // 12
    "General Protection Fault",    // 13
    "Page Fault",                  // 14
    "Unknown Interrupt",           // 15
    "Coprocessor Fault",           // 16
    "Alignment Check",             // 17
    "Machine Check",               // 18
    "Reserved",                    // 19-31
};

void init_exception() {
    idt_set_gate(0, (unsigned int)divide_error_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(1, (unsigned int)debug_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(3, (unsigned int)breakpoint_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(4, (unsigned int)overflow_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(5, (unsigned int)bound_range_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(6, (unsigned int)invalid_opcode_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(7, (unsigned int)device_not_available_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(8, (unsigned int)double_fault_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(10, (unsigned int)invalid_tss_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(11, (unsigned int)segment_not_present_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(12, (unsigned int)stack_fault_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(13, (unsigned int)gpf_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(14, (unsigned int)page_fault_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(16, (unsigned int)fpu_error_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(17, (unsigned int)alignment_check_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(18, (unsigned int)machine_check_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(19, (unsigned int)simd_fp_handler, KERNEL_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
}

void exception_handler_c(registers_t *regs) {
    print("\n*** EXCEPTION ***\n");

    if (regs->int_no < 20) {
        print(exception_messages[regs->int_no]);
    } else {
        print("Reserved Exception");
    }
    print("\n");

    char buf[20];
    print("Exception: ");
    itos(buf, regs->int_no);
    print(buf);
    print("\n");

    print("Error Code: ");
    itohs(buf, regs->err_code);
    print(buf);
    print("\n");

    if (regs->int_no == 14) {
        uint32_t faulting_address;
        asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

        print("Faulting address: ");
        itohs(buf, faulting_address);
        print(buf);
        print("\n");

        print("Caused by: ");
        if (!(regs->err_code & 0x1))
            print("Page not present ");
        if (regs->err_code & 0x2)
            print("Write operation ");
        else
            print("Read operation ");
        if (regs->err_code & 0x4)
            print("User mode");
        else
            print("Kernel mode");
        print("\n");
    }

    if (regs->int_no == 13) {
        print("Segment selector: ");
        itohs(buf, regs->err_code >> 3);
        print(buf);
        print("\n");
    }

    print("\nRegisters:\n");
    print("EAX=");
    itohs(buf, regs->eax);
    print(buf);
    print(" ");
    print("EBX=");
    itohs(buf, regs->ebx);
    print(buf);
    print("\n");
    print("ECX=");
    itohs(buf, regs->ecx);
    print(buf);
    print(" ");
    print("EDX=");
    itohs(buf, regs->edx);
    print(buf);
    print("\n");
    print("ESI=");
    itohs(buf, regs->esi);
    print(buf);
    print(" ");
    print("EDI=");
    itohs(buf, regs->edi);
    print(buf);
    print("\n");
    print("EBP=");
    itohs(buf, regs->ebp);
    print(buf);
    print(" ");
    print("ESP=");
    itohs(buf, regs->esp);
    print(buf);
    print("\n");
    print("EIP=");
    itohs(buf, regs->eip);
    print(buf);
    print(" ");
    print("EFLAGS=");
    itohs(buf, regs->eflags);
    print(buf);
    print("\n");

    print("\nSystem halted.\n");
    asm volatile("cli; hlt");

    while (1)
        ;
}
