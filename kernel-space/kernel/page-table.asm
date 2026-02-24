[BITS 32]

global enable_paging
global disable_paging
global load_page_directory

enable_paging:
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    ret

disable_paging:
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    ret

load_page_directory:
    mov eax, [esp + 4]      
    mov cr3, eax            
    ret

