[BITS 32]

global switch_to_process

switch_to_process:
  mov eax, [esp + 4] 
  
  mov bx, (4 * 8) | 3
  mov ds, bx 
  mov es, bx 
  mov fs, bx 
  mov gs, bx 

  mov ebx, eax
  
  push dword [ebx + 72]
  push dword [ebx + 68]
  push dword [ebx + 64]
  push dword [ebx + 60]
  push dword [ebx + 56]
  iret

