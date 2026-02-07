#include "kmalloc.h"
#include "../pch.h"
#include "config.h"

static struct heap_block_header *head;

void init_heap() {
  head = (void *)HEAP_START;
  head->size = HEAP_SIZE - sizeof(struct heap_block_header);
  head->is_free = true;
  head->next = NULL;
}

void *kmalloc(uint32_t n) {
  struct heap_block_header *heap_ptr = head;

  while (heap_ptr) {
    if (heap_ptr->is_free && heap_ptr->size >= n) {
      heap_ptr->size = n;
      heap_ptr->is_free = false;

      if (heap_ptr->next == NULL) {
        heap_ptr->next =
            (struct heap_block_header *)((char *)(heap_ptr + 1) + n);
        heap_ptr->next->size = (void *)HEAP_END - (void *)(heap_ptr->next + 1);
        heap_ptr->next->is_free = true;
        heap_ptr->next->next = NULL;
      }

      dump(heap_ptr);
      break;
    }

    dump(heap_ptr);
    heap_ptr = heap_ptr->next;
  }

  if (!heap_ptr) {
    print("ERROR: Out of heap memory!\n");
    return NULL;
  }

  return (void *)(heap_ptr + 1);
}

void kfree(void *ptr) {
  if (!ptr) {
    return;
  }
  struct heap_block_header *header = (struct heap_block_header *)ptr - 1;
  header->is_free = true;
}

void dump(struct heap_block_header *ptr) {
  print("\n");
  if (ptr->is_free == true)
    print("is_free: true");
  else
    print("is_free: false");

  char result[50];
  int_to_string(result, ptr->size);

  if (ptr->size) {
    print("\nsize: ");
    print(result);
  } else
    print("\nsize: 0");

  int_to_hex_string(result, (int)ptr);
  print("\ncurr: ");
  print(result);

  print("  ");
  int_to_string(result, (int)ptr);
  print(result);

  if (ptr->next) {
    int_to_hex_string(result, (int)ptr->next);
    print("\nnext: ");
    print(result);

    print("  ");
    int_to_string(result, (int)ptr->next);
    print(result);
  } else {
    print("\nnext: NULL");
  }

  print("\n");
}
