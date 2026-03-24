#include "../pch.h"

pcb_t *current_process = NULL;
pcb_t *scheduler_head_ptr = NULL;

uint32_t foreground_pid = 0;
uint32_t quantum_counter = 0;
uint32_t process_queue_size = 0;

bool enable_scheduler = false;

void dump_current_process() {
    char buf[20];
    if (!current_process) {
        print("current_process: NULL\n");
        return;
    }
    print("pid=");
    print(itos(buf, current_process->pid));
    print(" status=");
    if (current_process->status == RUNNING)
        print("RUNNING");
    if (current_process->status == READY)
        print("READY");
    if (current_process->status == WAITING)
        print("WAITING");
    if (current_process->status == ZOMBIE)
        print("ZOMBIE");
    print(" eip=");
    itohs(buf, current_process->registers.eip);
    print(buf);
    print(" kstack=");
    itohs(buf, current_process->kernel_stack);
    print(buf);
    print("\n");
}

void dump_processes() {
    pcb_t *p = scheduler_head_ptr;
    char buf[10];
    while (p) {
        print(itos(buf, p->pid));
        print("  ");
        print(itos(buf, p->parent_pid));
        print("  ");
        switch (p->status) {
        case 0:
            print("RUNNING");
            break;
        case 1:
            print("READY  ");
            break;
        case 2:
            print("WAITING");
            break;
        case 3:
            print("ZOMBIE ");
            break;
        default:
            print("UNKNOWN");
            break;
        }
        print("\n");
        p = p->next;
    }
}

void init_scheduler(pcb_t *pcb) {
    scheduler_head_ptr = pcb;
    current_process = pcb;
    process_queue_size++;
}

void scheduler_enqueue(pcb_t *pcb) {
    if (!scheduler_head_ptr) {
        scheduler_head_ptr = pcb;
        return;
    }

    pcb_t *current = scheduler_head_ptr;
    while (current->next) {
        current = current->next;
    }

    current->next = pcb;
    process_queue_size++;
}

void next_process() {
    current_process = current_process->next;
    uint32_t size = process_queue_size;
    if (!current_process)
        current_process = scheduler_head_ptr;

    while (size > 0 && current_process->status == WAITING) {
        current_process = current_process->next;
        if (current_process == NULL)
            current_process = scheduler_head_ptr;
        size--;
    }

    if (current_process->status == WAITING || current_process == NULL) {
        print("ERROR: NULL PROCESS");
        while (1)
            ;
    }
}

void start_scheduler() {
    if (!scheduler_head_ptr)
        return;

    enable_scheduler = true;
    scheduler_head_ptr->status = RUNNING;

    set_kernel_stack(scheduler_head_ptr->kernel_stack);
    load_page_directory(scheduler_head_ptr->page_directory);

    enable_paging();

    switch_to_process(scheduler_head_ptr);
}

bool decrement_wait_process() {
    bool woke = false;
    pcb_t *process = scheduler_head_ptr;
    while (process) {
        if (process->sleep_ticks > 0) {
            process->sleep_ticks--;
            if (process->sleep_ticks == 0 && process->status == WAITING) {
                process->status = READY;
                woke = true;
            }
        }
        process = process->next;
    }

    return woke;
}

void update_scheduler(registers_t *regs) {
    if (!enable_scheduler)
        return;

    bool woke = decrement_wait_process();
    quantum_counter++;
    if (current_process && !woke && quantum_counter < SCHEDULER_QUANTUM &&
        (current_process->status == READY || current_process->status == RUNNING)) {
        return;
    }

    if (current_process->pid == 3)
        dump_current_process();
    if (current_process && current_process->status == ZOMBIE) {
        int pid = current_process->pid;
        scheduler_wake(current_process->pid);
        next_process();
        scheduler_remove(pid);
    } else {
        memcpy(&current_process->registers, regs, sizeof(registers_t));
        next_process();
    }

    current_process->status = RUNNING;
    memcpy(regs, &current_process->registers, sizeof(registers_t));
    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
    quantum_counter = 0;
}

bool scheduler_remove(uint32_t pid) {
    if (pid == 1 || pid == 2)
        return false;

    if (pid == current_process->pid) {
        current_process->status = ZOMBIE;
        return true;
    }

    reparent_children(pid);
    pcb_t *current = scheduler_head_ptr;
    pcb_t *prev = NULL;

    while (current) {
        if (current->pid == pid) {
            if (prev == NULL)
                scheduler_head_ptr = current->next;
            else
                prev->next = current->next;

            process_queue_size--;

            clear_page_directory(current->page_directory);
            for (int i = 0; i < MAX_FDS; i++) {
                file_descriptor_t *process_fd = &current->fds[i];
                if (!process_fd->is_open || process_fd->type != FD_FILE)
                    continue;

                fat16_fd_t *fat_fd = (fat16_fd_t *)(process_fd->data);
                kfree(fat_fd->data);
                kfree(fat_fd);
            }

            kfree((void *)((uint32_t)(current->kernel_stack - PAGE_SIZE)));
            kfree(current);
            return true;
        }

        prev = current;
        current = current->next;
    }

    return true;
}

void reparent_children(uint32_t dying_pid) {
    pcb_t *current = scheduler_head_ptr;
    pcb_t *dying_process;

    while (current) {
        if (current->pid == dying_pid) {
            dying_process = current;
            break;
        }
        current = current->next;
    }

    current = scheduler_head_ptr;

    while (current) {
        if (current->parent_pid == dying_pid) {
            current->parent_pid = dying_process->parent_pid;

            pcb_t *grandparent = scheduler_head_ptr;
            while (grandparent) {
                if (grandparent->waiting_pid == dying_pid) {
                    grandparent->waiting_pid = current->pid;
                    break;
                }
                grandparent = grandparent->next;
            }
        }
        current = current->next;
    }
}

void scheduler_wake(uint32_t pid) {
    pcb_t *current = scheduler_head_ptr;

    while (current) {
        if (current->waiting_pid == pid) {
            current->status = READY;
            return;
        }

        current = current->next;
    }
}

void sigint_foreground() {
    print("^C\n");
    stdin.count = 0;
    stdin.read_index = stdin.write_index;

    if (foreground_pid == 0)
        return;

    if (stdin.wait_queue.process && stdin.wait_queue.process->pid == foreground_pid) {
        stdin.wait_queue.process = NULL;
        stdin.wait_queue.requested_len = -1;
    }

    pcb_t *p = scheduler_head_ptr;

    while (p) {
        if (p->pid == foreground_pid) {
            p->status = ZOMBIE;
            break;
        }
        p = p->next;
    }

    foreground_pid = 0;
}
