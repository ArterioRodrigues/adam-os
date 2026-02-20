#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process-control-block.h"

extern pcb_t *current_process;
extern pcb_t *scheduler_head_ptr;

static uint32_t quantum_counter;

void init_scheduler(pcb_t *pcb);
void scheduler_enqueue(pcb_t *pcb);
void next_process();

void start_scheduler();
void update_scheduler(registers_t *regs);

void debug_scheduler(); 
#endif
