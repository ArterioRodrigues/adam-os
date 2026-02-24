#include "../lib/helpers.h"

void main() {
    char buf[5];
    while (1) {
        wait();
        sys_read(0, buf, 5);
        sys_write(buf, 5);
        sys_write(".." , 2);
    }
}
