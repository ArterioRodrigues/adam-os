#include "../pch.h"

volatile unsigned int timer_ticks = 0;

void timer_handler_main() {
  timer_ticks++;
  outb(PIC1_COMMAND, PIC_EOI);
}

uint32_t get_timer_ticks() { return timer_ticks; }

uint32_t get_uptime_seconds() { return timer_ticks / TIMER_FREQ; }

void init_timer() {
  uint16_t divisor = PIT_BASE_FREQ / TIMER_FREQ;

  outb(PIT_COMMAND, PIT_MODE2);

  uint8_t low = divisor & BYTE_MASK;
  outb(PIT_CHANNEL0, low);

  uint8_t high = (divisor >> 8) & BYTE_MASK;
  outb(PIT_CHANNEL0, high);

  extern void timer_handler();
  idt_set_gate(IRQ_TIMER, (unsigned int)timer_handler, KERNEL_CODE_SEGMENT,
               IDT_FLAG_INTERRUPT_GATE);
}
