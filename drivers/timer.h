#ifndef TIMER_H
#define TIMER_H

extern void timer_handler();
extern volatile unsigned int timer_ticks;

void timer_handler_main();
void sleep(unsigned int seconds);
unsigned int get_timer_ticks();
unsigned int get_uptime_seconds();
void init_timer();

#endif
