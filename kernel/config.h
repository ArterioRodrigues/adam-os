#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// MEMORY LAYOUT
// ============================================================
// Physical address space map:
//
//  0x00000 - 0x100000  Kernel (1MB) — identity mapped, supervisor only
//  0x10000 - 0x90000   Heap — sits inside kernel region
//  0x90000             Kernel stack top
//  0x40000000          User function page (virtual)
//  0x50000000          User stack page (virtual)
//
#define KERNEL_START        0x00000     // Physical start of kernel
#define KERNEL_END          0x100000    // Physical end of kernel (1MB)
#define KERNEL_STACK_ADDRESS 0x90000   // Top of kernel stack (grows downward)

#define USER_FUNC_VADDR     0x40000000  // Virtual address user function is mapped to
#define USER_STACK_VADDR    0x50000000  // Virtual address user stack is mapped to

// ============================================================
// GDT SELECTORS
// ============================================================
// The GDT (Global Descriptor Table) holds segment descriptors.
// Each entry is 8 bytes wide, so the selector = entry index * 8.
//
// Selector format (16 bits):
//   Bits 15-3 : Index into GDT
//   Bit  2    : 0 = GDT, 1 = LDT
//   Bits 1-0  : RPL — Requested Privilege Level (0=kernel, 3=user)
//
// GDT layout:
//   Entry 0: 0x00 — Null descriptor       (required by CPU)
//   Entry 1: 0x08 — Kernel code  (ring 0, execute/read)
//   Entry 2: 0x10 — Kernel data  (ring 0, read/write)
//   Entry 3: 0x18 — User code    (ring 3, execute/read)
//   Entry 4: 0x20 — User data    (ring 3, read/write)
//   Entry 5: 0x28 — TSS          (Task State Segment)
//
#define GDT_ENTRY_SIZE      8           // Each GDT entry is 8 bytes

#define GDT_KERNEL_CODE     1           // Entry index
#define GDT_KERNEL_DATA     2
#define GDT_USER_CODE       3
#define GDT_USER_DATA       4
#define GDT_TSS             5

// Selectors = index * 8 (no RPL bits set — RPL 0 for kernel)
#define KERNEL_CODE_SEGMENT (GDT_KERNEL_CODE * GDT_ENTRY_SIZE)  // 0x08
#define KERNEL_DATA_SEGMENT (GDT_KERNEL_DATA * GDT_ENTRY_SIZE)  // 0x10

// User selectors have RPL=3 set in the bottom 2 bits
// Used in iret frame to jump to ring 3
#define USER_CODE_SEGMENT   ((GDT_USER_CODE * GDT_ENTRY_SIZE) | 3)  // 0x1B
#define USER_DATA_SEGMENT   ((GDT_USER_DATA * GDT_ENTRY_SIZE) | 3)  // 0x23

// TSS selector — loaded into TR register by ltr instruction
#define TSS_SEGMENT         (GDT_TSS * GDT_ENTRY_SIZE)              // 0x28

// ============================================================
// PAGE TABLE FLAGS
// ============================================================
// Each page directory/table entry has flag bits in bits 0-11.
// The physical address occupies bits 12-31 (4KB aligned).
//
// Bit 0 (0x1): Present       — must be set or any access causes page fault
// Bit 1 (0x2): Read/Write    — if clear, page is read-only
// Bit 2 (0x4): User          — if clear, only ring 0 can access (ring 3 = page fault)
//
// Common combinations:
//   0x3 = Present | R/W              — kernel page  (ring 3 cannot access)
//   0x7 = Present | R/W | User       — user page    (ring 3 can read and write)
//
#define PAGE_FLAG_KERNEL    0x3     // Present + R/W, supervisor only
#define PAGE_FLAG_USER      0x7     // Present + R/W + User accessible

// ============================================================
// PAGE FRAME ALLOCATOR
// ============================================================
#define PAGE_SIZE           4096
#define MAX_MEMORY          (128 * 1024 * 1024)     // 128MB physical RAM
#define NUM_FRAMES          (MAX_MEMORY / PAGE_SIZE) // 32768 frames

// ============================================================
// HEAP
// ============================================================
#define HEAP_START          0x10000
#define HEAP_SIZE           0x80000
#define HEAP_END            (HEAP_START + HEAP_SIZE)

// ============================================================
// VGA TEXT MODE
// ============================================================
// VGA text mode buffer lives at 0xB8000.
// Each character cell is 2 bytes: [color][ascii]
// Screen is 80 columns x 25 rows = 2000 cells.
//
#define VGA_ADDRESS         0xB8000
#define VGA_WIDTH           80
#define VGA_HEIGHT          25
#define VGA_SIZE            (VGA_WIDTH * VGA_HEIGHT)
#define VGA_COLOR_SHIFT     8           // Color byte is the high byte of each cell

// VGA cursor control — write index to CTRL, then value to DATA
#define VGA_CTRL_REGISTER   0x3D4
#define VGA_DATA_REGISTER   0x3D5
#define VGA_CURSOR_HIGH     0x0E       // High byte of cursor position register index
#define VGA_CURSOR_LOW      0x0F       // Low byte of cursor position register index

// VGA colors (foreground)
#define WHITE               0x0F
#define GREEN               0x02
#define BLUE                0x01
#define BRIGHT_GREEN        0x0A
#define BRIGHT_BLUE         0x0B

// ============================================================
// PIT — PROGRAMMABLE INTERVAL TIMER (Intel 8253/8254)
// ============================================================
// The PIT generates hardware IRQ0 at a configurable frequency.
// Base clock is 1.193182 MHz. Divisor = base / desired_freq.
//
// Port 0x43: Command register
// Port 0x40: Channel 0 data (IRQ0, connected to PIC IRQ0)
//
// Command byte 0x36:
//   Bits 7-6: 00  — Channel 0
//   Bits 5-4: 11  — Access mode: lobyte then hibyte
//   Bits 3-1: 011 — Mode 3: square wave
//   Bit  0:   0   — Binary (not BCD)
//
#define PIT_CHANNEL0        0x40
#define PIT_COMMAND         0x43
#define PIT_BASE_FREQ       1193182     // Hz — fixed crystal frequency
#define TIMER_FREQ          100         // Hz — interrupts per second we want
#define PIT_MODE2           0x36        // Channel 0, lobyte/hibyte, mode 3, binary

// ============================================================
// PIC — PROGRAMMABLE INTERRUPT CONTROLLER (Intel 8259)
// ============================================================
// Two cascaded PICs (master + slave) manage 15 hardware IRQ lines.
// After remapping, IRQs land at IDT entries 0x20-0x2F to avoid
// conflicting with CPU exception vectors (0x00-0x1F).
//
// Master PIC (PIC1): handles IRQ0-IRQ7  → remapped to IDT 0x20-0x27
// Slave  PIC (PIC2): handles IRQ8-IRQ15 → remapped to IDT 0x28-0x2F
//
#define PIC1_COMMAND        0x20        // Master PIC command port
#define PIC1_DATA           0x21        // Master PIC data/mask port
#define PIC2_COMMAND        0xA0        // Slave PIC command port
#define PIC2_DATA           0xA1        // Slave PIC data/mask port

#define PIC_ICW1_INIT       0x11        // Begin init sequence, cascade mode, ICW4 needed
#define PIC_ICW4_8086       0x01        // 8086 mode (not MCS-80)
#define PIC_EOI             0x20        // End Of Interrupt — must be sent after each IRQ

#define PIC1_OFFSET         0x20        // Master IRQ0 → IDT entry 0x20 (32)
#define PIC2_OFFSET         0x28        // Slave  IRQ8 → IDT entry 0x28 (40)

#define PIC1_CASCADE        0x04        // Master: slave is on IRQ2 (bit 2)
#define PIC2_CASCADE        0x02        // Slave:  cascade identity = IRQ2

// IRQ mask register: each bit masks one IRQ line (0=enabled, 1=masked)
// 0xFC = 11111100 — only IRQ0 (timer, bit 0) and IRQ1 (keyboard, bit 1) enabled
#define TIMER_KEYBOARD_IRQ_MASK 0xFC
#define KEYBOARD_IRQ_MASK   0xFD        // 11111101 — only keyboard enabled

// ============================================================
// IDT — INTERRUPT DESCRIPTOR TABLE
// ============================================================
// 256 entries covering:
//   0x00-0x1F: CPU exceptions (divide by zero, page fault, etc.)
//   0x20-0x2F: Hardware IRQs (remapped from PIC)
//   0x80:      Software syscall (int 0x80)
//
#define IDT_ENTRIES         256

// Gate type flags (byte in IDT entry):
//   0x8E = 1 000 01110
//          P DPL  type
//   P=1    : present
//   DPL=00 : callable from ring 0 only (kernel gates)
//   type=E : 32-bit interrupt gate (disables interrupts on entry)
//
//   0xEE = 1 110 01110
//   DPL=11 : callable from ring 3 (needed for int $0x80 syscall gate)
//
#define IDT_FLAG_INTERRUPT_GATE 0x8E   // Ring 0 interrupt gate
#define IDT_FLAG_SYSCALL_GATE   0xEE   // Ring 3 callable interrupt gate

// IDT vector numbers
#define IRQ_TIMER           32          // PIC1 IRQ0 → 0x20
#define IRQ_KEYBOARD        33          // PIC1 IRQ1 → 0x21
#define IRQ_SYSCALL         0x80        // Software interrupt for syscalls

// ============================================================
// KEYBOARD
// ============================================================
#define KEYBOARD_DATA_PORT  0x60
#define SCANCODE_ENTER      0x1C
#define SCANCODE_BACKSPACE  0x0E
#define SCANCODE_RELEASE_MASK 0x80     // Bit 7 set = key release event

// ============================================================
// MISC
// ============================================================
#define BYTE_MASK           0xFF        // Mask to isolate lowest byte
#define BITMASK_LOW         0xFFFF      // Mask for low 16 bits
#define BITMASK_HIGH        0xFFFF      // Mask for high 16 bits (after shift)
#define TAB_SIZE            4

#endif
