#ifndef FTL_DS_PACING_H
#define FTL_DS_PACING_H
#include <stdint.h>
/* Nintendo DS refresh is approximately 60 Hz, preserving the existing
 * GAME_HZ / 60 simulation ratio. No game timer or save format changes. */
#define PACING_VBLANK_HZ 60u
#define PACING_MAX_STEPS 4u
typedef struct { uint32_t sample, fraction, dropped; } Pacing;
/* Reset at menu/load/resume and other frozen-time boundaries. */
void pacing_reset(Pacing*,uint32_t vblank);
/* Poll once per loop; call simulation once for each returned step, then draw.
 * Long stalls are bounded and discarded, never replayed in later frames. */
unsigned pacing_advance(Pacing*,uint32_t vblank);
#endif
