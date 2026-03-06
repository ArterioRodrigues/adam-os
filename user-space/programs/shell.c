#include "../lib/lib.h"
#include "../lib/string.h"

typedef struct fat16_entry {
    uint8_t name[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved[14];
    uint16_t start_cluster;
    uint32_t file_size;
} __attribute__((packed)) fat16_entry_t;

char shell_path[50];
char input_buf[256];

static void handle_help() {
    print("\texec   - Run a program\n");
    print("\tls - List files in folder\n");
    print("\thelp   - Show this message\n");
    print("\tclear  - Clear terminal screen\n");
}

static void handle_clear() { print("\033[2J\033[H"); }

static void handle_exec(char *arg) {
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

static void handle_ls(char *cmd) {
    int fd = sys_open(cmd);
    if (fd == -1) {
        print("not a folder");
        return;
    }

    char buf[512];
    int size = sys_read(fd, buf, 512);

    if ((buf[0] != '.' || buf[32] != '.' ) && !strcmp(cmd, "/"))
        return;

    for (int i = 0; i < size; i += 32) {
        sys_write(0, buf + i, 8);
        print("\n");
    }

    sys_close(fd);
}

static void dispatch(char *line) {
    if (strcmp(line, "clear"))
        handle_clear();
    else if (strcmp(line, "help"))
        handle_help();
    else if (strncmp(line, "exec ", 5))
        handle_exec(line + 5);
    else if (strncmp(line, "ls ", 3))
        handle_ls(line + 3);
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
        if (size > 0)
            dispatch(input_buf);
    }
}
