extern void sys_write(char *buf, int len);
extern char sys_read(int fd, char *buf, int len);

void wait() {
    int i = 0;
    int j = 0;
    while (i < 10000) {
        while (j < 10000)
            j++;
        i++;
        j = 0;
    }
}
void main() {
    char buf[]="12345";
    while (1) {
        wait();
        sys_read(7, buf, 10);
        sys_write(buf, 5);
    }
}
