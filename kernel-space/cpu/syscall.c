#include "../pch.h"

extern void syscall_handler();

static inline file_descriptor_t *get_fd(int fd) {
    if (fd < 0 || fd >= MAX_FDS)
        return NULL;
    return &current_process->fds[fd];
}

static uint32_t parse_ansi_escape(char *buf, uint32_t len) {
    if (len == 7 && buf[0] == '\033' && buf[1] == '[' && buf[2] == '2' && buf[3] == 'J' && buf[4] == '\033' &&
        buf[5] == '[' && buf[6] == 'H') {
        clear_terminal();
        return len;
    }

    uint8_t color = WHITE;
    for (uint32_t i = 0; i < len; i++) {
        if (i + 1 < len && buf[i] == '\033' && buf[i + 1] == '[') {
            int code = stoi(&buf[i + 2]);
            if (code >= 30 && code <= 50)
                color = ansi_to_terminal[code - 30];
            else if (code == 0)
                color = WHITE;
            char tmp[12];
            i += strlen(itos(tmp, code)) + 2;
            continue;
        }
        terminal_print_char_color(buf[i], color);
    }
    return len;
}

static void write_file(file_descriptor_t *fd, const char *buf, uint32_t len, registers_t *regs) {
    if (!fd->is_open) {
        regs->eax = (uint32_t)-1;
        return;
    }

    fat16_fd_t *fat_fd = (fat16_fd_t *)fd->data;
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
}

static void handle_syscall_exit(registers_t *regs) {
    scheduler_remove(current_process->pid);
    update_scheduler(regs);
}

static void handle_syscall_fork(registers_t *regs) {
    uint32_t page_directory_address = allocate_frame();

    uint32_t pd_index = page_directory_address >> 22;
    uint32_t pt_index = (page_directory_address >> 12) & 0x3FF;
    page_table_t *pt = (page_table_t *)(current_process->page_directory->entries[pd_index] & 0xFFFFF000);
    pt->entries[pt_index] = page_directory_address | PAGE_FLAG_KERNEL;

    page_directory_t *child_pd = (page_directory_t *)page_directory_address;
    for (int i = 0; i < 1024; i++)
        child_pd->entries[i] = 0;
    copy_page_directory(child_pd, current_process->page_directory);

    registers_t child_regs;
    memcpy(&child_regs, regs, sizeof(registers_t));
    child_regs.eax = 0;

    pcb_t *child = create_process_control_block(child_pd, child_regs, current_process->pid, NULL);
    regs->eax = child->pid;
    scheduler_enqueue(child);
}

static void handle_syscall_exec(registers_t *regs) {
    char *file_name = (char *)regs->ecx;
    char *arg = (char *)regs->edx;

    char name_buf[64];
    char arg_buf[256];
    strcpy(name_buf, file_name);
    arg_buf[0] = '\0';
    if (arg)
        strcpy(arg_buf, arg);

    fat16_entry_t *entry = fat16_find_file(file_name);
    if (!entry) {
        regs->eax = (uint32_t)-1;
        return;
    }

    uint32_t cluster_size = bpb->sectors_per_cluster * 512;
    uint32_t num_clusters = ceil(entry->file_size, cluster_size);
    uint8_t *data = kmalloc(num_clusters * cluster_size + cluster_size);
    fat16_read_file(entry, data);

    clear_page_directory(current_process->page_directory);
    update_page_directory(current_process->page_directory, data, entry->file_size, regs);

    if (arg_buf[0] != '\0') {

        uint32_t stack_top = regs->useresp;
        uint32_t arg_len = strlen(arg_buf) + 1;
        stack_top -= arg_len;
        uint32_t str_addr = stack_top;
        memcpy((void *)str_addr, arg_buf, arg_len);
        stack_top -= 4;
        *(uint32_t *)stack_top = str_addr;
        regs->useresp = stack_top;
        regs->esp = stack_top;
    }

    kfree(entry);
    kfree(data);
}

static void handle_syscall_kill(registers_t *regs) {
    uint32_t pid = regs->ebx;
    regs->eax = scheduler_remove(pid) ? 0 : (uint32_t)-1;
}

static void handle_syscall_wait(registers_t *regs) {
    current_process->waiting_pid = regs->ebx;
    current_process->status = WAITING;
    update_scheduler(regs);
}

static void handle_syscall_ps(registers_t *regs) {
    ps_entry_t *buf = (ps_entry_t *)regs->ebx;
    int max = (int)regs->ecx;
    int count = 0;

    for (pcb_t *p = scheduler_head_ptr; p && count < max; p = p->next) {
        buf[count].pid = p->pid;
        buf[count].parent_pid = p->parent_pid;
        buf[count].status = p->status;
        count++;
    }

    regs->eax = count;
}

static void handle_syscall_sleep(registers_t *regs) {
    uint32_t ticks = regs->ebx;
    if (ticks == 0 || current_process->pid == 1)
        return;
    current_process->sleep_ticks = ticks;
    current_process->status = WAITING;
    update_scheduler(regs);
}

static void handle_syscall_uptime(registers_t *regs) { regs->eax = get_timer_ticks(); }

static void handle_syscall_read(registers_t *regs) {
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;
    file_descriptor_t *fd = get_fd(regs->ebx);

    if (!fd) {
        regs->eax = (uint32_t)-1;
        return;
    }

    switch (fd->type) {
    case FD_STDIN:
        if (stdin.count < len)
            stdin_block_process(regs, len);
        else
            regs->eax = stdin_read(buf, len);
        break;
    case FD_FILE: {
        if (!fd->is_open) {
            regs->eax = (uint32_t)-1;
            return;
        }
        fat16_fd_t *fat_fd = (fat16_fd_t *)fd->data;
        if (fat_fd->size <= fd->offset) {
            regs->eax = 0;
            break;
        }
        uint32_t size = min(len, fat_fd->size - fd->offset);
        memcpy(buf, fat_fd->data + fd->offset, size);
        fd->offset += size;
        regs->eax = size;
        break;
    }
    default:
        regs->eax = (uint32_t)-1;
        break;
    }
}

static void handle_syscall_write(registers_t *regs) {
    char *buf = (char *)regs->ecx;
    uint32_t len = regs->edx;
    file_descriptor_t *fd = get_fd(regs->ebx);

    if (!fd) {
        regs->eax = (uint32_t)-1;
        return;
    }

    switch (fd->type) {
    case FD_STDIN:
        for (uint32_t i = 0; i < len; i++)
            stdin_write(buf[i]);
        regs->eax = len;
        break;
    case FD_STDOUT:
        parse_ansi_escape(buf, len);
        terminal->cursor_floor = (TERMINAL_COLS * terminal->cursor_row) + terminal->cursor_column;
        regs->eax = len;
        break;
    case FD_FILE:
        write_file(fd, buf, len, regs);
        break;
    default:
        regs->eax = (uint32_t)-1;
        break;
    }
}

static void handle_syscall_open(registers_t *regs) {
    char *file_name = (char *)regs->ebx;
    if (!file_name) {
        regs->eax = (uint32_t)-1;
        return;
    }

    fat16_entry_t *entry = fat16_find_file(file_name);
    if (!entry) {
        regs->eax = (uint32_t)-1;
        return;
    }

    fat16_fd_t *fat_fd = kmalloc(sizeof(fat16_fd_t));
    fat_fd->attributes = entry->attributes;
    fat_fd->start_cluster = entry->start_cluster;

    if (entry->attributes & 0x10) {
        fat_fd->data = kmalloc(bpb->sectors_per_cluster * SECTOR_SIZE);
        fat_fd->size = fat16_read_folder(entry, fat_fd->data);
    } else {
        uint32_t size = entry->file_size + bpb->sectors_per_cluster * SECTOR_SIZE;
        fat_fd->data = kmalloc(size);
        fat_fd->size = fat16_read_file(entry, fat_fd->data);
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
    regs->eax = (uint32_t)-1;
}

static void handle_syscall_close(registers_t *regs) {
    file_descriptor_t *fd = get_fd(regs->ebx);
    if (!fd || !fd->is_open) {
        regs->eax = (uint32_t)-1;
        return;
    }

    fat16_fd_t *fat_fd = (fat16_fd_t *)fd->data;
    kfree(fat_fd->data);
    kfree(fat_fd);
    fd->data = NULL;
    fd->is_open = false;
    regs->eax = 0;
}

static void handle_syscall_create(registers_t *regs) {
    char *name = (char *)regs->ebx;
    uint8_t *data = (uint8_t *)regs->ecx;
    uint32_t len = regs->edx;
    regs->eax = fat16_write_file(name, data, len) ? 0 : (uint32_t)-1;
}

static void handle_syscall_poll(registers_t *regs) {
    int fd = regs->ebx;
    regs->eax = (fd == 0 && stdin.count > 0) ? 1 : 0;
}

static void handle_syscall_create_window(registers_t *regs) {
    create_window_t *params = (create_window_t *)regs->ebx;
    window_t *win =
        wm_create_window(params->x, params->y, params->width, params->height, params->title, current_process->pid);
    regs->eax = win->window_id;
}

static void handle_syscall_draw_rect(registers_t *regs) {
    create_rect_t *rect = (create_rect_t *)regs->ebx;
    window_t *win = get_window(rect->window_id);
    if (!win) {
        regs->eax = (uint32_t)-1;
        return;
    }
    window_draw_rect(win, rect->x, rect->y, rect->width, rect->height, rect->color);
}

static void handle_syscall_draw_text(registers_t *regs) {
    create_text_t *text = (create_text_t *)regs->ebx;
    window_t *win = get_window(text->window_id);
    if (!win) {
        regs->eax = (uint32_t)-1;
        return;
    }
    window_draw_string(win, text);
}

static void handle_syscall_get_event(registers_t *regs) {
    uint32_t id = regs->ebx;
    event_t *event = (event_t *)regs->ecx;

    window_t *win = get_window(id);
    if (!win) {
        regs->eax = (uint32_t)-1;
        return;
    }
    if (event_queue_is_empty(&win->event_queue)) {
        regs->eax = 0;
        return;
    }
    event_queue_pop(&win->event_queue, event);
    regs->eax = 1;
}

static void handle_syscall_destroy_window(registers_t *regs) {
    remove_window(regs->ebx);
    wm_dirty = true;
}

static void handle_syscall_flush(registers_t *regs) { wm_dirty = true; }

void syscall_handler_main(registers_t *regs) {
    switch (regs->eax) {
    case SYSCALL_EXIT:
        handle_syscall_exit(regs);
        break;
    case SYSCALL_FORK:
        handle_syscall_fork(regs);
        break;
    case SYSCALL_READ:
        handle_syscall_read(regs);
        break;
    case SYSCALL_WRITE:
        handle_syscall_write(regs);
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
    case SYSCALL_CREATE_WINDOW:
        handle_syscall_create_window(regs);
        break;
    case SYSCALL_DRAW_RECT:
        handle_syscall_draw_rect(regs);
        break;
    case SYSCALL_DRAW_TEXT:
        handle_syscall_draw_text(regs);
        break;
    case SYSCALL_GET_EVENT:
        handle_syscall_get_event(regs);
        break;
    case SYSCALL_DESTROY_WINDOW:
        handle_syscall_destroy_window(regs);
        break;
    case SYSCALL_FLUSH:
        handle_syscall_flush(regs);
        break;
    default:
        regs->eax = (uint32_t)-1;
        break;
    }
}

void init_syscall() { idt_set_gate(0x80, (unsigned int)syscall_handler, KERNEL_CODE_SEGMENT, RING3_CALL); }
