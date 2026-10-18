#ifndef FTL_DS_PERFORMANCE_H
#define FTL_DS_PERFORMANCE_H
/* ARM9 elapsed work incl IRQ; uses timers 0/1. Host reports unavailable. */
void performance_init(void);
void performance_begin(void);
void performance_end(void);
void performance_report(char *buffer,int capacity);
#endif
