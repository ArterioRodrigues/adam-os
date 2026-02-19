[BITS 32]

global jump_usermode
global sys_write
global sys_exit

USER_FUNC_VADDR  equ 0x40000000
USER_STACK_VADDR equ 0x50000000

jump_usermode:
    mov ax, (4 * 8) | 3     ; ring 3 data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; iret frame: SS, ESP, EFLAGS, CS, EIP
    push (4 * 8) | 3         ; SS  — ring 3 data
    push USER_STACK_VADDR + 0x1000 - 4  ; ESP — top of user stack page
    pushf
    push (3 * 8) | 3         ; CS  — ring 3 code
    push USER_FUNC_VADDR     ; EIP — virtual address of copied function
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
