#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
static Game g;
static void setup(void){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0));g.ticks=GAME_HZ;g.teleporter[1].cooldown_ticks=0;}
int main(void){int i,n,old;
 /* More than two eligible crew must not deadlock a two-person launch. */
 setup();g.enemy_crew_count=4;
 for(i=1;i<4;i++){game_init_crew(&g,1,i,g.teleporter[1].room);g.enemy_crew[i].ship=1;}
 boarding_step(&g,0);n=0;for(i=1;i<4;i++)n+=g.enemy_crew[i].ship==0;assert(n==2);
 /* Healthy Zoltan can board despite having fewer than 90 hitpoints. */
 setup();g.enemy_crew_count=2;game_init_crew(&g,1,1,g.teleporter[1].room);assert(game_set_crew_species(&g,1,1,SPEC_ZOLTAN));g.enemy_crew[1].health=70;
 boarding_step(&g,0);assert(g.enemy_crew[1].ship==0);
 /* Home healing/repair decisions must survive the boarding stage. */
 setup();g.enemy_crew[0].health=20;g.enemy_crew[0].destination=g.enemy.systems[SYS_MEDBAY].room;old=g.enemy_crew[0].destination;
 g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=g.enemy.systems[SYS_SHIELDS].room;
 boarding_step(&g,0);assert(g.enemy_crew[0].destination==old);
 /* A boarder leaves a destroyed system for productive sabotage. */
 setup();g.teleporter[1].power=0;g.enemy_crew_count=1;g.enemy_crew[0].ship=0;
 old=g.player.systems[SYS_WEAPONS].room;g.enemy_crew[0].room=g.enemy_crew[0].destination=old;
 g.player.systems[SYS_WEAPONS].health=g.player.systems[SYS_WEAPONS].power=0;
 for(i=0;i<g.crew_count;i++){g.crew[i].room=g.crew[i].destination=g.pilot_room;}
 boarding_step(&g,0);assert(g.enemy_crew[0].destination!=old);
 /* Ordinary power API counts enemy drones in the reactor budget. */
 setup();g.teleporter[1].power=0;g.enemy_drone.power=2;g.enemy.reactor=2;
 for(i=0;i<GAME_SYSTEM_COUNT;i++)g.enemy.systems[i].power=0;
 assert(!boarding_power(&g,1,1));
 puts("PASS: bounded enemy parties, species health, preserved home orders, retargeting and reactor accounting.");return 0;
}
