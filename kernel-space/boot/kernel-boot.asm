; Tells the assembler to generate 16-bit code.
; This is to be backward compatible
[BITS 16]
; This is the start of all bootsector it was standardized by IBM in 1981
; 0x7C00 = 31,744 bytes
[ORG 0x7C00]

; The sets a consant KERNEL_OFFSET = 0x1000
KERNEL_OFFSET equ 0x1000
KERNEL_SECTORS equ 128

start:
  ; When BIOS jumps to 0x7C00 it sets the dl register to be the booted drive
  ; 0x00 = First Floppy Drive
  ; 0x80 = First Hard Drive
  ; 0x80 = Second Hard Drive
  mov [BOOT_DRIVE], dl
  
  ; move 0x9000 into bp as the base pointer
  ; base pointer of the stack
  mov bp, 0x9000

  ; move the (sp) stack pointer to point to the base pointer
  ; stack pointer = 0x9000
  mov sp, bp

  ; (SI) Source Index we load the message into it
  ; We can print in the bios by using BIOS function
  ; after this we will have to make our own function
  ; to print to video memory
  mov si, msg_loading

  ; call the print function
  call print_rm
  
  ; call load kernel
  call load_kernel

  ; switch to VGA Mode 13h (320x200, 256 colors)
  ; Must be done in real mode before protected mode switch
  call load_vga

  ; call switch to protected mode
  call switch_to_pm

  jmp $

load_vga:
  xor ax, ax
  mov es, ax
  mov di, 0x500 
  mov ah, 0x4F
  mov al, 0x01 
  mov cx, 0x103
  int 0x10

  cmp ax, 0x004F
  jne .vga_error

  mov ah, 0x4F
  mov al, 0x02
  mov bh, 0x41
  mov bl, 0x03
  int 0x10

  cmp ax, 0x004F
  jne .vga_error
  ret

.vga_error:
  jmp $
; Load kernel using LBA extended read (int 0x13 AH=0x42)
; Loads to temporary address 0x10000 (segment 0x1000:offset 0x0000)
; to avoid overwriting bootloader at 0x7C00.
; Kernel will be relocated to 0x1000 in protected mode.
load_kernel:
  mov si, dap
  mov ah, 0x42
  mov dl, [BOOT_DRIVE]
  int 0x13
  jc disk_error
  ret

; Disk Address Packet for LBA extended read
dap:
  db 0x10                ; DAP size (16 bytes)
  db 0x00                ; reserved
  dw KERNEL_SECTORS      ; number of sectors to read
  dw 0x0000              ; destination offset
  dw 0x1000              ; destination segment (0x1000 << 4 = physical 0x10000)
  dd 1                   ; LBA start (sector 1, right after bootloader)
  dd 0                   ; upper 32 bits of LBA

disk_error:
  ; move the message into si print message
  mov si, msg_disk_error
  call print_rm
  ; loop infinitely
  jmp $

print_rm:
  ; this is the BIOS talking to screen function 0x0E 
  ; Prints character in TTY mode
  mov ah, 0x0E

  .loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
   
  .done:
    ret

switch_to_pm:
  ; Disable interrupts — IDT not set up yet
  cli

  ; Load Global Descriptor Table
  lgdt [gdt_descriptor]

  ; Set protected mode bit in CR0
  mov eax, cr0 
  or eax, 1
  mov cr0, eax
  
  ; Far jump to flush pipeline and load new code segment
  jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
  ; Reload all segment registers with protected mode selectors
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Set up protected mode stack
  mov ebp, 0x90000
  mov esp, ebp

  ; === Step 1: Relocate GDT to 0x80000 ===
  ; The kernel copy will overwrite 0x7C00 where the GDT lives.
  ; The CPU references the GDT continuously, so we must move it first.
  mov esi, gdt_start
  mov edi, 0x80000
  mov ecx, (gdt_end - gdt_start + 3) / 4
  cld
  rep movsd

  ; Point GDTR to the new GDT location
  lgdt [new_gdt_ptr]

  ; Far jump to flush pipeline with new GDT
  jmp CODE_SEG:.gdt_ok
.gdt_ok:

  ; === Step 2: Copy trampoline to 0x600 ===
  ; The trampoline is a small piece of code that copies the kernel
  ; from 0x10000 → 0x1000, then calls the kernel entry point.
  ; We put it at 0x600 (safely below 0x1000) so it won't be
  ; overwritten when the kernel is copied into place.
  mov esi, .trampoline
  mov edi, 0x600
  mov ecx, (.trampoline_end - .trampoline + 3) / 4
  cld
  rep movsd

  ; === Step 3: Jump to trampoline ===
  ; After this jump, everything at 0x7C00 can safely be overwritten
  jmp 0x600

; --- Trampoline code (will be copied to 0x600) ---
; Uses only absolute addresses so it runs correctly at any location.
.trampoline:
  ; Copy kernel from temp 0x10000 → linked address 0x1000
  mov esi, 0x10000
  mov edi, KERNEL_OFFSET
  mov ecx, (KERNEL_SECTORS * 512) / 4
  cld
  rep movsd

  ; Call kernel entry point
  mov eax, KERNEL_OFFSET
  call eax

  ; If kernel returns, halt forever
  jmp $
.trampoline_end:

; --- New GDT pointer (used after relocation to 0x80000) ---
new_gdt_ptr:
  dw gdt_end - gdt_start - 1
  dd 0x80000

; ============================================================
; GDT - GLOBAL DESCRIPTOR TABLE
; ============================================================
; In protected mode the CPU enforces memory protection. The GDT defines:
; - Which memory regions exist
; - What you can do with them (read, write, execute)
; - What privilege level is required
; - How large they are 
gdt_start:
  ; First entry must be null (x86 requirement)
  dq 0x0

; Kernel Code Segment (Ring 0)
; Base = 0x00000000, Limit = 4GB
; Access: Present, DPL=0, Code, Readable
gdt_code: 
  dw 0xFFFF       ; Limit (bits 0-15)
  dw 0x0          ; Base (bits 0-15)
  db 0x0          ; Base (bits 16-23)
  db 10011010b    ; Access byte: P=1, DPL=00, S=1, E=1, DC=0, RW=1, A=0
  db 11001111b    ; Flags: G=1, DB=1, L=0, AVL=0 + Limit bits 16-19
  db 0x0          ; Base (bits 24-31)

; Kernel Data Segment (Ring 0)
; Same as code but not executable
gdt_data:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 10010010b    ; Access byte: E=0 (data segment)
  db 11001111b
  db 0x0

; User Code Segment (Ring 3)
gdt_user_code:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 11111010b    ; Access byte: DPL=11 (ring 3), code
  db 11001111b
  db 0x0

; User Data Segment (Ring 3)
gdt_user_data:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 11110010b    ; Access byte: DPL=11 (ring 3), data
  db 11001111b
  db 0x0

; TSS Descriptor (filled in by kernel at runtime)
gdt_tss: 
  dw 0x0067
  dw 0x0
  db 0x0
  db 10001001b    ; Access byte: Present, DPL=0, TSS type
  db 00000000b
  db 0x0

gdt_end:

; GDT descriptor (pointer structure for lgdt)
gdt_descriptor:
  dw gdt_end - gdt_start - 1   ; Size of GDT
  dd gdt_start                  ; Address of GDT

; Segment selector offsets
CODE_SEG equ gdt_code - gdt_start      ; = 0x08
DATA_SEG equ gdt_data - gdt_start      ; = 0x10

; Data
BOOT_DRIVE db 0
msg_loading db "Loading kernel....", 13, 10, 0
msg_disk_error db "Disk read error!", 0

; Pad to 510 bytes + boot signature
times 510-($-$$) db 0
dw 0xAA55
