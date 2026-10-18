/* Charged-weapon fixtures isolate room impacts from misses, repairs and enemy
 * offense; these are behavior checks, not earned campaign evidence. */
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void setup(void){game_init(&g);assert(game_begin_encounter(&g,ENEMY_ARMORED,0,0));g.enemy.systems[SYS_ENGINES].power=0;g.enemy_weapon.cooldown_ticks=10000;for(int i=0;i<g.weapon_count;i++)g.weapons[i].fire_chance=g.weapons[i].breach_chance=0;}
static void volley(int slot,int room){g.weapons[slot].charge_ticks=g.weapons[slot].cooldown_ticks;assert(game_fire(&g,slot,room));for(int t=0;t<60;t++)game_step(&g);}
int main(void){
 setup();int room=g.enemy.systems[SYS_SHIELDS].room,hull=g.enemy.hull;assert(g.enemy.shields==1);volley(0,room);assert(g.enemy.hull==hull-2&&g.enemy.systems[SYS_SHIELDS].health==0&&g.enemy.systems[SYS_SHIELDS].power==0&&g.enemy.shields==0&&game_shield_capacity(&g.enemy)==0);assert(g.enemy.systems[SYS_WEAPONS].health==3);assert(strstr(g.message,"hit: system damaged."));
 setup();room=g.enemy.systems[SYS_WEAPONS].room;volley(0,room);assert(g.enemy.systems[SYS_WEAPONS].health==1&&g.enemy.systems[SYS_WEAPONS].power==1&&g.enemy_fire_ticks>0);volley(0,room);assert(g.enemy.systems[SYS_WEAPONS].health==0&&g.enemy.systems[SYS_WEAPONS].power==0&&g.enemy_fire_ticks==0);for(int t=0;t<90;t++)game_step(&g);assert(!g.enemy_fire_ticks&&!g.enemy_pending_shots);
 setup();room=g.enemy.systems[SYS_ENGINES].room;g.enemy.systems[SYS_ENGINES].power=2;g.rng=0;assert(game_enemy_escape_ready(&g));volley(0,room);assert(!game_enemy_escape_ready(&g));assert(!g.enemy.systems[SYS_ENGINES].health&&!g.enemy.systems[SYS_ENGINES].power&&g.enemy.systems[SYS_WEAPONS].health==3);
 /* One laser is absorbed; remaining shots hit only the queued room even when
  * the next-volley lock changes while the burst is in flight. */
 setup();room=g.enemy.systems[SYS_WEAPONS].room;hull=g.enemy.hull;g.weapons[1].charge_ticks=g.weapons[1].cooldown_ticks;assert(game_set_weapon_target(&g,1,room)&&game_fire(&g,1,room));assert(game_set_weapon_target(&g,1,g.enemy.systems[SYS_SHIELDS].room));for(int t=0;t<60;t++)game_step(&g);assert(g.enemy.hull==hull-2&&g.enemy.systems[SYS_WEAPONS].health==1&&g.enemy.systems[SYS_SHIELDS].health==2&&g.weapons[1].target_room==room&&g.weapons[1].aim_room!=room);
 setup();g.enemy.systems[SYS_WEAPONS].health=g.enemy.systems[SYS_WEAPONS].power=0;volley(0,g.enemy.systems[SYS_WEAPONS].room);assert(!strcmp(g.message,"Enemy hull hit."));
 setup();GameDamage zero={0};assert(game_launch_effect(&g,0,g.enemy.systems[SYS_WEAPONS].room,EFFECT_MISSILE,&zero,5));for(int t=0;t<20;t++)game_step(&g);assert(!strcmp(g.message,"Enemy room hit."));
 setup();GameDamage laser={1,1,5,0,0,0,0,0};hull=g.enemy.hull;assert(game_launch_effect(&g,0,g.enemy.systems[SYS_WEAPONS].room,EFFECT_LASER,&laser,0));for(int t=0;t<20;t++)game_step(&g);assert(g.enemy.hull==hull&&g.enemy.systems[SYS_WEAPONS].health==3&&!strcmp(g.message,"Enemy shield absorbed the shot."));
 setup();hull=g.enemy.hull;room=-1;for(int i=0;i<g.enemy.room_count;i++)if(g.enemy.rooms[i].system<0){room=g.enemy.rooms[i].id;break;}assert(room>=0);volley(0,room);assert(g.enemy.hull==hull-2&&g.enemy.systems[SYS_WEAPONS].health==3&&g.enemy.systems[SYS_SHIELDS].health==2&&g.enemy.systems[SYS_ENGINES].health==2);
 puts("Room impacts: Artemis disables shields/engines, successive weapon hits cross firing-power threshold, Burst Laser shield absorption and queued target persistence, empty-room hull-only damage passed.");
}
