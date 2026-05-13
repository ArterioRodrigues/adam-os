# AdamOS

A 32-bit x86 operating system built from scratch in C and x86 assembly. Named after the first man — because this is my first OS.

AdamOS boots from a custom bootloader, transitions into protected mode, and runs user-space processes with full ring 0 / ring 3 separation. It includes a preemptive round-robin scheduler, virtual memory with per-process page tables, a FAT16 filesystem on an ATA disk, `fork`/`exec` process management, a windowed GUI with a mouse-driven compositor, and a user-space shell that launches graphical programs.

![AdamOS Architecture](architecture.svg)

## Features

**Boot & CPU Setup** — A 16-bit bootloader loads the kernel from disk via BIOS `int 0x13`, switches to 32-bit protected mode with a GDT (6 entries: null, kernel code/data, user code/data, TSS), remaps the PIC, and jumps to the kernel entry point at `0x1000`. CPU exception handlers (0–19) catch faults and dump register state.

**Preemptive Scheduler** — A timer-driven round-robin scheduler (PIT at 100Hz, quantum of 20 ticks) manages processes through a linked list of PCBs. Each process has its own page directory, kernel stack, file descriptor table, and process state (RUNNING, READY, WAITING, ZOMBIE). Context switches happen transparently via `iret`.

**Virtual Memory** — 4KB paging with per-process page directories. The kernel is identity-mapped in the first 1MB; user processes run at virtual addresses `0x40000000` (code), `0x48000000` (heap), and `0x50000000` (stack). `fork` performs a deep copy of the entire page directory. A bitmap frame allocator tracks 32,768 frames across 128MB of physical RAM.

**Syscalls** — User-space programs talk to the kernel via `int 0x80`. The kernel exposes 24 syscalls covering process control (`fork`, `exec`, `exit`, `kill`, `waitpid`, `ps`, `sleep`, `uptime`), file I/O (`open`, `read`, `write`, `close`, `create`, `poll`), and the GUI (`create_window`, `create_rect`, `create_text`, `create_line`, `get_event`, `destroy_window`, `flush`).

**FAT16 Filesystem** — A persistent filesystem on a virtual ATA PIO disk. Supports reading files and directories, creating files, and writing data. The filesystem image is built at compile time with `mkfs.fat` and `mcopy`, then embedded in the OS image at LBA 2048. User programs (shell, bf, tetris, calculator, obj_viewer) are stored as real files on a real FAT16 partition.

**Windowing System & GUI** — A full graphical environment built on VBE (VESA BIOS Extensions). The VGA graphics driver provides pixel, rect, line, char, string, blit, and double-buffered flip primitives. The window manager maintains a linked list of windows with per-window pixel buffers, z-ordering, focus state, and drag state. The compositor renders windows back-to-front with a desktop background and a mouse cursor on top. Windows support title-bar dragging, a close button, and per-window event queues (64-event ring buffers for keypress, mouse move, and click events).

**Drivers** — VGA text mode (80×25 with ANSI color escape parsing), VBE graphics mode (pixel-addressable framebuffer with embedded 5×7 bitmap font), PS/2 keyboard with shift handling, PS/2 mouse (IRQ12, 3-byte packet decode), PIT timer at 100Hz, and an ATA PIO disk driver (polling-based LBA28 sector read/write with cache flush).

**User-Space Shell** — A ring 3 shell with commands: `ls`, `cd`, `cat`, `exec`, `fork`, `kill`, `ps`, `touch`, `bf`, `tetris`, `calculator`, `obj_viewer`, `clear`, and `help`. Child processes are launched via `fork`/`exec` with `waitpid` for synchronization, following the classic Unix pattern.

**User-Space Programs** — All running in ring 3 with isolated address spaces, communicating with the kernel only through syscalls:
- **shell** — interactive command shell
- **bf** — Brainfuck interpreter that loads programs from the filesystem
- **tetris** — windowed Tetris clone with ghost piece, level progression, and diff-based rendering
- **calculator** — windowed calculator
- **obj_viewer** — 3D wireframe renderer that loads `.obj` files from disk, with full 4×4 matrix math, perspective projection, and interactive yaw/pitch/zoom controls (`h`/`l` yaw, `j`/`k` pitch, `z`/`Z` zoom, `t` toggle point/wireframe mode)
- **idle** — idle loop process (PID 1)

## Memory Map

```
Address                 Description
─────────────────────────────────────────────────────────
0x00000   – 0x100000    Kernel (1 MB, identity mapped, ring 0)
0x100000  – 0x400000    Kernel heap (kmalloc / kfree, 3 MB)
0x90000                 Kernel stack top
0xB8000                 VGA text-mode buffer
VBE framebuffer         Set by BIOS via VBE mode info at 0x500
0x40000000              User code   (virtual)
0x48000000              User heap   (virtual)
0x50000000              User stack  (virtual)
```

## Syscall Table

| #  | Name             | Description                                  |
|----|------------------|----------------------------------------------|
| 1  | `exit`           | Terminate current process                    |
| 2  | `fork`           | Duplicate process (returns 0 / child PID)    |
| 3  | `read`           | Read from fd (stdin or file)                 |
| 4  | `write`          | Write to fd (stdout or file)                 |
| 5  | `open`           | Open file or directory, returns fd           |
| 6  | `close`          | Close file descriptor                        |
| 7  | `ps`             | List running processes                       |
| 11 | `exec`           | Replace process image with new binary        |
| 12 | `kill`           | Terminate a process by PID                   |
| 13 | `waitpid`        | Block until child exits                      |
| 14 | `create`         | Create a new file on disk                    |
| 15 | `poll`           | Check if fd has data ready                   |
| 16 | `sleep`          | Sleep for N timer ticks                      |
| 17 | `uptime`         | Return timer tick count since boot           |
| 18 | `create_window`  | Create a GUI window                          |
| 19 | `create_rect`    | Draw a filled rectangle in a window          |
| 20 | `create_text`    | Draw a text string in a window               |
| 21 | `get_event`      | Poll next event from a window's event queue  |
| 22 | `destroy_window` | Destroy a window                             |
| 23 | `flush`          | Trigger compositor redraw                    |
| 24 | `create_line`    | Draw a line segment in a window              |

## Directory Layout

```
/
├── run.sh                          # Build everything and launch QEMU
├── kernel-space/
│   ├── boot/
│   │   ├── kernel-boot.asm         # 16-bit bootloader, GDT, protected mode switch
│   │   └── kernel-entry.asm        # 32-bit entry point, calls kernel_main
│   ├── cpu/
│   │   ├── gdt.c / gdt.asm         # Global Descriptor Table + TSS
│   │   ├── idt.c                   # Interrupt Descriptor Table, PIC, I/O ports
│   │   ├── exceptions.c / .asm     # CPU exception handlers (0–19)
│   │   ├── interrupts.asm          # Timer, keyboard, mouse, syscall ISR wrappers
│   │   └── syscall.c               # int 0x80 handler: fork, exec, read, write, GUI...
│   ├── drivers/
│   │   ├── screen-temp.c           # VGA text-mode driver with ANSI color
│   │   ├── vga-graphics.c / .h     # VBE graphics driver (pixel, rect, line, char, blit, flip)
│   │   ├── keyboard.c              # PS/2 keyboard with shift support
│   │   ├── mouse.c                 # PS/2 mouse driver (IRQ12, 3-byte packet decode)
│   │   ├── timer.c                 # PIT timer at 100Hz
│   │   └── ata-disk.c / .h         # ATA PIO sector read/write (LBA28)
│   ├── kernel/
│   │   ├── kernel.c                # Main entry: init GDT, IDT, scheduler, launch shell
│   │   ├── config.h                # All constants: memory map, GDT selectors, PIC, PIT, ATA...
│   │   ├── types.h                 # uint32_t, registers_t, va_list, PCB types, event types
│   │   ├── scheduler.c             # Round-robin scheduler with quantum preemption
│   │   ├── process-control-block.c # PCB creation, context switch (ASM)
│   │   ├── page-table.c / .asm     # Paging: map_page, create/copy/clear page directories
│   │   ├── frame.c                 # Physical frame allocator (bitmap, 128MB / 32768 frames)
│   │   ├── kmalloc.c               # First-fit heap allocator (0x100000, 3MB)
│   │   ├── stdin.c                 # Stdin buffer with blocking read + wait queue
│   │   ├── window.c / .h           # Window manager: create, composite, drag, focus, z-order
│   │   └── event.c / .h            # Event queue (ring buffer, 64 events/window)
│   ├── lib/
│   │   ├── fat16.c                 # FAT16 filesystem: read, write, find, create
│   │   ├── string.c                # strcmp, strlen, itos, itohs, strtok...
│   │   ├── mem.c                   # memcpy, memset, memcmp
│   │   └── math.c                  # pow, min, max, ceil
│   ├── pch.h                       # Precompiled header — included in all .c files
│   ├── linker.ld                   # Kernel linked at 0x1000
│   └── build-kernel.sh             # Kernel build script
└── user-space/
    ├── lib/
    │   ├── syscalls.asm            # int 0x80 wrappers for all 24 syscalls
    │   ├── lib.h                   # User-space syscall declarations
    │   ├── types.h                 # User-space type definitions
    │   ├── string.h                # String + memory utilities for user programs
    │   └── malloc.c                # User-space heap allocator (sbrk-based)
    ├── shared/
    │   ├── obj.c / obj.h           # .obj file parser (vertices + faces)
    │   ├── render.c / render.h     # Shared 3D math: vec3, mat4, perspective projection
    ├── programs/
    │   ├── shell.c                 # Interactive shell with command dispatch
    │   ├── main.c                  # Init process: exec's the shell
    │   ├── idle.c                  # Idle loop (PID 1)
    │   ├── bf.c                    # Brainfuck interpreter
    │   ├── tetris.c                # Windowed Tetris clone
    │   ├── calculator.c            # Windowed calculator
    │   └── obj_viewer.c            # Interactive 3D .obj wireframe viewer
    └── build-user.sh               # User-space build script
```

## Building & Running

**Prerequisites:** `i686-elf-gcc`, `i686-elf-ld`, `nasm`, `qemu-system-x86_64`, `mtools` (for `mcopy` / `mkfs.fat`)

```bash
# Full build + launch QEMU
./run.sh

# Or build individually:
cd user-space   && bash build-user.sh    # compile user programs
cd kernel-space && bash build-kernel.sh  # compile kernel + create disk image
```

The build process compiles user-space programs into flat binaries, embeds them as linker objects in the kernel, assembles a 10 MB FAT16 disk image with the shell and programs, and concatenates everything into a single `os-image.bin` that QEMU boots from. The FAT16 partition starts at LBA 2048.

## Roadmap

The original four-objective proposal (user-space shell, fork/exec, ATA disk driver, persistent filesystem) is complete. The project is now in **Phase 2: evolving toward a Unix-like system**.

Active work, in order:

1. **Cleanup pass** — fix bugs surfaced by the code review (scheduler quantum comparison, `strncmp` early termination, shell `handle_create` writing to fd `-1`, ANSI parser bounds check, missing NULL guards in GUI syscalls).
2. **ELF loader** — replace flat binaries with a minimal ELF32 loader so `exec` can parse program headers and load `PT_LOAD` segments at their declared virtual addresses.
3. **Pipes + `dup2`** — give the shell `cmd1 | cmd2` and `>` / `<` redirection.
4. **VFS layer** — unify `open`/`read`/`write`/`close` behind a mount-table abstraction; add `devfs` (`/dev/null`, `/dev/zero`, `/dev/console`) and `procfs` (`/proc`).
5. **Signals** — SIGKILL, SIGTERM, SIGINT, SIGSEGV, SIGCHLD; per-process signal mask; Ctrl+C → SIGINT to foreground process group.
6. **TTY subsystem** — line discipline (backspace, Ctrl+C, Ctrl+D, Ctrl+Z), TTY as `/dev/tty0`, foreground/background job control.

Stretch goals: copy-on-write fork, `mmap`/shared memory, Ext2 filesystem, standard userland utilities (echo, grep, cp, mv, rm, mkdir), and a minimal TCP/IP stack.
