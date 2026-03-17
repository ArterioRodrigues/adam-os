#ifndef PROCESS_CONTROL_BLOCK_H
#define PROCESS_CONTROL_BLOCK_H

#include "fdh.h"
#include "page-table.h"
#include "types.h"

typedef enum { RUNNING, READY, WAITING, ZOMBIE } process_status_t;

typedef struct pcb {
    registers_t registers;
    page_directory_t *page_directory;

    process_status_t status;
    struct pcb *next;

    uint32_t kernel_stack;
    uint32_t pid;
    uint32_t parent_pid;

    int exit_code;
    file_descriptor_t fds[MAX_FDS];
    char *name;
    int waiting_pid;
    uint32_t sleep_ticks;
} pcb_t;

typedef struct {
    uint32_t pid;
    uint32_t parent_pid;
    process_status_t status;
} ps_entry_t;

extern uint32_t pid_static;

extern void switch_to_process(pcb_t *pcb);
pcb_t *create_process_control_block(page_directory_t *page_directory, registers_t registers, uint32_t parent_pid,
                                    pcb_t *next);

#endif
