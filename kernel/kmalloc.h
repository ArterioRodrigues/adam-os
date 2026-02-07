#ifndef KMALLOC_H
#define KMALLOC_H

#include "types.h"

struct heap_block_header {
  uint32_t size;
  bool is_free;
  struct heap_block_header *next;
};

static struct heap_block_header *head;

void *kmalloc(uint32_t n);
void kfree(void *header);
void init_heap();

void dump(struct heap_block_header *ptr);

#endif
