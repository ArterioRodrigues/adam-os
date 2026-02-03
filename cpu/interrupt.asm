[BITS 32]

global keyboardHandler
global idtLoad 
extern keyboardHandlerMain
extern shellHandlerMain


idtLoad:
  ; get the function argument from the stack we do ESP + 4 because that holds the first arguemnt 
  ; While esp + 0 holds the return address
  mov eax, [esp + 4]

  lidt [eax]
  ret

keyboardHandler:
  ; Save all registers onto stack
  pushad

  ; Call function to handle the keyboard handler
  call keyboardHandlerMain
  call shellHandlerMain
  
  ; Pop all registers onto stack
  popad

  ; return but all restores the CPU state
  iret
