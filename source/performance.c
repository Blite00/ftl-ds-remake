#include "performance.h"
#include <stdio.h>
#include <stdint.h>
#ifdef ARM9
#include <nds.h>
static uint64_t total_ticks;
static uint32_t samples,minimum_ticks,maximum_ticks;
static int available,running;
void performance_init(void){
 total_ticks=0;samples=0;minimum_ticks=UINT32_MAX;maximum_ticks=0;running=0;
 available=((TIMER0_CR|TIMER1_CR)&TIMER_ENABLE)==0;
}
void performance_begin(void){
 if(!available||running)return;
 if((TIMER0_CR|TIMER1_CR)&TIMER_ENABLE){available=0;return;}
 cpuStartTiming(0);running=1;
}
void performance_end(void){
 uint32_t ticks;if(!running)return;
 ticks=cpuEndTiming();running=0;
 if(ticks<minimum_ticks)minimum_ticks=ticks;
 if(ticks>maximum_ticks)maximum_ticks=ticks;
 total_ticks+=ticks;samples++;
}
void performance_report(char *buffer,int capacity){
 if(!buffer||capacity<=0)return;
 if(!available){snprintf(buffer,(size_t)capacity,"work timing unavailable: timers 0/1 busy");return;}
 if(!samples){snprintf(buffer,(size_t)capacity,"work timing: no samples");return;}
 snprintf(buffer,(size_t)capacity,"work_us n=%lu min=%lu avg=%lu max=%lu (elapsed incl IRQ)",
 (unsigned long)samples,(unsigned long)timerTicks2usec(minimum_ticks),
 (unsigned long)timerTicks2usec((uint32_t)(total_ticks/samples)),
 (unsigned long)timerTicks2usec(maximum_ticks));
}
#else
void performance_init(void){}
void performance_begin(void){}
void performance_end(void){}
void performance_report(char *buffer,int capacity){
 if(buffer&&capacity>0)snprintf(buffer,(size_t)capacity,"work timing unavailable: host stub, no DS measurement");
}
#endif
