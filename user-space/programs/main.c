#include "../lib/lib.h"

void main() {
    int pid = sys_fork();

    if (pid == 0) {
    
        while (1) {
            sys_write("child\n", 6);
            wait();
        }
    } else {
        while (1) {
            sys_write("parent\n", 7);
            wait();
        }
    }
}
