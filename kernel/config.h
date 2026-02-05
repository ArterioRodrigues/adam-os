#ifndef CONFIG_H
#define CONFIG_H

// VGA Configuration
#define COLOR 0x0F
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT)
#define VGA_COLOR_SHIFT 8
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_CURSOR_HIGH 0x0E
#define VGA_CURSOR_LOW 0x0F

// Optional: byte mask
#define BYTE_MASK 0xFF
// IDT Configuration
#define IDT_ENTRIES 256

// PIC (Programmable Interrupt Controller) Ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// PIC Commands
#define PIC_ICW1_INIT 0x11
#define PIC_ICW4_8086 0x01

// PIC IRQ Offsets
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

// PIC Cascade
#define PIC1_CASCADE 0x04
#define PIC2_CASCADE 0x02

// PIC EOI (End of Interrupt)
#define PIC_EOI 0x20

// Bit Masks
#define BITMASK_LOW 0xFFFF
#define BITMASK_HIGH 0xFFFF

// Keyboard Ports
#define KEYBOARD_DATA_PORT 0x60

// Keyboard Scancodes
#define SCANCODE_ENTER 0x1C
#define SCANCODE_BACKSPACE 0x0E
#define SCANCODE_RELEASE_MASK 0x80

// GDT Selectors
#define KERNEL_CODE_SEGMENT 0x08

// IDT Flags
#define IDT_FLAG_INTERRUPT_GATE 0x8E

// IRQ Numbers
#define IRQ_KEYBOARD 33

// Keyboard IRQ Mask
#define KEYBOARD_IRQ_MASK 0xFD

#define TAB_SIZE 2
#endif
