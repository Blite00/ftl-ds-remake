#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void impact(Game*g,int side,int room,int piercing,int fire){
 memset(g->effects,0,sizeof g->effects);
 GameEffect*e=&g->effects[0];e->active=1;e->duration=1;e->side=side;e->room=room;e->damage=1;e->system_damage=1;e->crew_damage=5;e->piercing=piercing;e->fire_chance=fire;
 game_step(g);
}
int main(void){
 Game g;game_init(&g);game_begin_encounter(&g,ENEMY_ASSAULT,0,0);
 g.enemy.systems[SYS_ENGINES].power=0;g.enemy_drone.power=0;
 int hull=g.enemy.hull,room=g.enemy.systems[SYS_SHIELDS].room;
 assert(game_shield_capacity(&g.enemy)==1);
 impact(&g,0,room,0,100);assert(g.enemy.hull==hull&&g.enemy.shields==0&&g.effects[0].result==EFFECT_SHIELD);
 for(int i=0;i<GAME_SHIELD_RECHARGE_TICKS-1;i++)game_step(&g);
 assert(g.enemy.shields==0);g.paused=1;game_step(&g);assert(g.enemy.shield_ticks==GAME_SHIELD_RECHARGE_TICKS-1);
 g.paused=0;game_step(&g);assert(g.enemy.shields==1&&g.enemy.shield_ticks==0);
 impact(&g,0,room,5,0);assert(g.enemy.hull==hull-1&&game_shield_capacity(&g.enemy)==0&&g.enemy.shields==0);
 g.player.systems[SYS_ENGINES].power=0;g.player.shields=0;g.player.systems[SYS_SHIELDS].power=0;
 int target=g.player.rooms[0].id;impact(&g,1,target,0,100);
 assert(g.player.rooms[0].fires==1);assert(g.effects[0].result==EFFECT_HIT);
 puts("Shields: absorb, exact recharge, pause, piercing collapse and projectile ignition passed");return 0;
}
