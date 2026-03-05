#include "../pch.h"
#include "exceptions.h"

extern void syscall_handler();

void handle_syscall_exit() {
    print("\nUser program exited\n");
    while (1)
        ;
}

void handle_syscall_fork(registers_t *regs) {
    uint32_t page_directory_address = allocate_frame();
    {
        uint32_t page_directory_index = page_directory_address >> 22;
        uint32_t page_table_index = (page_directory_address >> 12) & 0x3FF;
        page_table_t *pt =
            (page_table_t *)(current_process->page_directory->entries[page_directory_index] & 0xFFFFF000);
        pt->entries[page_table_index] = page_directory_address | PAGE_FLAG_KERNEL;
    }

    page_directory_t *child_page_directory = (page_directory_t *)page_directory_address;
    for (int i = 0; i < 1024; i++)
        child_page_directory->entries[i] = 0;

    copy_page_directory(child_page_directory, current_process->page_directory);

    registers_t child_regs;
    memcpy(&child_regs, regs, sizeof(registers_t));
    child_regs.eax = 0;

    pcb_t *child_process = create_process_control_block(child_page_directory, child_regs, current_process->pid, NULL);

    regs->eax = child_process->pid;
    scheduler_enqueue(child_process);
}

void handle_syscall_read(registers_t *regs) {
    int fd = regs->ebx;
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;

    if (stdin.count < len) {
        stdin_block_process(regs, len);
    }
}

void handle_syscall_write(registers_t *regs) {
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;

    if (strcmp(buf, "\033[2J\033[H")) {
        clear_screen();
        return;
    }

    for (uint32_t i = 0; i < len; i++) {
        print_char(buf[i]);
    }
    vga_cursor_floor = vga_index;
    regs->eax = len;
}

void handle_syscall_exec(registers_t *regs) {
    char *buf = (char *)regs->ecx;

    ramfs_node_t *file = ramfs_find(buf);

    if (file == NULL) {
        return;
    }

    page_directory_t *current_page_directory = current_process->page_directory;
    clear_page_directory(current_page_directory);
    update_page_directory(current_page_directory, file->data, file->size, regs);
}

void syscall_handler_main(registers_t *regs) {
    uint32_t syscall_num = regs->eax;

    switch (syscall_num) {
    case SYSCALL_EXIT:
        handle_syscall_exit();
        break;
    case SYSCALL_FORK:
        handle_syscall_fork(regs);
        break;
    case SYSCALL_WRITE:
        handle_syscall_write(regs);
        break;
    case SYSCALL_READ:
        handle_syscall_read(regs);
        break;
    case SYSCALL_EXEC:
        handle_syscall_exec(regs);
        break;
    default:
        print("Unknown syscall\n");
        regs->eax = -1;
    }
}

void init_syscall() { idt_set_gate(0x80, (unsigned int)syscall_handler, KERNEL_CODE_SEGMENT, RING3_CALL); }
