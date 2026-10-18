#ifndef FTL_DS_OUTCOME_H
#define FTL_DS_OUTCOME_H
#include "game.h"
#define OUTCOME_SAMPLE_COUNT 12
enum OutcomeSampleKind { OUTCOME_INACTIVE, OUTCOME_BLAST, OUTCOME_DEBRIS };
/* Pure presentation queries. Caller owns the age clock and pause policy.
   No samples for crew-only victory/defeat or a nonterminal result. */
int outcome_duration(const Game *game);
/* Ship to animate: 0 player, 1 enemy, -1 none. */
int outcome_ship(const Game *game);
/* Active sample centers are 0..99; radius is in the same normalized space.
   Return kind, or zero without changing outputs when inactive/invalid.
   Indices 0..5 are blasts, 6..11 are debris. Renderer clips at screen bounds. */
int outcome_sample(const Game *game,int age,int index,int *x,int *y,int *radius);
#endif
