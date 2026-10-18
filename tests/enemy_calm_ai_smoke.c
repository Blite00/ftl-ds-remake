/* V40 calm-ship CrewAI: bCalmShip detection, 99%/25% healing thresholds and
 * manning-task priorities: original pilot, shields, weapons, engines, with the V46
 * house rule swapping weapons ahead of shields (volleys need a gunner). */
#include "game.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
/* Same synthetic six-room ship as enemy_ai_smoke: rooms 0..4 hold systems 0..4, room 5 pilots. */
static void setup(int crew){int i,j;game_init(&g);game_begin_encounter(&g,ENEMY_REBEL,0,0);
 memset(&g.enemy,0,sizeof g.enemy);g.enemy.hull=20;g.enemy.reactor=16;g.enemy.oxygen=1000;g.enemy.room_count=6;
 for(i=0;i<6;i++){GameRoom*r=&g.enemy.rooms[i];r->id=i;r->w=2;r->h=1;r->system=i<5?i:-1;r->oxygen=1000;for(j=0;j<6;j++)g.enemy_adjacency[i][j]=i!=j;}
 for(i=0;i<GAME_SYSTEM_COUNT;i++){g.enemy.systems[i].room=i;g.enemy.systems[i].capacity=g.enemy.systems[i].health=2;}
 g.enemy_pilot_room=5;g.enemy_pilot_health=1;memset(&g.enemy_drone,0,sizeof g.enemy_drone);g.enemy_drone.room=-1;memset(&g.teleporter[1],0,sizeof g.teleporter[1]);memset(g.subsystems[1],0,sizeof g.subsystems[1]);
 g.enemy_crew_count=crew;for(i=0;i<crew;i++){Crew*c=&g.enemy_crew[i];memset(c,0,sizeof *c);c->ship=1;c->species=SPEC_HUMAN;c->health=100;c->room=c->destination=c->station=i==0?5:i==1?SYS_WEAPONS:SYS_ENGINES;}
 g.ticks=GAME_HZ;
}
int main(void){
 /* Calm: nothing wrong aboard. Each problem class clears it. */
 setup(3);assert(game_enemy_calm(&g));
 g.enemy.rooms[3].fires=1;assert(!game_enemy_calm(&g));g.enemy.rooms[3].fires=0;
 g.enemy.rooms[3].breaches=1;assert(!game_enemy_calm(&g));g.enemy.rooms[3].breaches=0;
 g.enemy.systems[SYS_ENGINES].health=1;assert(!game_enemy_calm(&g));g.enemy.systems[SYS_ENGINES].health=2;
 g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=4;assert(!game_enemy_calm(&g));g.crew[0].ship=0;
 g.enemy_pilot_health=0;assert(!game_enemy_calm(&g));g.enemy_pilot_health=1;assert(game_enemy_calm(&g));
 /* Calm ship: a lightly wounded (90%) crewmember heads to the medbay. */
 setup(3);g.enemy_crew[1].health=90;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);assert(g.enemy.systems[SYS_MEDBAY].power>0);
 /* Busy ship (fire elsewhere): the same crewmember keeps working. */
 setup(3);g.enemy_crew[1].health=90;g.enemy.rooms[SYS_OXYGEN].fires=1;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination!=SYS_MEDBAY);
 /* Below 25% heals regardless; a patient stays until 99%. */
 setup(3);g.enemy_crew[1].health=20;g.enemy.rooms[SYS_OXYGEN].fires=1;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);
 g.enemy_crew[1].room=SYS_MEDBAY;g.enemy_crew[1].health=98;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);
 /* Manning: three crew staff pilot, shields and weapons; engines goes unmanned. */
 setup(3);game_enemy_crew_orders(&g);assert(g.enemy_crew[0].station==5&&g.enemy_crew[2].station==SYS_SHIELDS&&g.enemy_crew[1].station==SYS_WEAPONS);
 assert(g.enemy_crew[2].destination==SYS_SHIELDS);
 /* Losing the pilot: the least important manned station (V46: shields 66) takes piloting (64). */
 g.enemy_crew[0].health=0;g.ticks=GAME_HZ;game_enemy_crew_orders(&g);assert(g.enemy_crew[2].station==5&&g.enemy_crew[1].station==SYS_WEAPONS);
 /* Four crew add engines; a fifth idles (no extra mannable console). */
 setup(5);game_enemy_crew_orders(&g);{int seen[6]={0};for(int i=0;i<5;i++)seen[g.enemy_crew[i].station]++;assert(seen[5]==1&&seen[SYS_SHIELDS]==1&&seen[SYS_WEAPONS]==1&&seen[SYS_ENGINES]>=1);}
 /* A patient's console is covered meanwhile and stations are stable on repeat. */
 setup(3);g.enemy_crew[2].health=10;game_enemy_crew_orders(&g);assert(g.enemy_crew[2].destination==SYS_MEDBAY&&g.enemy_crew[1].station==SYS_WEAPONS); /* V46: the gunner stays. */
 {Game before=g;g.ticks=GAME_HZ;game_enemy_crew_orders(&g);for(int i=0;i<3;i++)assert(g.enemy_crew[i].station==before.enemy_crew[i].station);}
 puts("PASS: calm-ship detection, 99%/25% healing thresholds, original manning priorities and reassignment.");return 0;
}
