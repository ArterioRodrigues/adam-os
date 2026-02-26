#include "../lib/lib.h"

void main() {
    sys_write("launching shell\n", 16);

    sys_exec("/shell");

    sys_write("\nexec failed", 12);
    sys_exit();
}
