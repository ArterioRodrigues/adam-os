[BITS 32]

global _start
global sys_write
global sys_exit

extern main
_start:
    call main       
    call sys_exit

; sys_write(char *buf, uint32_t len)
; returns number of bytes written in eax
sys_write:
    mov eax, 4          
    mov ecx, [esp + 4]  
    mov edx, [esp + 8]  
    int 0x80
    ret

; sys_exit()
; does not return
sys_exit:
    mov eax, 1
    int 0x80

