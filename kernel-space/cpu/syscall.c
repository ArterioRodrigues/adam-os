#include "../pch.h"

extern void syscall_handler();

void handle_syscall_exit(registers_t *regs) {
    scheduler_remove(current_process->pid);
    update_scheduler(regs);
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

    return;
}

void handle_syscall_kill(registers_t *regs) {
    uint32_t pid = regs->ebx;
    scheduler_remove(pid);
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
        else
            regs->eax = stdin_read(buf, len);
        break;
    case FD_FILE:
        if (!process_fd->is_open)
            print("error: file is not open");

        if (process_fd->data->size - process_fd->offset <= 0) {
            regs->eax = 0;
            break;
        }

        fat16_fd_t *fat_fd = (fat16_fd_t *)process_fd->data;
        uint32_t size = min(len, fat_fd->size - process_fd->offset);
        memcpy(buf, fat_fd->data + process_fd->offset, size);
        process_fd->offset += size;
        regs->eax = size;
        break;
    }
}

void handle_syscall_write(registers_t *regs) {
    int fd = regs->ebx;
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;
    file_descriptor_t *process_fd = &current_process->fds[fd];

    switch (process_fd->type) {
    case FD_STDIN:
        for (int i = 0; i < len; i++)
            stdin_write(buf[i]);
        break;
    case FD_STDOUT:
        uint8_t color = WHITE;

        if (strcmp(buf, "\033[2J\033[H")) {
            clear_screen();
            return;
        }

        for (uint32_t i = 0; i < len; i++) {
            if (buf[i] == '\033' && buf[i + 1] == '[') {
                int code = stoi(&buf[i + 2]);

                if (code >= 30 && code <= 50)
                    color = ansi_to_vga[code - 30];
                else if (code == 0) {
                    color = WHITE;
                }
                char tmp[12];
                i += strlen(itos(tmp, code)) + 3;
            }

            print_char_color(buf[i], color);
        }
        vga_cursor_floor = vga_index;
        regs->eax = len;
        break;
    case FD_FILE:
        if (!process_fd->is_open) {
            regs->eax = (uint32_t)-1;
            return;
        }

        fat16_fd_t *fat_fd = (fat16_fd_t *)process_fd->data;

        uint32_t to_write = min(len, fat_fd->size);
        memcpy(fat_fd->data, buf, to_write);

        uint32_t cluster_lba = geometry->data_start_lba + (fat_fd->start_cluster - 2) * bpb->sectors_per_cluster;
        uint8_t *sector = kmalloc(SECTOR_SIZE);

        for (uint32_t i = 0; i < bpb->sectors_per_cluster; i++) {
            uint32_t offset = i * SECTOR_SIZE;
            memset(sector, 0, SECTOR_SIZE);
            uint32_t remaining = (to_write > offset) ? to_write - offset : 0;
            if (remaining > 0)
                memcpy(sector, buf + offset, min(remaining, SECTOR_SIZE));
            ata_write_sector(cluster_lba + i, sector);
        }
        kfree(sector);

        regs->eax = to_write;
        break;
    }
}

void handle_syscall_open(registers_t *regs) {
    char *file_name = (char *)regs->ebx;

    if (!file_name)
        return;

    fat16_entry_t *entry = fat16_find_file(file_name);
    if (!entry) {
        regs->eax = -1;
        return;
    }

    fat16_fd_t *fat_fd;
    fat_fd = kmalloc(sizeof(fat16_fd_t));
    fat_fd->attributes = entry->attributes;

    if (entry->attributes & 0x10) {
        fat_fd->data = kmalloc(bpb->sectors_per_cluster * SECTOR_SIZE);
        fat_fd->size = fat16_read_folder(entry, fat_fd->data);
        fat_fd->start_cluster = entry->start_cluster;
    } else {
        uint32_t size = entry->file_size + bpb->sectors_per_cluster * SECTOR_SIZE;
        fat_fd->data = kmalloc(size);
        fat_fd->size = fat16_read_file(entry, fat_fd->data);
        fat_fd->start_cluster = entry->start_cluster;
    }

    for (int i = 2; i < MAX_FDS; i++) {
        if (!current_process->fds[i].is_open) {
            current_process->fds[i].type = FD_FILE;
            current_process->fds[i].data = fat_fd;
            current_process->fds[i].offset = 0;
            current_process->fds[i].is_open = true;
            regs->eax = i;
            kfree(entry);
            return;
        }
    }

    kfree(fat_fd->data);
    kfree(fat_fd);
    kfree(entry);
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
    char *arg = (char *)regs->edx;

    fat16_entry_t *entry = fat16_find_file(file_name);

    if (!entry) {
        regs->eax = (uint32_t)-1;
        return;
    }

    uint32_t cluster_size = bpb->sectors_per_cluster * 512;
    uint32_t num_clusters = ceil(entry->file_size, cluster_size);
    uint8_t *data = kmalloc(num_clusters * cluster_size + cluster_size);
    fat16_read_file(entry, (uint8_t *)data);

    page_directory_t *current_page_directory = current_process->page_directory;
    clear_page_directory(current_page_directory);
    update_page_directory(current_page_directory, data, entry->file_size, regs);

    if (arg) {
        uint32_t stack_top = regs->useresp;
        uint32_t arg_len = strlen(arg) + 1;

        stack_top -= arg_len;
        uint32_t str_addr = stack_top;
        memcpy((void *)str_addr, arg, arg_len);

        stack_top -= 4;
        *(uint32_t *)stack_top = str_addr;

        regs->useresp = stack_top;
        regs->esp = stack_top;
    }
    kfree(entry);
    kfree(data);
}

void handle_syscall_ps(registers_t *regs) {
    ps_entry_t *buf = (ps_entry_t *)regs->ebx;
    int max = (int)regs->ecx;
    int count = 0;

    pcb_t *p = scheduler_head_ptr;
    while (p && count < max) {
        buf[count].pid = p->pid;
        buf[count].parent_pid = p->parent_pid;
        buf[count].status = p->status;
        count++;
        p = p->next;
    }

    regs->eax = count;
}

void handle_syscall_wait(registers_t *regs) {
    int pid = regs->ebx;
    current_process->status = WAITING;
    current_process->waiting_pid = pid;
    update_scheduler(regs);
}

void handle_syscall_create(registers_t *regs) {
    char *name = (char *)regs->ebx;
    uint8_t *data = (uint8_t *)regs->ecx;
    uint32_t len = regs->edx;

    regs->eax = fat16_write_file(name, data, len) ? 0 : -1;
}
void handle_syscall_poll(registers_t *regs) {
    int fd = regs->ebx;
    if (fd == 0)
        regs->eax = (stdin.count > 0) ? 1 : 0;
    else
        regs->eax = 0;
}

void handle_syscall_sleep(registers_t *regs) {
    uint32_t ticks = regs->ebx;
    if (ticks == 0)
        return;
    current_process->sleep_ticks = ticks;
    current_process->status = WAITING;
    update_scheduler(regs);
}

void handle_syscall_uptime(registers_t *regs) { regs->eax = get_timer_ticks(); }

void syscall_handler_main(registers_t *regs) {
    uint32_t syscall_num = regs->eax;

    switch (syscall_num) {
    case SYSCALL_EXIT:
        handle_syscall_exit(regs);
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
    case SYSCALL_PS:
        handle_syscall_ps(regs);
        break;
    case SYSCALL_KILL:
        handle_syscall_kill(regs);
        break;
    case SYSCALL_WAIT:
        handle_syscall_wait(regs);
        break;
    case SYSCALL_CREATE:
        handle_syscall_create(regs);
        break;
    case SYSCALL_POLL:
        handle_syscall_poll(regs);
        break;
    case SYSCALL_SLEEP:
        handle_syscall_sleep(regs);
        break;
    case SYSCALL_UPTIME:
        handle_syscall_uptime(regs);
        break;
    default:
        print("Unknown syscall\n");
        regs->eax = -1;
    }
}

void init_syscall() { idt_set_gate(0x80, (unsigned int)syscall_handler, KERNEL_CODE_SEGMENT, RING3_CALL); }
