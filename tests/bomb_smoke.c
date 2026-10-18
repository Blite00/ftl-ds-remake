#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void init(void){game_init(&g);g.enemy_weapon.cooldown_ticks=999999;g.enemy.systems[SYS_ENGINES].power=0;g.pilot_health=0;}
static void tick(int n){while(n--)game_step(&g);}
static void equip(void){assert(game_equip(&g,0,6));g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;}
int main(void){int i,room,hull,ammo;GameDamage payload={0,2,30,0,0,0,0,0};
 init();equip();assert(g.weapons[0].bomb&&g.weapons[0].damage==0&&g.weapons[0].system_damage==2&&g.weapons[0].crew_damage==30&&g.weapons[0].missiles==1&&g.weapons[0].power==1&&g.weapons[0].cooldown_ticks==13*GAME_HZ&&g.weapons[0].fire_chance==10);
 snapshot=g;assert(!game_set_bomb_target(&g,1,1,0)&&!game_set_bomb_target(&g,0,2,0)&&!game_launch_bomb(&g,2,0,&payload));payload.hull=1;assert(!game_launch_bomb(&g,0,0,&payload));assert(!memcmp(&snapshot,&g,sizeof g));payload.hull=0;
 /* Shield bypass retains layers when the target is a different system. */
 g.enemy.systems[SYS_SHIELDS].capacity=g.enemy.systems[SYS_SHIELDS].health=g.enemy.systems[SYS_SHIELDS].power=2;g.enemy.shields=1;room=g.enemy.systems[SYS_WEAPONS].room;hull=g.enemy.hull;assert(g.enemy.shields>0);assert(game_launch_bomb(&g,0,room,&payload));tick(12);assert(g.effects[0].result==EFFECT_HIT&&g.enemy.hull==hull&&g.enemy.shields>0&&g.enemy.systems[SYS_WEAPONS].health==0);
 /* Enemy bombs can miss according to ordinary evasion. */
 init();g.pilot_health=1;for(i=0;i<10000;i++)if((((unsigned)i*1664525u+1013904223u)>>16)%100u<(unsigned)game_evasion(&g)){g.rng=(unsigned)i;break;}assert(i<10000);room=g.player.systems[SYS_WEAPONS].room;assert(game_launch_bomb(&g,1,room,&payload));tick(12);assert(g.effects[0].result==EFFECT_MISS&&g.player.systems[SYS_WEAPONS].health==3);
 /* Self target is immutable once queued, bypasses friendly evasion and hurts both factions. */
 init();equip();g.pilot_health=1;room=g.player.systems[SYS_WEAPONS].room;g.enemy_crew_count=1;game_init_crew(&g,1,0,room);g.enemy_crew[0].ship=0;assert(game_set_bomb_target(&g,0,1,room));ammo=g.missiles;assert(game_fire(&g,0,room));assert(!game_fire(&g,0,room)&&g.missiles==ammo-1);assert(game_set_bomb_target(&g,0,0,g.enemy.rooms[0].id));g.weapons[0].crew_damage=90;g.weapons[0].system_damage=0;g.paused=1;snapshot=g;tick(4);assert(!memcmp(&g,&snapshot,sizeof g));g.paused=0;tick(1);assert(g.effects[0].side==1&&g.effects[0].room==room&&g.effects[0].bomb_friendly&&g.effects[0].crew_damage==30);tick(12);assert(g.effects[0].result==EFFECT_HIT&&g.player.hull==30&&g.player.systems[SYS_WEAPONS].health==1&&g.crew[2].health==70&&g.enemy_crew[0].health==70);
 /* Full effect pool retains one queued shot and spends ammunition only once. */
 init();equip();room=g.enemy.rooms[0].id;for(i=0;i<GAME_MAX_EFFECTS;i++){g.effects[i].active=1;g.effects[i].duration=999;}ammo=g.missiles;assert(game_fire(&g,0,room));tick(4);assert(g.weapons[0].pending_shots==1&&g.missiles==ammo-1);g.effects[0].active=0;tick(1);assert(!g.weapons[0].pending_shots&&g.effects[0].kind==EFFECT_BOMB&&g.missiles==ammo-1);
 init();equip();g.missiles=0;assert(!game_fire(&g,0,g.enemy.rooms[0].id)&&!g.weapons[0].pending_shots);
 init();equip();assert(game_set_bomb_target(&g,0,1,g.player.rooms[0].id));game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(!g.weapons[0].bomb_side&&!g.weapons[0].queued_bomb_side&&!g.weapons[0].queued_bomb);
 /* Enemy scheduler snapshots bomb payload and cannot spend again while pool is full. */
 init();equip();g.enemy_weapon=g.weapons[0];g.enemy_weapon.cooldown_ticks=1;g.enemy_missiles=2;for(i=0;i<GAME_MAX_EFFECTS;i++){g.effects[i].active=1;g.effects[i].duration=999;}tick(5);assert(g.enemy_missiles==1&&g.enemy_pending_shots==1);g.effects[0].active=0;tick(1);assert(g.effects[0].kind==EFFECT_BOMB&&g.effects[0].side==1&&!g.effects[0].bomb_friendly&&g.effects[0].crew_damage==30&&!g.enemy_pending_shots);tick(1);assert(g.enemy_missiles==0);tick(5);assert(g.enemy_missiles==0);
 puts("PASS: Small Bomb source payload, shield bypass, evasion, self targeting/friendly fire, queue snapshot, pause, bounded pool and both-side ammunition.");return 0;
}

