#ifndef FTL_DS_FIRE_H
#define FTL_DS_FIRE_H
#include "game.h"
/* Bounded demake fire rules; timings are authored, not verified PC behavior. */
void fire_ignite(Ship *ship,int room,int amount);
/* Called once per simulation tick after atmosphere/crew; service affects player only. */
void fire_step(Game *game,int service);
#endif
