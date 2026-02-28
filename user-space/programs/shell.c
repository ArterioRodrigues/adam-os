#include "../lib/lib.h"
#include "../lib/string.h"

char shell_path[50];
char input_buf[256];

static void help_handler() {
    print("\tclear  - Clear terminal screen\n");
    print("\thelp   - Show this message\n");
    print("\texec   - Run a program\n");
}

static void clear_handler() { print("\033[2J\033[H"); }

static void exec_handler(char *arg) {
    if (arg[0] == '\0') {
        print("Usage: exec <path>\n");
        return;
    }

    sys_exec(arg);
    print("exec failed: ");
    print(arg);
    print("\n");
}

static void error_handler(char *cmd) {
    print("command not found: ");
    print(cmd);
    print("\n");
    print("type 'help' for command list\n");
}

static void dispatch(char *line) {
    if (strcmp(line, "clear"))
        clear_handler();
    else if (strcmp(line, "help"))
        help_handler();
    else if (strncmp(line, "exec ", 5))
        exec_handler(line + 5);
    else
        error_handler(line);
}

void main() {
    shell_path[0] = '/';
    shell_path[1] = '\0';

    while (1) {
        print(shell_path);
        print("> ");

        int size = sys_read(0, input_buf, 256);

        input_buf[size] = '\0';
        dispatch(input_buf);
    }
}
