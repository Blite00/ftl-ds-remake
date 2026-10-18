#include "pacing.h"
#include "game.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static unsigned run(unsigned frames,unsigned stride){
 Pacing p={0};unsigned steps=0;for(unsigned now=stride;now<=frames;now+=stride)steps+=pacing_advance(&p,now);return steps;
}
int main(void){
 /* Identical game-time progression with render work taking 1, 2, 3, or 4
  * display frames. Old every-other-loop scheduling fails these cases. */
 assert(run(600,1)==10*GAME_HZ);assert(run(600,2)==10*GAME_HZ);
 assert(run(600,3)==10*GAME_HZ);assert(run(600,4)==10*GAME_HZ);
 Pacing p={0};assert(pacing_advance(&p,1)==0);assert(pacing_advance(&p,1)==0);
 assert(pacing_advance(&p,2)==1);assert(pacing_advance(&p,3)==0);
 assert(pacing_advance(&p,8)==3); /* Fraction survives variable frame times. */
 assert(pacing_advance(&p,1000)==PACING_MAX_STEPS);assert(p.dropped==984);
 assert(pacing_advance(&p,1000)==0);assert(pacing_advance(&p,1001)==0);
 assert(pacing_advance(&p,1002)==1); /* No backlog after a long stall. */
 pacing_reset(&p,5000);assert(pacing_advance(&p,5001)==0);
 assert(pacing_advance(&p,5002)==1); /* Menu/load time was discarded. */
 pacing_reset(&p,UINT32_MAX-1);assert(pacing_advance(&p,UINT32_MAX)==0);
 assert(pacing_advance(&p,0)==1); /* Unsigned IRQ counter wraps safely. */
 puts("pacing_smoke: passed");return 0;
}
