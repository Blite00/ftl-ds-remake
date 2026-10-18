#ifndef FTL_DS_SPECIES_H
#define FTL_DS_SPECIES_H
#include "game.h"
enum CrewSpecies { SPEC_HUMAN,SPEC_ENGI,SPEC_MANTIS,SPEC_ROCK,SPEC_SLUG,SPEC_ZOLTAN,SPEC_CRYSTAL,SPEC_COUNT };
#define CRYSTAL_LOCKDOWN_TICKS (12*GAME_HZ)
#define CRYSTAL_COOLDOWN_TICKS (40*GAME_HZ)
const char *species_name(int species);
int species_max_health(int species);
int species_move_ticks(int species);
int species_work_rate(int species,int ticks);
int species_melee_damage(int species);
int species_fire_damage(int species,int fires);
int species_suffocation_damage(int species,int ticks);
int game_set_crew_species(Game *game,int faction,int index,int species);
void species_refresh_power(Game *game);
int species_teleporter_power(const Game *game,int side);
void species_step(Game *game,int service);
int game_crystal_lockdown(Game *game,int index);
int species_door_locked(const Game *game,int side,const GameDoor *door);
int species_slug_senses(const Game *game,int side,int room);
#endif
