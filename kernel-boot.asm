[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000

start:
  mov [BOOT_DRIVE], dl

  mov bp, 0x9000
  mov sp, bp

  mov si, msg_loading
  call print_rm

  call load_kernel

  call switch_to_pm

  jmp $

load_kernel:
  mov bx, KERNEL_OFFSET
  mov dh, 3 
  mov dl, [BOOT_DRIVE]
  call disk_load
  ret

disk_load:
  push dx

  mov ah, 0x02
  mov al, dh
  mov ch, 0x00
  mov dh, 0x00
  mov cl, 0x02

  int 0x13
  jc disk_error

  pop dx
  cmp al, dh
  jne disk_error
  ret

disk_error:
  mov si, msg_disk_error
  call print_rm
  jmp $

print_rm:
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
  cli
  lgdt [gdt_descriptor]

  mov eax, cr0 
  or eax, 1
  mov cr0, eax
  
  jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  mov ebp, 0x90000
  mov esp, ebp

  call KERNEL_OFFSET

  jmp $

gdt_start:
  dq 0x0

gdt_code: 
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 10011010b
  db 11001111b
  db 0x0

gdt_data:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 10010010b
  db 11001111b
  db 0x0

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start 

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

BOOT_DRIVE db 0
msg_loading db "Loading kernel....", 13, 10, 0
msg_disk_error db "Disk read error!", 0

times 510-($-$$) db 0
dw 0xAA55
