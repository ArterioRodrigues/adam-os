#include "../pch.h"
char shell_path[50];

void init_shell() { shell_path[0] = '/'; }

void clear_handler() { clear_screen(); }

void help_handler() {
  print("\tabout  - Info about OS version\n");
  print("\tclear  - Clear terminal screen\n");
  print("\techo   - Print message to screen\n");
  print("\tuptime - Show system uptime\n");
  print("\tls     - List files in system\n");
  print("\ttouch  - Creat a new file");
}
void uptime_handler() {
  char buffer[20];
  itos(buffer, get_uptime_seconds());
  print("uptime: ");
  print(buffer);
  print(" seconds");
}
void ls_handler(char *path) {
  ramfs_node_t *current = ramfs_find(path);
  ramfs_node_t *child = current->children;

  if (current->parent != NULL) {
    print("..\n");
  }
  while (child != NULL) {
    if (child->type == RAMFS_FILE) {
      print("\t");
      print(itos("", child->size));
      print("\t");
      print(child->name);
    } else {
      print_color(child->name, BRIGHT_BLUE);
      print_color("/", BRIGHT_BLUE);
    }

    print("\n");
    child = child->next;
  }
}
void touch_handler(char *path, char *name) { ramfs_make_file(path, name); }
void cd_handler(char *path, char *name) {
  char result[50];
  strcpy(result, path);
  strcat(result, name);

  if (ramfs_find(result)->type == RAMFS_DIR) {
    if (!strcmp(name, "..")) {
      strcpy(path, result);
      strcat(path, "/");
    }
  } else {
    print("Folder not found!");
  }
}
void mkdir_handler(char *path, char *name) { ramfs_make_folder(path, name); }
void cat_handler(char *path) {
  ramfs_node_t *file = ramfs_find(path);

  if (!file) {
    return;
  }

  if (file->type != RAMFS_FILE) {
    return;
  }

  if (!file->data || file->size == 0) {
    return;
  }
  print((char *)file->data);
}
void write_handler(char *buffer) {
  char *space = buffer;
  int i = 0;
  while (space[i] != ' ' && space[i] != '\0')
    i++;

  if (space[i] == '\0') {
    print("Usage: write filename content");
    return;
  }

  space[i] = '\0';
  char *filename = buffer;
  char *content = space + i + 1;

  if (!ramfs_write(filename, content)) {
    print("No files found!");
  }
}
void error_handler() {
  print("\tCommand not found!\n");
  print("\tType 'help' for command list");
}
void shell_update_cusor() {
  int x = vga_index % VGA_WIDTH;
  int y = vga_index / VGA_WIDTH;
  update_cursor(x, y);
}
void shell_handler_main() {
  if (keyboard_buffer_index == 0)
    return;

  char c = keyboard_buffer[keyboard_buffer_index - 1];

  if (c != '\0') {

    if (c == '\b') {
      keyboard_buffer[keyboard_buffer_index - 1] = '\0';
      keyboard_buffer_index -= 2;

      if (keyboard_buffer_index < 0) {
        keyboard_buffer_index = 0;
        keyboard_buffer[1] = '\0';
      }
    }

    int x = vga_index % VGA_WIDTH;
    if (keyboard_pressed && !(c == '\b' && x <= strlen(shell_path))) {
      print_char(c);
      shell_update_cusor();
    }
    return;
  }

  print_char('\n');
  if (strcmp(keyboard_buffer, "clear"))
    clear_handler();
  else if (strcmp(keyboard_buffer, "help"))
    help_handler();
  else if (strcmp(keyboard_buffer, "uptime"))
    uptime_handler();
  else if (strcmp(keyboard_buffer, "ls"))
    ls_handler(shell_path);
  else if (strncmp(keyboard_buffer, "touch ", 6))
    touch_handler(shell_path, keyboard_buffer + 6);
  else if (strncmp(keyboard_buffer, "cd ", 3))
    cd_handler(shell_path, keyboard_buffer + 3);
  else if (strncmp(keyboard_buffer, "mkdir ", 6))
    mkdir_handler(shell_path, keyboard_buffer + 6);
  else if (strncmp(keyboard_buffer, "cat ", 4))
    cat_handler(keyboard_buffer + 4);
  else if (strncmp(keyboard_buffer, "write ", 6))
    write_handler(keyboard_buffer + 6);
  else
    error_handler();
  print_char('\n');

  keyboard_buffer_index = 0;
  print_color(shell_path, BRIGHT_GREEN);
  shell_update_cusor();
}
