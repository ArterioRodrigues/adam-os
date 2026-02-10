#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "../kernel/types.h"

extern void divide_error_handler();
extern void debug_handler();
extern void breakpoint_handler();
extern void overflow_handler();
extern void bound_range_handler();
extern void invalid_opcode_handler();
extern void device_not_available_handler();
extern void double_fault_handler();
extern void invalid_tss_handler();
extern void segment_not_present_handler();
extern void stack_fault_handler();
extern void gpf_handler();
extern void page_fault_handler();
extern void fpu_error_handler();
extern void alignment_check_handler();
extern void machine_check_handler();
extern void simd_fp_handler();

extern const char* exception_messages[];

typedef struct {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void init_exception();
void exception_handler_c(registers_t *regs);
#endif
