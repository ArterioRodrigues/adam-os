#ifndef KERNEL_H
#define KERNEL_H

#include "page-table.h"

extern uint8_t _binary_idle_bin_start[];
extern uint8_t _binary_shell_bin_start[];
extern uint8_t _binary_main_bin_start[];

registers_t make_initial_registers(uint32_t entry_vaddr, uint32_t stack_vaddr);
page_directory_t *create_kernel_page_directory(void *fn);
#endif
