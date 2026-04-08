#ifndef MALLOC_H
#define MALLOC_H
#include "../../shared/types.h"

typedef struct heap_block_header {
    bool is_free;
    uint32_t size;

    struct heap_block_header *next;
} heap_block_header_t;

extern heap_block_header_t *heap_head_ptr;
void *malloc(uint32_t size);
void free(void *ptr);

#endif
