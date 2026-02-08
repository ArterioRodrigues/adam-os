#ifndef TIMER_H
#define TIMER_H

extern void timer_handler();
extern volatile unsigned int timer_ticks;

void init_timer();
void timer_handler_main();

unsigned int get_timer_ticks();
unsigned int get_uptime_seconds();
#endif
