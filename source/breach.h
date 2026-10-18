#ifndef FTL_DS_BREACH_H
#define FTL_DS_BREACH_H
#include "game.h"
#define BREACH_SEAL_TICKS (5*GAME_HZ)
void breach_add(Ship *ship,int room,int amount);
void breach_step(Game *game,int service);
#endif
