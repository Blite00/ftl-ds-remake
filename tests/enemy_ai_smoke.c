#include "game.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void game_enemy_crew_orders(Game*g);
static Game g;
static void setup(void){int i,j;game_init(&g);game_begin_encounter(&g,ENEMY_REBEL,0,0);
 memset(&g.enemy,0,sizeof g.enemy);g.enemy.hull=20;g.enemy.reactor=16;g.enemy.oxygen=1000;g.enemy.room_count=6;
 for(i=0;i<6;i++){GameRoom*r=&g.enemy.rooms[i];r->id=i;r->w=2;r->h=1;r->system=i<5?i:-1;r->oxygen=1000;for(j=0;j<6;j++)g.enemy_adjacency[i][j]=i!=j;}
 for(i=0;i<GAME_SYSTEM_COUNT;i++){g.enemy.systems[i].room=i;g.enemy.systems[i].capacity=g.enemy.systems[i].health=2;}
 g.enemy_pilot_room=5;g.enemy_pilot_health=1;memset(&g.enemy_drone,0,sizeof g.enemy_drone);g.enemy_drone.room=-1;memset(&g.teleporter[1],0,sizeof g.teleporter[1]);
 g.enemy_crew_count=3;for(i=0;i<3;i++){Crew*c=&g.enemy_crew[i];memset(c,0,sizeof *c);c->ship=1;c->species=SPEC_HUMAN;c->health=100;c->room=c->destination=c->station=i==0?5:i==1?2:0;}
 g.ticks=GAME_HZ;
}
static int assigned(int room){int i,n=0;for(i=0;i<g.enemy_crew_count;i++)n+=g.enemy_crew[i].destination==room;return n;}
int main(void){int i,budget;
 setup();{Game snapshot;g.paused=1;snapshot=g;game_enemy_crew_orders(&g);assert(!memcmp(&g,&snapshot,sizeof g));g.paused=0;g.outcome=GAME_VICTORY;snapshot=g;game_enemy_crew_orders(&g);assert(!memcmp(&g,&snapshot,sizeof g));}
 setup();game_set_crew_species(&g,1,1,SPEC_ZOLTAN);g.enemy_crew[1].health=17;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);
 g.enemy_crew[1].room=SYS_MEDBAY;g.enemy_crew[1].health=68;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);
 g.enemy_crew[1].health=70;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==g.enemy_crew[1].station);
 setup();game_set_crew_species(&g,1,1,SPEC_ROCK);g.enemy_crew[1].health=37;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);
 setup();g.enemy_crew[1].health=24;g.enemy.rooms[SYS_MEDBAY].oxygen=0;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination!=SYS_MEDBAY);
 setup();g.enemy_crew[1].health=24;for(i=0;i<6;i++)g.enemy_adjacency[i][SYS_MEDBAY]=g.enemy_adjacency[SYS_MEDBAY][i]=0;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination!=SYS_MEDBAY);
 setup();g.enemy.systems[SYS_SHIELDS].health=0;g.enemy.systems[SYS_WEAPONS].health=0;game_enemy_crew_orders(&g);assert(g.enemy_crew[2].destination==SYS_SHIELDS);assert(g.enemy_crew[0].destination==5);assert(assigned(SYS_SHIELDS)==1);
 setup();g.enemy.systems[SYS_SHIELDS].health=0;g.enemy.systems[SYS_OXYGEN].health=0;g.enemy.oxygen=200;game_enemy_crew_orders(&g);assert(assigned(SYS_OXYGEN)==1);assert(g.enemy_crew[0].destination==5);
 setup();g.enemy_crew[1].health=20;g.crew[0].ship=1;g.crew[0].room=SYS_WEAPONS;g.enemy.systems[SYS_SHIELDS].health=0;game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==SYS_MEDBAY);assert(assigned(SYS_SHIELDS)==1);assert(assigned(SYS_WEAPONS)==1);
 setup();g.enemy.rooms[SYS_MEDBAY].w=1;for(i=0;i<3;i++)g.enemy_crew[i].health=20;game_enemy_crew_orders(&g);assert(assigned(SYS_MEDBAY)==1);
 for(budget=0;budget<16;budget++){int used;setup();g.enemy.reactor=budget;g.enemy_crew[1].health=20;g.enemy_drone.health=3;g.teleporter[1].installed=1;g.teleporter[1].health=2;g.enemy.systems[SYS_SHIELDS].bonus_power=1;game_enemy_crew_orders(&g);used=g.enemy_drone.power+g.teleporter[1].power;for(i=0;i<GAME_SYSTEM_COUNT;i++){assert(g.enemy.systems[i].power>=0);assert(g.enemy.systems[i].power+g.enemy.systems[i].bonus_power<=g.enemy.systems[i].health);used+=g.enemy.systems[i].power;}assert(used<=budget);}
 setup();g.teleporter[1].installed=1;g.teleporter[1].health=g.teleporter[1].capacity=1;g.teleporter[1].room=3;g.enemy_board_mode=1;g.enemy_board_stay=-1;g.enemy.systems[SYS_SHIELDS].health=0;game_enemy_crew_orders(&g);assert(assigned(SYS_SHIELDS)==1);assert(g.enemy_crew[1].destination==3);assert(g.enemy_crew[2].destination==SYS_SHIELDS);assert(g.enemy_crew[0].destination==5);
 puts("PASS: species-relative healing, safe accessible beds, prioritized bounded repair/defense, teleporter staging and reactor budgets.");return 0;
}
