#ifndef KMALLOC_H
#define KMALLOC_H

#include "types.h"

typedef struct heap_block_header {
  uint32_t size;
  bool is_free;
  struct heap_block_header *next;
} heap_block_header_t;

static heap_block_header_t *head;

void init_heap();

void *kmalloc(uint32_t size);
void kfree(void *header);

void dump(heap_block_header_t *ptr);
void dump_heap();

#endif
