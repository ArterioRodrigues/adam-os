#include "../pch.h"

void memcpy(void *dest, const void *src, uint32_t n) {
  char *d = (char *)dest;
  char *s = (char *)src;
  while (n--) {
    *d++ = *s++;
  }
}

void memset(void *ptr, int value, uint32_t n) {
  char *d = (char *)ptr;
  while (n--) {
    *d++ = value;
  }
}

int memcmp(void *ptr1, void *ptr2, uint32_t n) {
  char *p1 = (char *)ptr1;
  char *p2 = (char *)ptr2;

  while (n--) {
    if (*p1 != *p2)
      return -1;
  }
  return 0;
}
