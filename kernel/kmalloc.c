#include "../pch.h"

static heap_block_header_t *head;

void init_heap() {
    head = (void *)HEAP_START;
    head->size = HEAP_SIZE - sizeof(heap_block_header_t);
    head->is_free = true;
    head->next = NULL;
}

void split_block(heap_block_header_t *heap_ptr, uint32_t size) {
    uint32_t remaining_size = heap_ptr->size - size - sizeof(heap_block_header_t);

    if (remaining_size < 16) {
        heap_ptr->size = size;
        heap_ptr->is_free = false;
        return;
    }

    heap_block_header_t *new_block = (heap_block_header_t *)((char *)(heap_ptr + 1) + size);
    new_block->size = remaining_size;
    new_block->is_free = true;
    new_block->next = heap_ptr->next;

    heap_ptr->size = size;
    heap_ptr->is_free = false;
    heap_ptr->next = new_block;
}

void *kmalloc(uint32_t size) {
    if (size <= 0)
        return NULL;

    heap_block_header_t *heap_ptr = head;

    while (heap_ptr && !(heap_ptr->is_free && heap_ptr->size >= size))
        heap_ptr = heap_ptr->next;

    if (!heap_ptr)
        return NULL;

    split_block(heap_ptr, size);

    return (void *)(heap_ptr + 1);
}

void coalesce_blocks() {
    heap_block_header_t *block = head;
    heap_block_header_t *prev = NULL;

    while (block) {
        if (prev->is_free && block->is_free) {
            prev->size += block->size + sizeof(heap_block_header_t);
            prev->next = block->next;
            block = block->next;
        } else {
            prev = block;
            block = block->next;
        }
    }
}

void kfree(void *ptr) {
    if (!ptr) {
        return;
    }
    heap_block_header_t *header = (heap_block_header_t *)ptr - 1;
    header->is_free = true;
    coalesce_blocks();
}

void dump_heap() {
    print("\n");
    heap_block_header_t *curr = head;
    uint32_t size = 0;
    uint32_t free_size = 0;
    uint32_t not_free_size = 0;

    while (curr) {
        dump(curr);
        size += curr->size;
        free_size += curr->is_free ? 1 : 0;
        not_free_size += curr->is_free ? 0 : 1;
        curr = curr->next;
    }

    char result[20];
    printf("Allocated size: ", itos(result, size), "\n");
    printf("Number of free blocks: ", itos(result, free_size), "\n");
    printf("Number of not free blocks: ", itos(result, not_free_size), "\n");
}

void dump(heap_block_header_t *ptr) {
    char buf[50];
    print(ptr->is_free ? "F=1 " : "F=0 ");

    itos(buf, ptr->size);
    print("S=");
    print(buf);

    itohs(buf, (int)ptr);
    print(" C=");
    print(buf);

    if (ptr->next) {
        itohs(buf, (int)ptr->next);
        print(" N=");
        print(buf);
    } else {
        print(" N=0");
    }
    print("\n");
}
