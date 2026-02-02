[BITS 32]
[EXTERN kernelMain]

global _start

_start:
  call kernelMain
  jmp $
