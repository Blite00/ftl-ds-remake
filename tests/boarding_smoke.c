#include "boarding.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void setup(void){game_init(&g);game_begin_encounter(&g,ENEMY_REBEL,0,0);g.enemy_weapon.shots=0;g.enemy_weapon.cooldown_ticks=999999;assert(boarding_install(&g,0));g.player.reactor=20;assert(boarding_power(&g,0,1));}
static void gather(unsigned mask){int i;for(i=0;i<g.crew_count;i++)if(mask&(1u<<i)){g.crew[i].room=g.crew[i].destination=g.teleporter[0].room;}}
static void tick(int n){while(n--)game_step(&g);}
int main(void){int i,j,room;unsigned id;
 setup();id=g.crew[1].id;assert(!boarding_deploy(&g,2u,g.enemy_pilot_room));gather(2u);
 assert(boarding_deploy(&g,2u,g.enemy_pilot_room));assert(game_away_crew(&g)==1&&g.crew[1].id==id);
 assert(!boarding_recall(&g,2u));g.teleporter[0].cooldown_ticks=0;assert(boarding_recall(&g,2u));assert(!game_away_crew(&g));
 gather(7u);g.teleporter[0].cooldown_ticks=0;assert(!boarding_deploy(&g,7u,g.enemy_pilot_room));
 g.teleporter[0].health=0;assert(!boarding_deploy(&g,2u,g.enemy_pilot_room));g.teleporter[0].health=1;
 g.crew[1].health=0;assert(!boarding_deploy(&g,2u,g.enemy_pilot_room));
 /* Melee exchanges are simultaneous and faction-specific. */
 setup();gather(2u);room=g.enemy_crew[0].room;assert(boarding_deploy(&g,2u,room));
 g.crew[1].combat_ticks=g.enemy_crew[0].combat_ticks=GAME_HZ-1;
 i=g.enemy_crew[0].health;boarding_step(&g,0);assert(g.crew[1].health==94&&g.enemy_crew[0].health==i-6);
 g.paused=1;snapshot=g;boarding_step(&g,0);assert(!memcmp(&snapshot,&g,sizeof g));g.paused=0;
 /* Sabotage cannot repair or operate opposing systems. */
 setup();gather(2u);room=g.enemy.systems[SYS_SHIELDS].room;assert(boarding_deploy(&g,2u,room));
 for(i=0;i<g.enemy_crew_count;i++){g.enemy_crew[i].room=g.enemy_crew[i].destination=g.enemy_pilot_room;}
 for(i=0;i<5*GAME_HZ;i++){g.ticks=1;boarding_step(&g,0);}assert(g.enemy.systems[SYS_SHIELDS].health==1&&g.enemy.shields==0);
 /* Captured-ship service cools teleporters and permits recall without simulating enemy. */
 g.outcome=GAME_VICTORY;g.result_reason=RESULT_ENEMY_CREW;
 for(i=0;i<BOARDING_COOLDOWN;i++)game_service_step(&g);
 assert(boarding_recall(&g,2u)&&g.crew[1].ship==0);
 /* Destruction kills aboard occupants, not hostiles already on the surviving ship. */
 setup();gather(2u);assert(boarding_deploy(&g,2u,g.enemy_pilot_room));
 g.enemy_crew[0].ship=0;g.enemy_crew[0].room=g.enemy_crew[0].destination=g.player.systems[SYS_WEAPONS].room;
 g.enemy.hull=0;game_step(&g);assert(g.crew[1].health==0&&g.enemy_crew[0].health>0&&g.outcome==GAME_PLAYING);
 g.enemy_crew[0].health=0;game_step(&g);assert(g.outcome==GAME_VICTORY&&g.result_reason==RESULT_ENEMY_HULL);
 boarding_prepare_departure(&g);assert(g.crew[1].ship==0&&g.crew[1].health==0&&g.crew[1].id==id);
 setup();gather(2u);assert(boarding_deploy(&g,2u,g.enemy_pilot_room));assert(game_abandon_crew(&g)==1);boarding_prepare_departure(&g);assert(g.crew[1].health==0&&g.crew[1].ship==0);
 /* Enemy uses real paths into teleporter and launches a party. */
 setup();game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0);g.enemy_weapon.shots=0;
 {int away=0;for(j=0;j<20*GAME_HZ&&!away;j++){tick(1);for(i=0;i<g.enemy_crew_count;i++)away+=g.enemy_crew[i].ship==0&&g.enemy_crew[i].health>0;}assert(away>0);}
 /* Repair may restore a system without allocating more than the reactor allows. */
 setup();game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0);g.enemy_weapon.shots=0;
 g.enemy.systems[SYS_OXYGEN].health=g.enemy.systems[SYS_OXYGEN].power=0;
 for(j=0;j<12*GAME_HZ;j++){int used=g.teleporter[1].power;tick(1);for(i=0;i<GAME_SYSTEM_COUNT;i++)used+=g.enemy.systems[i].power;assert(used<=g.enemy.reactor);}
 puts("PASS: two-way boarding, cooldown, party validation, melee, sabotage, recall, abandonment and destruction invariants.");return 0;
}

