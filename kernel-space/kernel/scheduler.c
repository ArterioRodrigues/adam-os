#include "../pch.h"

pcb_t *current_process = NULL;
pcb_t *scheduler_head_ptr = NULL;
uint32_t quantum_counter = 0;
uint32_t process_queue_size = 0;

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
void init_scheduler(pcb_t *pcb) {
    scheduler_head_ptr = pcb;
    current_process = pcb;
    process_queue_size++;

    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
    enable_paging();
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
        if (!current_process)
            current_process = scheduler_head_ptr;
        size--;
    }

    if (current_process->status == WAITING)
        current_process = NULL;
}

void start_scheduler() {
    if (!scheduler_head_ptr)
        return;

    switch_to_process(current_process);
}
void update_scheduler(registers_t *regs) {
    quantum_counter++;

    if (!scheduler_head_ptr || !current_process || quantum_counter < SCHEDULER_QUANTUM)
        return;

    quantum_counter = 0;
    memcpy(&current_process->registers, regs, sizeof(registers_t));

    next_process();

    if (current_process == NULL) {
        dump_current_process();
        print("NULL PROCESS");
        while (1)
            ;
    }

    current_process->status = RUNNING;
    memcpy(regs, &current_process->registers, sizeof(registers_t));
    set_kernel_stack(current_process->kernel_stack);
    load_page_directory(current_process->page_directory);
}
