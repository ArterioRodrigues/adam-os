#include "../lib/lib.h"
#include "../lib/malloc.h"

#define MAX_INPUT 256
#define MAX_PS 16
#define READ_BUF 512

static char shell_path[64];
static char input_buf[MAX_INPUT];

static void print_error(const char *msg) {
    print("error: ");
    print(msg);
    print("\n");
}

static void handle_help() {
    print("  ls             List files in current directory\n");
    print("  cd <dir>       Change directory\n");
    print("  cat <file>     Print file contents\n");
    print("  exec <file>    Run program (replaces shell)\n");
    print("  fork <file>    Run program as child process\n");
    print("  kill <pid>     Kill a process\n");
    print("  ps             Show running processes\n");
    print("  touch <f> <d>  Create file with content\n");
    print("  bf <file>      Run Brainfuck program\n");
    print("  clear          Clear terminal screen\n");
    print("  help           Show this message\n");
}

static void handle_clear() {
    print("\033[2J\033[H");
    print("SHELL\n");
}

static void handle_exec(char *file_name, char *arg) {
    if (file_name[0] == '\0') {
        print("Usage: exec <path>\n");
        return;
    }
    sys_exec(file_name, arg);
    print("exec failed: ");
    print(file_name);
    print("\n");
}

static void handle_fork(char *file_name, char *arg) {
    int child = sys_fork();
    if (child == 0)
        handle_exec(file_name, arg);
    else
        sys_waitpid(child);
}

static void handle_ls() {
    int fd = sys_open(shell_path);
    if (fd == -1) {
        print_error("cannot open directory");
        return;
    }

    char buf[READ_BUF];
    int size = sys_read(fd, buf, READ_BUF);

    for (int i = 0; i < size; i += sizeof(fat16_entry_t)) {
        fat16_entry_t *entry = (fat16_entry_t *)(buf + i);

        if (entry->name[0] == 0x00 || entry->name[0] == 0xE5)
            continue;

        int name_len = strfind((char *)entry->name, ' ');
        if (name_len < 0)
            name_len = 8;
        sys_write(1, (char *)entry->name, name_len);

        int ext_len = strfind((char *)entry->extension, ' ');
        if (ext_len > 0) {
            print(".");
            sys_write(1, (char *)entry->extension, ext_len);
        }
        print("\n");
    }

    sys_close(fd);
}

static void handle_cd(char *cmd) {
    if (strcmp(cmd, "..")) {
        int size = strlen(shell_path);
        for (int i = size - 2; i >= 0; i--) {
            if (shell_path[i] == '/') {
                shell_path[i + 1] = '\0';
                return;
            }
        }
        return;
    }

    int fd = sys_open(cmd);
    if (fd == -1) {
        print_error("not a directory");
        return;
    }

    char buf[READ_BUF];
    sys_read(fd, buf, READ_BUF);

    if (!(buf[0] == '.' && buf[32] == '.')) {
        print_error("not a directory");
        sys_close(fd);
        return;
    }

    strcat(shell_path, cmd);
    strcat(shell_path, "/");
    sys_close(fd);
}

static void handle_ps() {
    ps_entry_t buf[MAX_PS];
    char num[16];
    int count = sys_ps(buf, MAX_PS);

    static const char *status_names[] = {"RUNNING", "READY  ", "WAITING", "ZOMBIE "};

    print("\nPID  STATUS   PPID\n");
    print("---  -------  ----\n");

    for (int i = 0; i < count; i++) {
        print(itos(num, buf[i].pid));
        print("    ");

        if (buf[i].status <= ZOMBIE)
            print(status_names[buf[i].status]);
        else
            print("UNKNOWN");

        print("  ");
        print(itos(num, buf[i].parent_pid));
        print("\n");
    }
    print("\n");
}

static void handle_kill(char *arg) {
    if (arg[0] == '\0') {
        print("Usage: kill <pid>\n");
        return;
    }
    sys_kill(stoi(arg));
}

static void handle_cat(char *arg) {
    if (arg[0] == '\0') {
        print("Usage: cat <file>\n");
        return;
    }

    int fd = sys_open(arg);
    if (fd == -1) {
        print_error("file not found");
        return;
    }

    char buf[READ_BUF];
    int size;
    while ((size = sys_read(fd, buf, READ_BUF)) > 0)
        sys_write(1, buf, size);

    sys_close(fd);
    print("\n");
}

static void handle_create(char *arg) {
    char name[13];
    int i = 0;
    while (arg[i] && arg[i] != ' ' && i < 12) {
        name[i] = arg[i];
        i++;
    }
    name[i] = '\0';

    if (name[0] == '\0') {
        print("Usage: touch <name> <content>\n");
        return;
    }

    char *content = (arg[i] == ' ') ? arg + i + 1 : "";
    int content_len = strlen(content);

    int fd = sys_open(name);
    if (fd == -1) {
        sys_create(name, content, content_len);
        return;
    }

    sys_write(fd, content, content_len);
    sys_close(fd);
}

static void handle_dump() {

}

static void dispatch(char *line) {
    if (strcmp(line, "clear"))
        handle_clear();
    else if (strcmp(line, "help"))
        handle_help();
    else if (strcmp(line, "ls"))
        handle_ls();
    else if (strcmp(line, "ps"))
        handle_ps();
    else if (strncmp(line, "exec ", 5))
        handle_exec(line + 5, "");
    else if (strncmp(line, "cd ", 3))
        handle_cd(line + 3);
    else if (strncmp(line, "fork ", 5))
        handle_fork(line + 5, "");
    else if (strncmp(line, "kill ", 5))
        handle_kill(line + 5);
    else if (strncmp(line, "touch ", 6))
        handle_create(line + 6);
    else if (strncmp(line, "cat ", 4))
        handle_cat(line + 4);
    else if (strncmp(line, "bf ", 3))
        handle_fork("bf", line + 3);
    else if (strcmp(line, "dump"))
        handle_dump();
    else {
        print("command not found: ");
        print(line);
        print("\ntype 'help' for command list\n");
    }
}

int main() {
    shell_path[0] = '/';
    shell_path[1] = '\0';

    print("SHELL\n");

    while (1) {
        print(shell_path);
        print("> ");

        int size = sys_read(0, input_buf, MAX_INPUT - 1);
        input_buf[size] = '\0';

        if (size > 0)
            dispatch(input_buf);
    }

    return 0;
}
