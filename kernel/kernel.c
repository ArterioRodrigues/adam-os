#include "../pch.h"

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
  init_exception();
  load_idtp();

  init_heap();
  init_ramfs();
  
  int x = 10/0;
  while (1)
    ;
}
