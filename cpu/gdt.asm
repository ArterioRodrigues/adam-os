[BITS 32]

global tss_flush
global gdt_flush

extern gdt_ptr_struct;

TSS_SELECTOR    equ (5 * 8) | 0
KERNEL_CODE_SEG equ 0x08
KERNEL_DATA_SEG equ 0x10

tss_flush:
	mov ax, TSS_SELECTOR 
	ltr ax
	ret

gdt_flush:
    lgdt [gdt_ptr_struct]          
    jmp KERNEL_CODE_SEG:.reload_cs

.reload_cs:
    mov ax, KERNEL_DATA_SEG 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
