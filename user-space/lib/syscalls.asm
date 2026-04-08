[BITS 32]

global _start
global sys_exit
global sys_fork
global sys_read
global sys_write
global sys_open
global sys_close
global sys_ps

global sys_create
global sys_exec
global sys_kill
global sys_waitpid

global sys_poll
global sys_sleep
global sys_uptime
global sys_create_window
global sys_create_rect
global sys_create_text
global sys_get_event
global sys_destroy_window
global sys_flush
global sys_getpid
global sys_getppid
global sys_sbrk 

extern main
_start:
    call main       
    call sys_exit

sys_exit:
    mov eax, 1
    int 0x80
    ret

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

sys_ps:
  mov eax, 7 
  mov ebx, [esp + 4]
  mov ecx, [esp + 8]
  int 0x80
  ret

sys_exec:
    mov eax, 11          
    mov ecx, [esp + 4]  
    mov edx, [esp + 8]  
    int 0x80
    ret

sys_kill:
    mov eax, 12 
    mov ebx, [esp + 4]  
    int 0x80
    ret

sys_waitpid:
    mov eax, 13 
    mov ebx, [esp + 4]  
    int 0x80
    ret


sys_create:
    mov eax, 14
    mov ebx, [esp + 4]   
    mov ecx, [esp + 8]  
    mov edx, [esp + 12]  
    int 0x80
    ret

sys_poll:
    mov eax, 15
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_sleep:
    mov eax, 16
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_uptime:
    mov eax, 17
    int 0x80
    ret

sys_create_window:
    mov eax, 18
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_create_rect:
    mov eax, 19
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_create_text:
    mov eax, 20
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_get_event:
    mov eax, 21 
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int 0x80
    ret

sys_destroy_window:
    mov eax, 22 
    mov ebx, [esp + 4]
    int 0x80
    ret

sys_flush:
    mov eax, 23 
    int 0x80
    ret

sys_getpid:
    mov eax, 24
    int 0x80
    ret

sys_getppid:
    mov eax, 25
    int 0x80
    ret

sys_sbrk: 
    mov eax, 26
    mov ebx, [esp + 4]
    int 0x80
    ret
