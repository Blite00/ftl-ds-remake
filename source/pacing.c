#include "pacing.h"
#include "game.h"
#if GAME_HZ < 1 || GAME_HZ > 60
#error GAME_HZ must be in 1..60
#endif
void pacing_reset(Pacing*p,uint32_t sample){p->sample=sample;p->fraction=0;}
unsigned pacing_advance(Pacing*p,uint32_t sample){
 uint32_t elapsed=sample-p->sample;p->sample=sample;
 /* Bound multiplication and work even after suspension or sample wrap. */
 const uint32_t limit=PACING_MAX_STEPS*PACING_VBLANK_HZ/GAME_HZ;
 if(elapsed>limit){p->dropped+=elapsed-limit;elapsed=limit;}
 uint32_t total=p->fraction+elapsed*GAME_HZ;
 p->fraction=total%PACING_VBLANK_HZ;
 return total/PACING_VBLANK_HZ;
}
