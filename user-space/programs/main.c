#include "../lib/lib.h"
#include "../lib/string.h"


void main() {
    sys_exec("shell");
    print("Error: exec failed");

    int fd = sys_open("/");
    char buf[100];
    int size = sys_read(fd, buf, 100);
    sys_write(0, buf, size);

    while (1)
        ;
}
