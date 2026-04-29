[BITS 32]
global switch_to_process

switch_to_process:
    mov eax, [esp + 4]      ; pcb pointer
    mov ebx, eax            ; use ebx as base; eax will be reloaded from PCB

    mov dx, (4 * 8) | 3
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    ; Build the iret frame
    push dword [ebx + 72]   ; ss
    push dword [ebx + 68]   ; useresp
    push dword [ebx + 64]   ; eflags
    push dword [ebx + 60]   ; cs
    push dword [ebx + 56]   ; eip

    ; Load GPRs from PCB. Load ebx LAST since we're using it as the base.
    mov edi, [ebx + 16]
    mov esi, [ebx + 20]
    mov ebp, [ebx + 24]
    mov edx, [ebx + 36]
    mov ecx, [ebx + 40]
    mov eax, [ebx + 44]
    mov ebx, [ebx + 32]

    iret
