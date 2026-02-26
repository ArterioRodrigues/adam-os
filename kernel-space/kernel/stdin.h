#ifndef STDIN_H
#define STDIN_H
#include "process-control-block.h"

typedef struct wait_queue {
    pcb_t *process;
    int requested_len;
} wait_queue_t;

typedef struct stdin_buffer {
    char buffer[STANDARD_IN_BUFFER_SIZE];
    int read_index;
    int write_index;
    int count;
    wait_queue_t wait_queue;
} stdin_buffer_t;

extern stdin_buffer_t stdin;

void stdin_write(char c);
void stdin_read(char *buf, int len);
void stdin_wake_process(registers_t *regs);
void stdin_block_process(registers_t *regs, int len);

#endif
