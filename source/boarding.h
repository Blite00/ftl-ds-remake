#ifndef FTL_DS_BOARDING_H
#define FTL_DS_BOARDING_H
#include "game.h"
#define BOARDING_COOLDOWN (20*GAME_HZ)
int boarding_install(Game *game,int side);
int boarding_power(Game *game,int side,int delta);
int boarding_deploy(Game *game,unsigned mask,int enemy_room);
int boarding_recall(Game *game,unsigned mask);
void boarding_step(Game *game,int service);
int boarding_contested(const Game *game,const Crew *crew,int faction);
void boarding_prepare_departure(Game *game);
void boarding_destroyed_ships(Game *game);
/* Original FTL ShipAI/CrewAI boarding rules (see docs/V39_BOARDING_AI.md). */
int boarding_enemy_request(Game *game);
int boarding_manning_priority(const Game *game,const Crew *crew);
/* current[i] is crew i's present task priority (lower is more important). */
unsigned boarding_enemy_party(Game *game,const int *current,int count);
int boarding_enemy_evac_forced(const Game *game);
int boarding_unit_sabotage(Game *game,int side,int room,int *ticks);
#endif
