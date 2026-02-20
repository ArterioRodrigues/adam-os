[BITS 32]

global jump_usermode
global sys_write
global sys_exit

USER_FUNC_VADDR  equ 0x40000000
USER_STACK_VADDR equ 0x50000000

jump_usermode:
    mov ax, (4 * 8) | 3     
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push (4 * 8) | 3         
    push USER_STACK_VADDR + 0x1000 - 4  
    pushf
    push (3 * 8) | 3         
    push USER_FUNC_VADDR     
    iret

; sys_write(char *buf, uint32_t len)
; returns number of bytes written in eax
sys_write:
    mov eax, 4          ; SYSCALL_WRITE
    mov ecx, [esp + 4]  ; buf
    mov edx, [esp + 8]  ; len
    int 0x80
    ret

; sys_exit()
; does not return
sys_exit:
    mov eax, 1          ; SYSCALL_EXIT
    int 0x80

