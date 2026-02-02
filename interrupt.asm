[BITS 32]

global keyboardHandler
global idtLoad 
extern keyboardHandlerMain

idtLoad:
  mov eax, [esp + 4]
  lidt [eax]
  ret

keyboardHandler:
  pushad

  call keyboardHandlerMain

  popad
  iret
