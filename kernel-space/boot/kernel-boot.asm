; Tells the assembler to generate 16-bit code.
; This is to be backward compatible
[BITS 16]
; This is the start of all bootsector it was standardized by IBM in 1981
; 0x7C00 = 31,744 bytes
[ORG 0x7C00]

; The sets a consant KERNEL_OFFSET = 0x1000
KERNEL_OFFSET equ 0x1000

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

  ; call swith to protected mode
  call switch_to_pm

  jmp $

load_kernel:
  ; loads KERNEL_OFFSET into bx
  ; specifies where in memory the kernel is 
  mov bx, KERNEL_OFFSET
  ; how many sectors to read
  ; 3 sectors of 512 bytes 
  ; Should be the size of your bios file
  ; This is corresponding to kernel size so we need to make sure we load enough;
  ; memory for the kernel so all the function are there
  mov dh, 50 
  ; The drive to read the sectors from
  mov dl, [BOOT_DRIVE]

  call disk_load
  ret

disk_load:
  ; stores the dx registers to te stack
  push dx
  
  ; The function number for the BIOS interrupt 0x13
  ; you put the function number in ah and the paramters in al
  mov ah, 0x02
  ; the paramter is the number of sectors being 3
  mov al, dh
  ; Cylinder: Stack of platters in the hard drive. Which platter to start at
  mov ch, 0x00
  ; Head: Read or write head
  mov dh, 0x00
  ; Sector: A slice of the track (Usually 512btyes).
  ; The bootloader is in the first slice so we call the second to start 
  mov cl, 0x02

  ; Software interrupt, NOT Hardware interrupt
  int 0x13

  ; If carry flag is set something went wrong go to the disk error mode
  jc disk_error
  
  ; Pop from stack into dx
  pop dx
  ; compate al and dh double check that al and dh are the same we requested
  ; 3 if they aren't the same jump to error again 
  cmp al, dh
  jne disk_error

  ret

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
    ; load the string byte from SI to AL and increment SI by 1
    ; Move SI along the string until you reach the 0 at the end
    ; Compare al to 0 to see if end of string was reached
    ; if you reach the end of the string jump to done
    ; call print character interrupt 
    ; else keep loop 
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
   
  .done:
    ret

switch_to_pm:
  ; Switching into proteced mode 
  ; (CLI) Clear interrupt flag - disable maskable interrupts
  ; If enable during switch it will cause total system failure if interrupt was called
  ; Because in protected mode interrupts use IDT and we haven't set up IDT
  ; System would find garabge and crash due to this.
  cli
  ; Load Global Description Table
  ; GDT is a table taht defines memory segments to know where GDT is before we switch to
  ; protected mode.after switch CPU will use GDT to interpret segment registers.
  lgdt [gdt_descriptor]

  ; CRO - Control Register 0: A special CPU register that controls operation mode
  ; BIT 0 of CRO = Protected Mode BIT 1 of CRO = Real Mode
  mov eax, cr0 
  ; Preserves all other bits in CRO
  ; Could do mov cr0, 1 which would zero all other bis
  or eax, 1
  ; after mov cr0, eax we are in proteced mode
  mov cr0, eax
  
  ; Before transistion we do FAR JUMP
  ; This is a special jump that loads both CS (code segment) and IP (Instruction Pointer)
  ; We are technically in protected mode
  ; But CS still has the old mode value 
  ; This far jump flused the CPU and loads the new code segment
  jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
  ; We reload all the registers with valid protected mode values
  ; Register cs doesn't need to be updated since it was already in the far jump
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; We set the new base pointer to 0x90000
  mov ebp, 0x90000
  ; We set the new stack pointer to 0x90000
  ; This give ourselves more room  (old stack was tiny)
  ; Move it away from where we load our kernel
  ; 0x90000 is conventional for bootloader
  mov esp, ebp

  ; Call kernel offset at 0x1000
  ; Jump to kernel we loaded our kernel at 0x1000, so now we call it
  ; Kernel takes over and the bootloader is done
  call KERNEL_OFFSET
  
  ; if for some reason kernel returns we loop for evers
  jmp $

; GDT - Memory Segment
; In protected mode the CPU enforces memory protection. The GDT defines:
; - Which memory regions exist
; - what you can do with them (read, write, execute)
; - What privilege level is required
; - How large they are 
gdt_start:
  ; First entry must be null on the GDT as of x86 architecture
  ; Why - If a segment register is accidentally zero, the CPU 
  ; will catch it and fault instead of accessing random memory it's safer
  ; dq - define quadword: 8bytes of zeros
  dq 0x0

; This is weird to allow for bakward compatiblity
; The limit is the size of the segment in this case 0xFFFF and 11001111b = 0xFFFFF * 4KB  = 4GB
; The base is the starting address of the segment. Base = Segment starts at memory address 0
; Combined with the limit of 4GB this segment covers memory from 0 to 4gb,  
; Access Byte = 10011010b
; Let's break down each bit:
; Bit 7: P (Present)              = 1 (segment is present in memory)
; Bit 6-5: DPL (Privilege Level)  = 00 (ring 0 = kernel)
; Bit 4: S (Descriptor Type)      = 1 (code or data segment)
; Bit 3: E (Executable)           = 1 (this is a code segment)
; Bit 2: DC (Direction/Conforming)= 0 (code grows up)
; Bit 1: RW (Readable/Writable)   = 1 (code can be read)
; Bit 0: A (Accessed)             = 0 (CPU will set this)

; Present bit = 1:If 0, accessing this segment causes a fault. Used for virtual memory (swapping segments to disk).

; DPL = 00 (Ring 0): Privilege levels:
; - Ring 0: Kernel (highest privilege)
; - Ring 1-2: Rarely used
; - Ring 3: User applications (lowest privilege)
; 
; Your kernel runs in Ring 0. Later, user programs will run in Ring 3.
; **Executable = 1:** This is a code segment. The CPU can fetch instructions from here.
; **Readable = 1:** Code can be read as data. Some systems forbid this (execute-only code) for security.
; **Why readable code?** Sometimes you need to read your own code (debugging, self-modifying code, reading constants embedded in code).
; #### **Flags + Limit = 11001111b**
; Bit 7: G (Granularity)          = 1 (limit is in 4KB units)
; Bit 6: DB (Default operation)   = 1 (32-bit segment)
; Bit 5: L (Long mode)            = 0 (not 64-bit)
; Bit 4: AVL (Available)          = 0 (for OS use)
; Bits 3-0: Limit bits 16-19      = 1111
; Granularity = 1: Each unit of limit represents 4KB, not 1 byte.
gdt_code: 
  dw 0xFFFF ; Limit (bits 0-15)
  dw 0x0    ; Base (bits 0-15)
  db 0x0    ; Base (bits 16-23)
  db 10011010b ; Access byte
  db 11001111b ; Flags + limit (bits 16 -19)
  db 0x0 ; base (bits 24 - 31)

; **Almost identical to code segment!** Only one bit different:
; **Access byte = 10010010b:**
; Bit 3: E (Executable) = 0 (this is a data segment, not code)
; This means the data segment isn't allowed to execute
gdt_data:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 10010010b
  db 11001111b
  db 0x0

;code segment: Execute + Read but not write
;data segment: Read + Write but not execute

; User Code Segment (Ring 3)
gdt_user_code:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 11111010b    ; Only difference: DPL=11 (ring 3) instead of 00
  db 11001111b
  db 0x0

; User Data Segment (Ring 3)
gdt_user_data:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 11110010b    ; Only difference: DPL=11 (ring 3) instead of 00
  db 11001111b
  db 0x0

gdt_tss: 
  dw 0x0067
  dw 0x0
  db 0x0
  db 10001001b    ; Only difference: DPL=11 (ring 3) instead of 00
  db 00000000b
  db 0x0

;GDT end marker
gdt_end:

gdt_descriptor:
  ; Size of GDT
  dw gdt_end - gdt_start - 1
  ; Address of GDT
  dd gdt_start 

; GDT LAYOUT - 8 bytes wide each
; gdt_start: 0x00
; gdt_code: 0x80
; gdt_descriptor 0x10
CODE_SEG equ gdt_code - gdt_start ; = 0x08
DATA_SEG equ gdt_data - gdt_start ; = 0x10

BOOT_DRIVE db 0
msg_loading db "Loading kernel....", 13, 10, 0
msg_disk_error db "Disk read error!", 0

times 510-($-$$) db 0
dw 0xAA55
