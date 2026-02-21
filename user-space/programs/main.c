extern void sys_write(char *buf, int len);

void main() {
    char msg[] = "1";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}
