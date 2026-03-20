extern void sys_exec(char *filname, char *buf);

void main() {
    sys_exec("shell", "");
    while (1)
        ;
}

