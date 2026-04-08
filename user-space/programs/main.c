#include "../lib/malloc.h"
extern void sys_exec(char *filname, char *buf);

void main() {
    int *x = malloc(sizeof(int) * 10);
  
    for(int i = 0;  i < 10; i++)
      x[i] = i;
    
    sys_exec("shell", "");
    while (1)
        ;
}

