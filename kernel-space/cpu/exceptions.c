#include "exceptions.h"
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

#define PADDING 20
void exception_handler_c(registers_t *regs) {
    vga_draw_rect(0, 0, 800, 800, BLUE);
    vga_draw_string(PADDING, PADDING, "*** EXCEPTION ***", WHITE);

    if (regs->int_no < 20)
        vga_draw_string(PADDING, PADDING + 10,(char *)exception_messages[regs->int_no], WHITE);
    else
        vga_draw_string(PADDING, PADDING + 10, "Reserved Exception", WHITE);

    char buf[20];
    vga_draw_string(PADDING, PADDING + 20, "Exception: ", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 20, itos(buf, regs->int_no), WHITE);

    vga_draw_string(PADDING, PADDING + 30, "Error Code: ", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 30, itos(buf, regs->err_code), WHITE);

    if (regs->int_no == 14) {
        uint32_t faulting_address;
        asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

        vga_draw_string(PADDING, PADDING + 40, "Faulting address: ", WHITE);
        vga_draw_string(PADDING + 200, PADDING + 40, itohs(buf, faulting_address), WHITE);

        vga_draw_string(PADDING, PADDING + 50, "Caused by: ", WHITE);
        if (!(regs->err_code & 0x1))
            vga_draw_string(PADDING, PADDING + 60, "Page not present", WHITE);
        if (regs->err_code & 0x2)
            vga_draw_string(PADDING + 200, PADDING + 60, "Write operation", WHITE);
        else
            vga_draw_string(PADDING + 200, PADDING + 60, "Read operation", WHITE);
        if (regs->err_code & 0x4)
            vga_draw_string(PADDING + 400, PADDING + 60, "User mode", WHITE);
        else
            vga_draw_string(PADDING + 400 , PADDING + 60, "Kernel mode", WHITE);
    }

    if (regs->int_no == 13) {
        vga_draw_string(PADDING, PADDING + 70, "Segment selector: ", WHITE);
        vga_draw_string(PADDING + 200, PADDING + 70, itohs(buf, regs->err_code >> 3), WHITE);
    }

    vga_draw_string(PADDING, PADDING + 80, "Registers: ", WHITE);

    vga_draw_string(PADDING, PADDING + 90, "EAX=", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 90, itohs(buf, regs->eax), WHITE);
    vga_draw_string(PADDING + 220, PADDING + 90, "EBX=", WHITE);
    vga_draw_string(PADDING + 320, PADDING + 90, itohs(buf, regs->ebx), WHITE);

    vga_draw_string(PADDING,  PADDING + 100, "ECX=", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 100, itohs(buf, regs->ecx), WHITE);
    vga_draw_string(PADDING + 220, PADDING + 100, "EDX=", WHITE);
    vga_draw_string(PADDING + 320, PADDING + 100, itohs(buf, regs->edx), WHITE);

    vga_draw_string(PADDING,  PADDING + 110, "ESI=", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 110, itohs(buf, regs->esi), WHITE);
    vga_draw_string(PADDING + 220, PADDING + 110, "EDI=", WHITE);
    vga_draw_string(PADDING + 320, PADDING + 110, itohs(buf, regs->edi), WHITE);

    vga_draw_string(PADDING,  PADDING + 120, "EBP=", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 120, itohs(buf, regs->ebp), WHITE);
    vga_draw_string(PADDING + 220, PADDING + 120, "ESP=", WHITE);
    vga_draw_string(PADDING + 320, PADDING + 120, itohs(buf, regs->esp), WHITE);

    vga_draw_string(PADDING,  PADDING + 130, "EIP=", WHITE);
    vga_draw_string(PADDING + 100, PADDING + 130, itohs(buf, regs->eip), WHITE);
    vga_draw_string(PADDING + 220, PADDING + 130, "EFLAGS=", WHITE);
    vga_draw_string(PADDING + 320, PADDING + 130, itohs(buf, regs->eflags), WHITE);

    vga_draw_string(PADDING, PADDING + 140, "System halted.", WHITE);
    vga_flip();
    asm volatile("cli; hlt");

    while (1)
        ;
}

void dump_registers(registers_t *regs) {
    char buf[20];
    print("\n=== REGISTERS ===\n");
    print("EAX=");
    print(itohs(buf, regs->eax));
    print(" EBX=");
    print(itohs(buf, regs->ebx));
    print("\n");
    print("ECX=");
    print(itohs(buf, regs->ecx));
    print(" EDX=");
    print(itohs(buf, regs->edx));
    print("\n");
    print("ESI=");
    print(itohs(buf, regs->esi));
    print(" EDI=");
    print(itohs(buf, regs->edi));
    print("\n");
    print("EBP=");
    print(itohs(buf, regs->ebp));
    print(" ESP=");
    print(itohs(buf, regs->esp));
    print("\n");
    print("EIP=");
    print(itohs(buf, regs->eip));
    print(" EFLAGS=");
    print(itohs(buf, regs->eflags));
    print("\n");
    print("CS=");
    print(itohs(buf, regs->cs));
    print(" SS=");
    print(itohs(buf, regs->ss));
    print("\n");
    print("DS=");
    print(itohs(buf, regs->ds));
    print(" INT=");
    print(itohs(buf, regs->int_no));
    print("\n");
    print(" USERESP=");
    print(itohs(buf, regs->useresp));
    print("\n");
    print("=================\n");
}
