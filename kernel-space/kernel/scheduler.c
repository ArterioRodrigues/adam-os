#include "../pch.h"
#include "page-table.h"

pcb_t *current_process = NULL;
pcb_t *scheduler_head_ptr = NULL;
static uint32_t quantum_counter = 0;

void init_scheduler(pcb_t *pcb) {
    scheduler_head_ptr = pcb;
    current_process = pcb;
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
}

void next_process() {
    current_process = current_process->next;
    if (!current_process)
        current_process = scheduler_head_ptr;
}

void start_scheduler() {
    if (!scheduler_head_ptr)
        return;

    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
    enable_paging();
    switch_to_process(current_process);
}

void update_scheduler(registers_t *regs) {
    quantum_counter++;
    if (!scheduler_head_ptr || quantum_counter < SCHEDULER_QUANTUM)
        return;

    quantum_counter = 0;
    memcpy(&current_process->registers, regs, sizeof(registers_t));
    current_process->status = READY;

    next_process();

    current_process->status = RUNNING;
    memcpy(regs, &current_process->registers, sizeof(registers_t));

    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
}
