#include "../lib/lib.h"
#include "../lib/string.h"

void main() {
    print("Adam OS\n");

    sys_exec("/shell");

    print("\nexec failed");
    sys_exit();
}
