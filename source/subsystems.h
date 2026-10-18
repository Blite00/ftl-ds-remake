#ifndef FTL_DS_SUBSYSTEMS_H
#define FTL_DS_SUBSYSTEMS_H
#include "game.h"
void subsystems_init(Game *game,int side);
int subsystem_level(const Game *game,int side,int kind);
int game_room_visible(const Game *game,int side,int room);
int game_enemy_details_visible(const Game *game);
void subsystem_damage(Game *game,int side,int room,int amount);
void subsystem_step(Game *game,int service);
int subsystem_door_pass(Game *game,int faction,int side,int door);
#endif
