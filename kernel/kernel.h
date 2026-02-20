#ifndef KERNEL_H
#define KERNEL_H

#include "page-table.h"
page_directory_t *create_kernel_page_directory(void * fn);
#endif
