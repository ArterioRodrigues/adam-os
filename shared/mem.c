#include "mem.h"

void memcpy(void *dest, const void *src, uint32_t n) {
    uint32_t dwords = n / 4;
    uint32_t remain = n % 4;

    asm volatile("rep movsl"
        : "+D"(dest), "+S"(src), "+c"(dwords)
        :
        : "memory");

    unsigned char *d = dest;
    const unsigned char *s = src;
    while (remain--)
        *d++ = *s++;
}

void temp_memcpy(void *dest, const void *src, uint32_t n) {
    unsigned char *d = dest;
    unsigned char *s = (char *)src;

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
        p1++;
        p2++;
    }
    return 0;
}
