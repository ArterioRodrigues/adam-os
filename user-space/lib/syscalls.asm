[BITS 32]

global _start
global sys_exit
global sys_fork
global sys_read
global sys_write
global sys_open
global sys_close

global sys_exec

extern main
_start:
    jmp start_real
    db 0xAD, 0xDE, 0x4F, 0x53, 0x41, 0x44, 0x41, 0x4D
start_real:
    call main       
    call sys_exit
    call main       
    call sys_exit

sys_exit:
    mov eax, 1
    int 0x80

sys_fork:
    mov eax, 2 
    int 0x80
    ret

sys_read:
    mov eax, 3 
    mov ebx, [esp + 4]  
    mov ecx, [esp + 8]  
    mov edx, [esp + 12]  
    int 0x80
    ret

sys_write:
    mov eax, 4          
    mov ebx, [esp + 4]  
    mov ecx, [esp + 8]  
    mov edx, [esp + 12]  
    int 0x80
    ret

sys_open:
  mov eax, 5
  mov ebx, [esp + 4]
  int 0x80
  ret

sys_close:
  mov eax, 6 
  mov ebx, [esp + 4]
  int 0x80
  ret

sys_exec:
    mov eax, 11          
    mov ecx, [esp + 4]  
    int 0x80
    ret
