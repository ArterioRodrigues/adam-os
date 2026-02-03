[BITS 32]
; Declares a external function kernelMain 
; linker will add the function here
[EXTERN kernelMain]

; Makes this global for the linker to see it and able to call it 
; It's starts the kernel at 0x1000
global _start

_start:
  ; linker will put the address of kernelMain here for this call
  call kernelMain
  jmp $
