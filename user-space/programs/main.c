#include "../lib/lib.h"
#include "../lib/string.h"


void main() {
    sys_exec("shell", "");
    print("Error: exec failed");
    while (1)
        ;
}

