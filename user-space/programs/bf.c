#include "../lib/lib.h"
#include "../lib/string.h"

char program[4096];
char tape[256];

void main() {
    char buf[8];
    print("enter bf file: ");
    sys_read(0, buf, 8);

    int fd = sys_open(buf);
    if (fd == -1) {
        print("No file found\n");
        return;
    }

    int size = sys_read(fd, program, 4096);
    sys_close(fd);

    int ptr = 0;
    int pc = 0;

    while (pc < size) {
        char c = program[pc];

        if (c == '>')
            ptr++;
        else if (c == '<')
            ptr--;
        else if (c == '+')
            tape[ptr]++;
        else if (c == '-')
            tape[ptr]--;
        else if (c == '.')
            sys_write(1, &tape[ptr], 1);
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
        } else if (c == ']') {
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

    return;
}
