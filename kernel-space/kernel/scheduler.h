#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process-control-block.h"

extern pcb_t *current_process;
extern pcb_t *scheduler_head_ptr;

extern uint32_t quantum_counter;
extern uint32_t process_queue_size;

void init_scheduler(pcb_t *pcb);
void scheduler_enqueue(pcb_t *pcb);
bool scheduler_remove(uint32_t pid);
void scheduler_wake(uint32_t pid);
void next_process();

void start_scheduler();
void update_scheduler(registers_t *regs);
void set_process_ready(unsigned int pid);

void dump_processes();
void debug_scheduler();
void reparent_children(uint32_t dying_pid);
#endif
