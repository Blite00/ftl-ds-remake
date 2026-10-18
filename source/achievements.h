#ifndef FTL_DS_ACHIEVEMENTS_H
#define FTL_DS_ACHIEVEMENTS_H
#include "game.h"
/* V59 base-game ship achievements, 3 per cruiser in hangar class order (id/3 is
 * the class). Two of a cruiser's three unlock its Type B layout. Names and
 * descriptions come from the original data (achievement_catalog.h). Each one
 * only counts while flying its cruiser (either layout), as in the original. */
enum Achievement {
 ACH_KESTREL_FEDERATION, ACH_KESTREL_ARSENAL, ACH_KESTREL_TOUGH,
 ACH_STEALTH_DESTROY, ACH_STEALTH_AVOID, ACH_STEALTH_TACTICAL,
 ACH_MANTIS_CREW_DEAD, ACH_MANTIS_SLAUGHTER, ACH_MANTIS_SURVIVOR,
 ACH_ENGI_ROBOTIC, ACH_ENGI_ONLY_DRONES, ACH_ENGI_IONED,
 ACH_FED_PATIENCE, ACH_FED_DIPLOMACY, ACH_FED_UPGRADE,
 ACH_SLUG_VISION, ACH_SLUG_NEBULA, ACH_SLUG_BIO,
 ACH_ROCK_FIRE, ACH_ROCK_MISSILES, ACH_ROCK_CRYSTAL,
 ACH_ZOLTAN_SHIELDS, ACH_ZOLTAN_POWER, ACH_ZOLTAN_MANPOWER,
 ACH_CRYSTAL_SHARD, ACH_CRYSTAL_LOCKDOWN, ACH_CRYSTAL_CLASH,
 ACH_COUNT };
/* Effect origins (GameEffect.origin) for "only drones / only artillery / shard" checks. */
enum AchOrigin { ORIGIN_NONE, ORIGIN_WEAPON, ORIGIN_MISSILE, ORIGIN_DRONE, ORIGIN_ARTILLERY, ORIGIN_SHARD, ORIGIN_BIO };
const char *achievement_name(int id);
const char *achievement_desc(int id);
/* Encounter tracking, called by the game core. */
void achievements_begin_encounter(Game *game);
void achievements_step(Game *game);
void achievements_effect_resolved(Game *game,const GameEffect *effect,int enemy_hull_before,int enemy_alive_before);
int achievements_enemy_alive(const Game *game);
/* Earn when flying the achievement's cruiser (sets game->ach.earned). */
void achievements_earn(Game *game,int id);
#endif
