#include "drones.h"
#include "species.h"
#include "fire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static int effects(void){int i,n=0;for(i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active;return n;}
static void setup(void){int i;game_init(&g);assert(!g.drones.installed&&g.drones.room==-1&&g.drones.slot_count==2&&g.drone_parts==2);for(i=0;i<GAME_MAX_DRONES;i++)assert(g.drones.slots[i].catalog_id==-1);
 assert(drones_install(&g));assert(g.drones.room==1&&g.drones.health==2);assert(drones_equip(&g,0,0));assert(drones_equip(&g,1,0));g.player.reactor=30;g.drone_parts=4;
}
int main(void){int i,charge,target;unsigned rng;DroneState first;
 setup();before=g;assert(!drones_equip(&g,-1,0)&&!drones_equip(&g,2,0)&&!drones_equip(&g,0,GAME_DRONE_CATALOG_COUNT)&&!drones_remove(&g,2)&&!drones_reorder(&g,0,2));assert(!memcmp(&before,&g,sizeof g));
 assert(!drones_toggle(&g,0)&&g.drone_parts==4&&!g.drones.slots[0].deployed);assert(drones_adjust_power(&g,1)&&drones_adjust_power(&g,1));assert(!drones_adjust_power(&g,1));assert(game_used_power(&g)>=2);
 g.drone_parts=0;assert(!drones_toggle(&g,0)&&!g.drones.slots[0].enabled&&!g.drones.slots[0].deployed);g.drone_parts=4;g.paused=1;assert(drones_toggle(&g,0)&&g.drone_parts==3);drones_step(&g);assert(!g.drones.slots[0].charge_ticks);g.paused=0;
 assert(!drones_toggle(&g,1)&&g.drone_parts==3);for(i=0;i<50;i++)drones_step(&g);assert(g.drones.slots[0].charge_ticks==50);
 assert(drones_toggle(&g,0));for(i=0;i<20;i++)drones_step(&g);assert(g.drones.slots[0].charge_ticks==50);assert(drones_toggle(&g,0)&&g.drone_parts==3);
 assert(drones_adjust_power(&g,-1));charge=g.drones.slots[0].charge_ticks;drones_step(&g);assert(g.drones.slots[0].charge_ticks==charge&&g.drones.slots[0].deployed);assert(drones_adjust_power(&g,1));
 g.drones.ion_bars=2;drones_step(&g);assert(g.drones.slots[0].charge_ticks==charge);g.drones.ion_bars=0;g.drones.health=1;drones_step(&g);assert(g.drones.slots[0].charge_ticks==charge);g.drones.health=2;
 g.drones.power=0;g.drones.bonus_power=2;g.drones.ion_bars=2;assert(drones_powered(&g,0));drones_step(&g);assert(g.drones.slots[0].charge_ticks==charge+1);g.drones.bonus_power=g.drones.ion_bars=0;g.drones.power=2;
 /* A saturated projectile pool retains readiness and one target. */
 for(i=0;i<GAME_MAX_EFFECTS;i++){g.effects[i].active=1;}g.drones.slots[0].charge_ticks=GAME_DRONE_COOLDOWN_TICKS-1;drones_step(&g);target=g.drones.slots[0].target_room;rng=g.rng;assert(target>=0&&g.drones.slots[0].charge_ticks==GAME_DRONE_COOLDOWN_TICKS);drones_step(&g);assert(g.rng==rng&&g.drones.slots[0].target_room==target);g.effects[2].active=0;drones_step(&g);assert(g.effects[2].active&&g.effects[2].side==0&&g.effects[2].room==target&&g.effects[2].kind==EFFECT_LASER&&g.effects[2].damage==1&&!g.drones.slots[0].charge_ticks&&g.drone_parts==3);
 memset(g.effects,0,sizeof g.effects);g.outcome=GAME_VICTORY;drones_step(&g);assert(!effects()&&!drones_toggle(&g,0));g.outcome=GAME_PLAYING;g.enemy.hull=0;drones_step(&g);assert(!effects()&&!drones_toggle(&g,0));
 first=g.drones.slots[0];assert(drones_reorder(&g,0,1));assert(!memcmp(&first,&g.drones.slots[1],sizeof first));drones_reset(&g);assert(g.drone_parts==3&&g.drones.power==2&&g.drones.slots[1].catalog_id==0&&!g.drones.slots[1].deployed&&!g.drones.slots[1].enabled&&g.drones.slots[1].target_room==-1);
 game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(drones_toggle(&g,0)&&g.drone_parts==2);assert(drones_remove(&g,0)&&g.drones.slots[0].catalog_id==-1&&!g.drones.slots[0].deployed&&g.drone_parts==2);
 /* Room-targeted ion, damage, and ordinary effect resolution share core rules. */
 setup();g.drones.power=2;assert(drones_toggle(&g,0));g.enemy.systems[SYS_ENGINES].power=0;g.enemy.shields=1;g.enemy.shield_ticks=0;g.enemy_weapon.cooldown_ticks=999999;g.drones.slots[0].charge_ticks=GAME_DRONE_COOLDOWN_TICKS-1;game_step(&g);assert(effects()==1);for(i=0;i<12;i++)game_step(&g);assert(g.enemy.shields==0);
 /* Refused departure cannot refresh or erase a paid deployment. */
 first=g.drones.slots[0];g.crew[0].ship=1;game_begin_encounter(&g,ENEMY_REBEL,1,0);assert(!memcmp(&first,&g.drones.slots[0],sizeof first));g.crew[0].ship=0;
 setup();g.enemy_weapon.cooldown_ticks=999999;g.player.systems[SYS_SHIELDS].power=0;g.player.shields=0;g.player.systems[SYS_ENGINES].power=0;g.drones.power=2;assert(drones_toggle(&g,0));assert(game_launch_ion(&g,1,g.drones.room,1));for(i=0;i<12;i++)game_step(&g);assert(g.drones.ion_bars==1&&!drones_powered(&g,0));charge=g.drones.slots[0].charge_ticks;game_step(&g);assert(g.drones.slots[0].charge_ticks==charge);
 {GameDamage damage={0,2,0,0,0,0,0,0};assert(game_launch_bomb(&g,1,g.drones.room,&damage));for(i=0;i<12;i++)game_step(&g);assert(!g.drones.health&&!g.drones.power&&g.drones.slots[0].deployed);}
 g.crew[1].room=g.crew[1].destination=g.drones.room;g.crew[1].repair_ticks=0;g.outcome=GAME_VICTORY;for(i=0;i<10*GAME_HZ;i++)game_service_step(&g);assert(g.drones.health==2&&!g.drones.ion_bars&&!g.drones.power);assert(g.drones.slots[0].charge_ticks==charge);
 assert(game_set_crew_species(&g,0,1,SPEC_ZOLTAN));g.crew[2].room=g.crew[2].destination=g.drones.room;assert(game_set_crew_species(&g,0,2,SPEC_ZOLTAN));species_refresh_power(&g);assert(g.drones.bonus_power==2&&drones_powered(&g,0));g.outcome=GAME_PLAYING;game_step(&g);assert(g.drones.slots[0].charge_ticks==charge+1);
 /* Unattended control fire removes health and prevents operation. */
 setup();g.drones.power=2;g.ticks=3*GAME_HZ;fire_ignite(&g.player,g.drones.room,1);fire_step(&g,1);assert(g.drones.health==1&&g.drones.power==1);
 puts("PASS: drone parts deployment, pause, suspension, power/ion/health/Zoltan guards, bounded shot retry, reset, slot identity and shields.");return 0;
}
