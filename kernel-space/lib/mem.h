#ifndef MEM_H
#define MEM_H
#include "../kernel/types.h"

void memcpy(void *dest, const void *src, uint32_t n);
void memset(void *ptr, int value, uint32_t n);
int memcmp(void *ptr1, void *ptr2, uint32_t n);

#endif
