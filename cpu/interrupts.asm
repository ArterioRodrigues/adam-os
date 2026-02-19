[BITS 32]

global timer_handler
global keyboard_handler
global syscall_handler
global idt_load 

extern timer_handler_main 
extern shell_handler_main
extern syscall_handler_main
extern keyboard_handler_main


idt_load:
  mov eax, [esp + 4]

  lidt [eax]
  ret

timer_handler:
  pushad

  call timer_handler_main

  popad
  iret

keyboard_handler:
  pushad

  call keyboard_handler_main
  call shell_handler_main
  
  popad

  iret

syscall_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10   
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp      
    call syscall_handler_main
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    iret
