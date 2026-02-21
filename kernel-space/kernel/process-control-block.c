#include "../pch.h"

pcb_t *create_process_control_block(page_directory_t *page_directory, registers_t registers, uint32_t parent_pid,
                                    pcb_t *next) {
    pcb_t *pcb = kmalloc(sizeof(pcb_t));

    memcpy(&pcb->registers, &registers, sizeof(registers_t));
    pcb->page_directory = page_directory;

    uint32_t *stack_mem = kmalloc(PAGE_SIZE);
    pcb->kernel_stack = (uint32_t)(stack_mem + PAGE_SIZE);

    pcb->pid = pid_static++;
    pcb->parent_pid = parent_pid;
    pcb->status = READY;
    pcb->next = next;

    for (int i = 0; i < MAX_FDS; i++) {
        pcb->fds[i].type = FD_NONE;
        pcb->fds[i].data = NULL;
        pcb->fds[i].offset = 0;
        pcb->fds[i].is_open = false;
    }

    pcb->fds[0].type = FD_STDIN;
    pcb->fds[0].is_open = true;

    pcb->fds[1].type = FD_STDOUT;
    pcb->fds[1].is_open = true;

    return pcb;
}
