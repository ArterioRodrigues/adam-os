#include "../lib/lib.h"
#include "../lib/math.h"
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

static void handle_clear() {
    print("\033[2J\033[H");
    print("AdamOS\n");
}
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

static void handle_ls() {
    int fd = sys_open(shell_path);
    if (fd == -1) {
        print("not a folder\n");
        return;
    }

    char buf[512];
    int size = sys_read(fd, buf, 512);

    fat16_entry_t *entry;
    for (int i = 0; i < size; i += sizeof(fat16_entry_t)) {
        entry = (fat16_entry_t *)(buf + i);

        sys_write(0, entry->name, strfind(entry->name, ' '));

        if (strfind(entry->extension, ' ')) {
            print(".");
            sys_write(0, entry->extension, 3);
        }
        print("\n");
    }

    sys_close(fd);
}

void handle_cd(char *cmd) {
    if (strcmp(cmd, "..")) {
        int size = strlen(shell_path);
        for (int i = size - 2; i > -1; i--) {
            if (shell_path[i] == '/') {
                shell_path[i + 1] = '\0';
                return;
            }
        }
        return;
    }

    int fd = sys_open(cmd);
    if (fd == -1) {
        print("not a folder\n");
        return;
    }

    char buf[512];
    int size = sys_read(fd, buf, 512);

    if (!(buf[0] == '.' && buf[32] == '.')) {
        print("not a folder\n");
        sys_close(fd);
        return;
    }

    strcat(shell_path, cmd);
    strcat(shell_path, "/");

    sys_close(fd);
}

void handle_ps() {
    ps_entry_t buf[10];
    char num[16];
    int count = sys_ps(buf, 10);

    print("\nPID  STATUS   PPID");
    print("\n---  -------  ----\n");

    for (int i = 0; i < count; i++) {
        print(itos(num, buf[i].pid));
        print("    ");

        switch (buf[i].status) {
        case 0:
            print("RUNNING");
            break;
        case 1:
            print("READY  ");
            break;
        case 2:
            print("WAITING");
            break;
        case 3:
            print("ZOMBIE ");
            break;
        default:
            print("UNKNOWN");
            break;
        }

        print("     ");
        print(itos(num, buf[i].parent_pid));
        print("\n");
    }
    print("\n");
}

void handle_fork(char *arg) {
    int child = sys_fork();

    if (child == 0) {
        handle_exec(arg);
        sys_exit();
    } else {
        sys_waitpid(child);
    }
}

void handle_kill(char *arg) { sys_kill(stoi(arg)); }

void handle_create(char *arg) {
    char name[13];
    int i = 0;
    while (arg[i] && arg[i] != ' ') {
        name[i] = arg[i];
        i++;
    }
    name[i] = '\0';
    i++;

    char *content = arg + i;
    int fd = sys_open(name);

    if (fd == -1) {
        sys_create(name, content, strlen(content));
    }

    sys_write(fd, content, strlen(content));
    sys_close(fd);
}

void handle_cat(char *arg) {
    int fd = sys_open(arg);
    char buf[100];

    int size = sys_read(fd, buf, 100);
    sys_write(0, buf, size);

    sys_close(fd);
    print("\n");
}

void handle_bf(char *arg) {
    int child = sys_fork();

    if (child == 0) {
        sys_exec("bf");
        sys_exit();
    } else {
        sys_waitpid(child);
    }
}

static void dispatch(char *line) {
    if (strcmp(line, "clear"))
        handle_clear();
    else if (strcmp(line, "help"))
        handle_help();
    else if (strncmp(line, "exec ", 5))
        handle_exec(line + 5);
    else if (strncmp(line, "ls ", 3))
        handle_ls();
    else if (strncmp(line, "ls", 2))
        handle_ls();
    else if (strncmp(line, "cd ", 2))
        handle_cd(line + 3);
    else if (strncmp(line, "ps ", 2))
        handle_ps();
    else if (strncmp(line, "fork ", 5))
        handle_fork(line + 5);
    else if (strncmp(line, "kill ", 5))
        handle_kill(line + 5);
    else if (strncmp(line, "touch ", 6))
        handle_create(line + 6);
    else if (strncmp(line, "cat ", 4))
        handle_cat(line + 4);
    else if (strncmp(line, "bf ", 3))
        handle_bf(line + 3);
    else
        error_handler(line);
}

void main() {
    shell_path[0] = '/';
    shell_path[1] = '\0';

    print("AdamOS\n");
    while (1) {
        print(shell_path);
        print("> ");

        int size = sys_read(0, input_buf, 256);

        input_buf[size] = '\0';
        if (size > 0)
            dispatch(input_buf);
    }
}
