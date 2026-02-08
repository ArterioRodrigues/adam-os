#include "../pch.h"
#include "config.h"
#include "kmalloc.h"

void kernel_main() {
  clear_screen();

  print("================\n");
  print("=== ADAM OS! ===\n");
  print("================\n");

  init_shell();
  print_color(shell_path, BRIGHT_GREEN);

  init_idtp();

  remap_pic();

  init_keyboard();
  init_timer();

  load_idtp();

  init_heap();
  init_ramfs();

  while (1)
    ;
}

