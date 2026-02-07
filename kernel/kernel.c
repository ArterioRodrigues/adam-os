#include "../pch.h"
#include "kmalloc.h"

void kernel_main() {
  clear_screen();

  print("================\n");
  print("=== ADAM OS! ===\n");
  print("================\n");
  print(">> ");

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
