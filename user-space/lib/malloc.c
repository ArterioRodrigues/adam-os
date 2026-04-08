#include "malloc.h"
#include "lib.h"

heap_block_header_t *heap_head_ptr = NULL;

void create_heap_block(heap_block_header_t **ptr) {
    heap_block_header_t *block = (heap_block_header_t *)sys_sbrk(PAGE_SIZE);
    block->is_free = true;
    block->size = PAGE_SIZE - sizeof(heap_block_header_t);
    block->next = NULL;
    *ptr = block;
}

void split_block(heap_block_header_t *ptr, uint32_t size) {
    uint32_t remaining_size = ptr->size - size - sizeof(heap_block_header_t);

    if (remaining_size < sizeof(heap_block_header_t) * 3) {
        ptr->is_free = false;
        return;
    }

    heap_block_header_t *new_block = (heap_block_header_t *)((char *)(ptr + 1) + size);
    new_block->size = remaining_size;
    new_block->is_free = true;
    new_block->next = ptr->next;

    ptr->size = size;
    ptr->is_free = false;
    ptr->next = new_block;
}

void *malloc(uint32_t size) {
    if (!heap_head_ptr)
        create_heap_block(&heap_head_ptr);

    heap_block_header_t *current = heap_head_ptr;
    heap_block_header_t *tail = NULL;

    while (current && !current->is_free && size < current->size) {
        tail = current;
        current = current->next;
    }

    if (!current) {
        create_heap_block(&tail->next);
        current = tail->next;
    }

    split_block(current, size);

    return (void *)(current + 1);
}
void free(void *ptr) {
    if (!ptr)
        return;

    heap_block_header_t *current = (heap_block_header_t *)ptr - 1;
    current->is_free = true;
}
