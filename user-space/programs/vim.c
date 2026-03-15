#include "../lib/lib.h"
#include "../lib/string.h"

char vim_buffer[2000];

void main(char *arg) {

    char file[8];
    memcpy(file, arg, 8);

    print("\033[2J\033[H");
    print("VIM\n");

    int fd = sys_open(file);
    if (fd == -1) {
        sys_create(file, vim_buffer, 2000);
        fd = sys_open(file);
    } else {
        sys_read(fd, vim_buffer, 2000);

        int size = 0;
        while (vim_buffer[size] != '\0')
            size++;

        sys_write(0, vim_buffer, size);
    }

    int size = 0;
    while (size < 2000) {
        size += sys_read(0, vim_buffer + size, 2000 - size);

        if (size >= 5 && strcmp(vim_buffer + size - 5, "/quit"))
            break;
    }

    if (size >= 5 && strcmp(vim_buffer + size - 5, "/quit"))
        size -= 5;

    vim_buffer[size] = '\0';

    print("\n");
    if (size <= 0)
        return;

    sys_write(fd, vim_buffer, size);
    sys_close(fd);

    print("\033[2J\033[H");
    return;
}
