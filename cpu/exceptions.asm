; For exceptions WITHOUT error code (0, 1, 3, 4, 5, 6, 16, 18, 19)
; Need to push dummy 0 to keep stack consistent

; Common stub - saves registers and calls C handler
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
  push 0              ; Dummy error code (exception 0 doesn't push one)
  push 0              ; Exception number (0 for divide error)
  jmp exception_common_stub

debug_handler:
  push 1 
  jmp exception_common_stub

breakpoint_handler:
  push 3 
  jmp exception_common_stub

overflow_handler:
  push 4 
  jmp exception_common_stub

bound_range_handler:
  push 5
  jmp exception_common_stub

invalid_opcode_handler:
  push 0              ; Dummy error code
  push 6              ; Exception number (6 for invalid opcode)
  jmp exception_common_stub

device_not_available_handler:
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
  push 13             ; Exception number (13 for GPF)
  jmp exception_common_stub

page_fault_handler:
  push 14             ; Exception number (14 for page fault)
  jmp exception_common_stub

fpu_error_handler:
  push 16
  jmp exception_common_stub

alignment_check_handler:
  push 17 
  jmp exception_common_stub

machine_check_handler:
  push 18 
  jmp exception_common_stub

simd_fp_handler:
  push  19
  jmp exception_common_stub

exception_common_stub:
    ; Save all registers
    pusha               ; Pushes: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler
    ; Stack now has: int_no, err_code, and all registers
    call exception_handler_c
    
    ; Restore segments
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore registers
    popa
    
    ; Clean up exception number and error code
    add esp, 8
    
    ; Return from interrupt
    iret
