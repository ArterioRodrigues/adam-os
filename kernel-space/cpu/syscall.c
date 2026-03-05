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

    file_descriptor_t *process_fd = &current_process->fds[fd];
    switch (process_fd->type) {
    case FD_STDIN:
        if (stdin.count < len)
            stdin_block_process(regs, len);
        break;
    case FD_FILE:
        if (!process_fd->is_open)
            print("ERROR: FILE NOT OPEN");
        fat16_fd_t *fat_fd = (fat16_fd_t *)process_fd->data;
        uint32_t size = min(len, fat_fd->size - process_fd->offset);
        memcpy(buf, fat_fd->data + process_fd->offset, size);
        process_fd->offset += size;
        regs->eax = size;
        break;
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

void handle_syscall_open(registers_t *regs) {
    char *file_name = (char *)regs->ebx;
    fat16_entry_t *entry = fat16_find_file(file_name);
    if (!entry) {
        print("ERROR: FILE NOT FOUND");
        return;
    }

    fat16_fd_t *fat_fd = kmalloc(sizeof(fat16_fd_t));
    fat_fd->data = kmalloc(entry->file_size + 512);
    fat_fd->size = fat16_read_file(entry, fat_fd->data);

    for (int i = 2; i < MAX_FDS; i++) {
        if (!current_process->fds[i].is_open) {
            current_process->fds[i].type = FD_FILE;
            current_process->fds[i].data = fat_fd;
            current_process->fds[i].offset = 0;
            current_process->fds[i].is_open = true;
            regs->eax = i;
            return;
        }
    }

    kfree(fat_fd->data);
    print("ERROR: TO MANY FILE OPEN");
    return;
}

void handle_syscall_close(registers_t *regs) {
    int fd = regs->ebx;

    file_descriptor_t *process_fd = &current_process->fds[fd];

    if (!process_fd->is_open) {
        regs->eax = -1;
        return;
    }

    fat16_fd_t *fat_fd = (fat16_fd_t *)process_fd->data;
    kfree(fat_fd->data);
    kfree(fat_fd);
    process_fd->data = NULL;
    process_fd->is_open = false;
    process_fd->is_open = false;

    regs->eax = 0;
}

void handle_syscall_exec(registers_t *regs) {
    char *file_name = (char *)regs->ecx;

    fat16_entry_t *entry = fat16_find_file(file_name);

    if (!entry)
        return;

    uint8_t *data = kmalloc(entry->file_size + 512);
    fat16_read_file(entry, (uint8_t *)data);

    page_directory_t *current_page_directory = current_process->page_directory;
    clear_page_directory(current_page_directory);
    update_page_directory(current_page_directory, data, entry->file_size, regs);
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
    case SYSCALL_OPEN:
        handle_syscall_open(regs);
        break;
    case SYSCALL_CLOSE:
        handle_syscall_close(regs);
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
