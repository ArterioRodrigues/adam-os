#include "../pch.h"

void test_user_function() {
    char msg[] = "HELLO FROM USER\n";
    sys_write(msg, 16);
    sys_exit();
    while (1)
        ;
}
