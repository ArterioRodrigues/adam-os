#include "../lib/lib.h"
#include "../lib/string.h"

char program[4096];
char tape[256];

void main(char *arg) {

    int fd = sys_open(arg);
    if (fd == -1) {
        print("no file found\n");
        return;
    }

    int size = sys_read(fd, program, 4096);
    sys_close(fd);

    for (int i = 0; i < 256; i++)
        tape[i] = 0;

    int ptr = 0;
    int pc = 0;

    while (pc < size) {
        char c = program[pc];

        if (c == '>')
            ptr = (ptr + 1) & 0xFF;
        else if (c == '<')
            ptr = (ptr - 1) & 0xFF;
        else if (c == '+')
            tape[ptr]++;
        else if (c == '-')
            tape[ptr]--;
        else if (c == '.') {
            char *buf = &tape[ptr];
            sys_write(0, buf, 1);
        }

        else if (c == ',') {
            char ch[1];
            sys_read(0, ch, 1);
            tape[ptr] = *ch;
        }

        else if (c == '[') {
            if (tape[ptr] == 0) {
                int depth = 1;
                while (depth > 0) {
                    pc++;
                    if (program[pc] == '[')
                        depth++;
                    if (program[pc] == ']')
                        depth--;
                }
            }
        }

        else if (c == ']') {
            if (tape[ptr] != 0) {
                int depth = 1;
                while (depth > 0) {
                    pc--;
                    if (program[pc] == ']')
                        depth++;
                    if (program[pc] == '[')
                        depth--;
                }
            }
        }

        pc++;
    }

    print("\n");
    return;
}
