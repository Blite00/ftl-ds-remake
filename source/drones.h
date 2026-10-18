#ifndef FTL_DS_DRONES_H
#define FTL_DS_DRONES_H
#include "game.h"
void drones_init(Game *g);
int drones_install(Game *g);
int drones_equip(Game *g,int slot,int catalog_id);
int drones_remove(Game *g,int slot);
int drones_reorder(Game *g,int from,int to);
int drones_adjust_power(Game *g,int delta);
int drones_toggle(Game *g,int slot);
int drones_powered(const Game *g,int slot);
int drones_effective_power(const Game *g);
void drones_step(Game *g);
void drones_intercept(Game *g);
int drones_is_defense(int catalog_id);
int drones_cooldown(int catalog_id);
void drones_reset(Game *g);
const char *drones_name(int catalog_id);
int drones_price(int catalog_id);
int drones_power(int catalog_id);
/* V56: 0 combat, 1 defense, 2 beam, 3 hull repair, 4 system repair, 5 anti-personnel, 6 boarding. */
int drones_kind(int catalog_id);
int drones_rarity(int catalog_id);
int drones_outside_combat(int catalog_id);
void drones_service_step(Game *g);
#endif
