#include "../pch.h"

static heap_block_header_t *head;

void init_heap() {
  head = (void *)HEAP_START;
  head->size = HEAP_SIZE - sizeof(heap_block_header_t);
  head->is_free = true;
  head->next = NULL;
}

void *kmalloc(uint32_t n) {
  heap_block_header_t *heap_ptr = head;

  while (heap_ptr) {
    if (heap_ptr->is_free && heap_ptr->size >= n) {
      heap_ptr->size = n;
      heap_ptr->is_free = false;

      if (heap_ptr->next == NULL) {
        heap_ptr->next = (heap_block_header_t *)((char *)(heap_ptr + 1) + n);
        heap_ptr->next->size = (void *)HEAP_END - (void *)(heap_ptr->next + 1);
        heap_ptr->next->is_free = true;
        heap_ptr->next->next = NULL;
      }
      break;
    }

    heap_ptr = heap_ptr->next;
  }

  if (!heap_ptr) {
    return NULL;
  }

  return (void *)(heap_ptr + 1);
}

void kfree(void *ptr) {
  if (!ptr) {
    return;
  }
  heap_block_header_t *header = (heap_block_header_t *)ptr - 1;
  header->is_free = true;
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
