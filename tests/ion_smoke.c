#include "game.h"
#include "species.h"
#include "subsystems.h"
#include "boarding.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void init(void){game_init(&g);g.pilot_health=0;g.player.systems[SYS_SHIELDS].power=0;g.player.shields=0;g.enemy_weapon.cooldown_ticks=999999;g.enemy_weapon.shots=0;}
static void tick(int n){while(n--)game_step(&g);}
static void ion(int side,int room,int amount){assert(game_launch_ion(&g,side,room,amount));tick(12);}
int main(void){int room,i,power;
 init();assert(game_equip(&g,0,4));assert(g.weapons[0].damage==0&&g.weapons[0].system_damage==0&&g.weapons[0].crew_damage==0&&g.weapons[0].ion_damage==1&&g.weapons[0].cooldown_ticks==8*GAME_HZ&&g.weapons[0].power==1);
 snapshot=g;assert(!game_launch_ion(&g,2,0,1)&&!game_launch_ion(&g,1,0,0)&&!game_launch_ion(&g,1,0,9));assert(!memcmp(&snapshot,&g,sizeof g));
 /* Shield interception redirects ion to shields, with no physical damage. */
 init();g.player.systems[SYS_SHIELDS].power=2;g.player.shields=1;room=g.player.systems[SYS_WEAPONS].room;
 ion(1,room,1);assert(g.player.hull==30&&g.player.systems[SYS_SHIELDS].health==2&&g.player.systems[SYS_SHIELDS].ion_bars==1&&g.player.systems[SYS_SHIELDS].ion_ticks==GAME_ION_TICKS&&!g.player.systems[SYS_WEAPONS].ion_bars&&g.player.shields==0);
 assert(g.crew[2].health==100);power=g.player.systems[SYS_SHIELDS].power;
 g.paused=1;snapshot=g;game_step(&g);assert(!memcmp(&snapshot,&g,sizeof g));g.paused=0;
 for(i=0;i<GAME_ION_TICKS-1;i++)game_service_step(&g);
 assert(g.player.systems[SYS_SHIELDS].ion_ticks==1&&game_shield_capacity(&g.player)==0);
 game_service_step(&g);assert(!g.player.systems[SYS_SHIELDS].ion_bars&&g.player.systems[SYS_SHIELDS].power==power&&game_shield_capacity(&g.player)==1);
 /* Ion follows normal evasion and a miss creates no lock. */
 init();g.pilot_health=1;assert(game_evasion(&g)>0);
 for(i=0;i<10000;i++)if((((unsigned)i*1664525u+1013904223u)>>16)%100u<(unsigned)game_evasion(&g)){g.rng=(unsigned)i;break;}
 assert(i<10000);ion(1,g.player.systems[SYS_WEAPONS].room,1);assert(g.effects[0].result==EFFECT_MISS&&!g.player.systems[SYS_WEAPONS].ion_bars);
 /* Repeated hits stack bars and remaining duration, bounded independently. */
 init();room=g.player.systems[SYS_WEAPONS].room;ion(1,room,1);ion(1,room,1);
 assert(g.player.systems[SYS_WEAPONS].ion_bars==2&&g.player.systems[SYS_WEAPONS].ion_ticks==2*GAME_ION_TICKS-12&&game_system_power(&g.player,SYS_WEAPONS)==1);
 ion(1,room,8);assert(g.player.systems[SYS_WEAPONS].ion_bars==8&&g.player.systems[SYS_WEAPONS].ion_ticks==GAME_ION_MAX_TICKS&&game_system_power(&g.player,SYS_WEAPONS)==0);
 /* Own status survives a jump; the newly initialized enemy has no old locks. */
 game_begin_encounter(&g,ENEMY_REBEL,0,0);assert(g.player.systems[SYS_WEAPONS].ion_bars==8);for(i=0;i<GAME_SYSTEM_COUNT;i++)assert(!g.enemy.systems[i].ion_bars);
 /* Teleporter, sensor, door, pilot and drone consumers respect ion state. */
 init();assert(boarding_install(&g,0));g.player.reactor=20;assert(boarding_power(&g,0,1));ion(1,g.teleporter[0].room,1);assert(!species_teleporter_power(&g,0));
 ion(1,g.subsystems[0][SUB_SENSORS].room,1);assert(!subsystem_level(&g,0,SUB_SENSORS));ion(1,g.subsystems[0][SUB_DOORS].room,1);assert(!game_toggle_door(&g,0));
 ion(1,g.pilot_room,1);assert(g.pilot_ion_ticks[0]>0);
 init();game_begin_encounter(&g,ENEMY_ASSAULT,0,0);g.enemy.systems[SYS_ENGINES].power=0;g.enemy.systems[SYS_SHIELDS].power=0;g.enemy.shields=0;
 ion(0,g.enemy_drone.room,1);assert(g.enemy_drone.ion_bars==1);g.enemy_drone.charge_ticks=7;tick(1);assert(g.enemy_drone.charge_ticks==0);
 /* Zoltan power remains available while normal allocated bars are disabled. */
 init();game_set_crew_species(&g,0,2,SPEC_ZOLTAN);room=g.crew[2].room;assert(g.player.systems[SYS_WEAPONS].bonus_power==1);ion(1,room,8);assert(game_system_power(&g.player,SYS_WEAPONS)==1);
 /* Firing snapshots ion amount before any later weapon configuration changes. */
 init();assert(game_equip(&g,0,4));g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;g.player.systems[SYS_WEAPONS].power=3;assert(game_fire(&g,0,g.enemy.rooms[0].id));g.weapons[0].ion_damage=8;tick(1);
 {int found=0;for(i=0;i<GAME_MAX_EFFECTS;i++)if(g.effects[i].active&&g.effects[i].side==0){assert(g.effects[i].kind==EFFECT_ION&&g.effects[i].ion_damage==1&&!g.effects[i].damage);found++;}assert(found==1);}
 /* Enemy weapon scheduling uses the same ion payload path. */
 init();g.enemy_weapon.damage=g.enemy_weapon.system_damage=g.enemy_weapon.crew_damage=0;g.enemy_weapon.ion_damage=1;g.enemy_weapon.shots=1;g.enemy_weapon.cooldown_ticks=1;tick(1);
 {int found=0;for(i=0;i<GAME_MAX_EFFECTS;i++)if(g.effects[i].active&&g.effects[i].side==1){assert(g.effects[i].kind==EFFECT_ION&&g.effects[i].ion_damage==1);found++;}assert(found==1);}
 puts("PASS: Ion Blast catalog, shield interception, bounded stacking/recovery, pause/service, all power consumers, Zoltan bonus and both-side queued ion volleys.");return 0;
}
