[BITS 32]

global timer_handler
global keyboard_handler
global mouse_handler
global syscall_handler
global idt_load 

extern timer_handler_main 
extern syscall_handler_main
extern keyboard_handler_main
extern mouse_handler_main 


idt_load:
  mov eax, [esp + 4]

  lidt [eax]
  ret

%macro ISR_WRAPPER 2
global %1
%1:
    push 0
    push 0x80
    pushad
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
    call %2
    add esp, 4
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8
    iret
%endmacro

ISR_WRAPPER timer_handler, timer_handler_main
ISR_WRAPPER keyboard_handler, keyboard_handler_main
ISR_WRAPPER mouse_handler, mouse_handler_main
ISR_WRAPPER syscall_handler, syscall_handler_main
