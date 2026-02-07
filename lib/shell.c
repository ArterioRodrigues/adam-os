#include "../pch.h"
#include "string.h"

char *path = '/';

void shell_handler_main() {
  int x = vga_index % VGA_WIDTH;
  int y = vga_index / VGA_WIDTH;
  update_cursor(x, y);

  if (keyboard_buffer_index == 0) {
    print(">> ");
    x = vga_index % VGA_WIDTH;
    y = vga_index / VGA_WIDTH;
    update_cursor(x, y);
    return;
  }

  char c = keyboard_buffer[keyboard_buffer_index - 1];
  if (c != '\0' && keyboard_pressed) {
    print_char(c);
    return;
  }

  if (c != '\0') {
    return;
  }

  if (strcmp(keyboard_buffer, "clear")) {
    clear_screen();
  } else if (strcmp(keyboard_buffer, "about")) {
    print("\n\tname    - Adam OS");
    print("\n\tversion - 0.0.1\n");
  } else if (strcmp(keyboard_buffer, "help")) {
    print("\n\tabout  - Info about OS version");
    print("\n\tclear  - Clear terminal screen");
    print("\n\techo   - Print message to screen");
    print("\n\tuptime - Show system uptime\n");
    print("\n\tls - List files in system\n");
  } else if (strncmp(keyboard_buffer, "echo ", 5)) {
    print("\n\t");
    print(keyboard_buffer + 5);
    print("\n");
  } else if (strncmp(keyboard_buffer, "sleep ", 6)) {
    uint32_t seconds = string_to_int(keyboard_buffer + 6);
    sleep(seconds);

    char result[3];
    int_to_string(result, seconds);
    print("\n\twaits ");
    print(result);
    print(" seconds\n");

  } else if (strcmp(keyboard_buffer, "uptime")) {
    char buffer[20];
    int_to_string(buffer, get_uptime_seconds());
    print("\nuptime: ");
    print(buffer);
    print(" seconds\n");
  } else if (strcmp(keyboard_buffer, "ls")) {
    ramfs_ls("/");
    print("\n");
  } else {
    print("\n\tCommand not found!");
    print("\n\tType 'help' for command list\n");
  }

  keyboard_buffer_index = 0;
}
