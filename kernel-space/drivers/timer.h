#ifndef TIMER_H
#define TIMER_H
#include "../kernel/types.h"

extern void timer_handler();
extern volatile unsigned int timer_ticks;

void init_timer();

void timer_handler_main(registers_t *regs);
unsigned int get_timer_ticks();
unsigned int get_uptime_seconds();
#endif
