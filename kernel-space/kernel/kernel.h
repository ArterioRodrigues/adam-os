#ifndef KERNEL_H
#define KERNEL_H

#include "page-table.h"
registers_t make_initial_registers(uint32_t entry_vaddr, uint32_t stack_vaddr);
page_directory_t *create_kernel_page_directory(void *fn);
#endif
