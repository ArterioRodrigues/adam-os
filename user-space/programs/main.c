#include "../lib/lib.h"

void main() {
    sys_exec("shell", "");
    print("Error: exec failed");
    while (1)
        ;
}

