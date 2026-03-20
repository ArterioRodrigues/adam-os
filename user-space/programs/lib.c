#include "../lib/lib.h"

void print(const char *s) { sys_write(1, (char *)s, strlen(s)); }
