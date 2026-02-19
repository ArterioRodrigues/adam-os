[BITS 32]
; For exceptions WITHOUT error code (0, 1, 3, 4, 5, 6, 16, 18, 19)
; Need to push dummy 0 to keep stack consistent
extern exception_handler_c

global divide_error_handler
global debug_handler
global breakpoint_handler
global overflow_handler
global bound_range_handler
global invalid_opcode_handler
global device_not_available_handler
global double_fault_handler
global invalid_tss_handler
global segment_not_present_handler
global stack_fault_handler
global gpf_handler
global page_fault_handler
global fpu_error_handler
global alignment_check_handler
global machine_check_handler
global simd_fp_handler


divide_error_handler:
  push 0              
  push 0              
  jmp exception_common_stub

debug_handler:
  push 0  
  push 1 
  jmp exception_common_stub

breakpoint_handler:
  push 0
  push 3 
  jmp exception_common_stub

overflow_handler:
  push 0
  push 4 
  jmp exception_common_stub

bound_range_handler:
  push 0
  push 5
  jmp exception_common_stub

invalid_opcode_handler:
  push 0              
  push 6              
  jmp exception_common_stub

device_not_available_handler:
  push 0
  push 7 
  jmp exception_common_stub

double_fault_handler:
  push 8 
  jmp exception_common_stub

invalid_tss_handler:
  push 10
  jmp exception_common_stub

segment_not_present_handler:
  push 11 
  jmp exception_common_stub

stack_fault_handler:
  push 12 
  jmp exception_common_stub

gpf_handler:
  push 13             
  jmp exception_common_stub

page_fault_handler:
  push 14             
  jmp exception_common_stub

fpu_error_handler:
  push 16
  jmp exception_common_stub

alignment_check_handler:
  push 17 
  jmp exception_common_stub

machine_check_handler:
  push 0
  push 18 
  jmp exception_common_stub

simd_fp_handler:
  push 0
  push  19
  jmp exception_common_stub

exception_common_stub:
    pusha              
    
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10      
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call exception_handler_c
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    
    popa
    
    add esp, 8
    
    iret
