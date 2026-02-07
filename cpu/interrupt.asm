[BITS 32]

global timer_handler
global keyboard_handler
global idt_load 

extern keyboard_handler_main
extern timer_handler_main 
extern shell_handler_main


idt_load:
  ; get the function argument from the stack we do ESP + 4 because that holds the first arguemnt 
  ; While esp + 0 holds the return address
  mov eax, [esp + 4]

  lidt [eax]
  ret

timer_handler:
  pushad

  call timer_handler_main

  popad
  iret

keyboard_handler:
  ; Save all registers onto stack
  pushad

  ; Call function to handle the keyboard handler
  call keyboard_handler_main
  call shell_handler_main
  
  ; Pop all registers onto stack
  popad

  ; return but all restores the CPU state
  iret
