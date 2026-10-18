/* Focused medbay smoke: source/game.c with -Isource -Igenerated. */
#include "game.h"
#include <assert.h>
#include <stdio.h>
static void advance(Game *g,int n){while(n-->0)game_step(g);}
static void quiet(Game *g){game_init(g);g->enemy.systems[SYS_WEAPONS].health=0;g->enemy.systems[SYS_WEAPONS].capacity=0;g->enemy.systems[SYS_WEAPONS].power=0;}
static void power_medbay(Game *g){assert(game_adjust_power(g,SYS_ENGINES,-1));assert(game_adjust_power(g,SYS_MEDBAY,1));}
int main(void){
 Game g;int hp,partial;
 quiet(&g);
 assert(GAME_SYSTEM_COUNT==5&&SYS_MEDBAY==4);
 assert(g.player.systems[SYS_MEDBAY].room==4&&g.player.systems[SYS_MEDBAY].capacity==1);
 assert(g.player.systems[SYS_MEDBAY].health==1&&g.player.systems[SYS_MEDBAY].power==0);
 assert(game_used_power(&g)==8&&g.player.shields==1);
 assert(!game_adjust_power(&g,SYS_MEDBAY,1));power_medbay(&g);
 assert(game_used_power(&g)==8);
 g.crew[0].room=g.crew[0].destination=4;g.crew[0].health=50;
 g.crew[1].health=50;
 assert(game_crew_healing(&g,0)&&!game_crew_healing(&g,1));
 advance(&g,GAME_HZ);assert(g.crew[0].health==56&&g.crew[1].health==50);
 advance(&g,1);hp=g.crew[0].health;partial=g.crew[0].heal_accumulator;
 game_toggle_pause(&g);advance(&g,60);
 assert(g.crew[0].health==hp&&g.crew[0].heal_accumulator==partial);
 game_toggle_pause(&g);
 assert(game_adjust_power(&g,SYS_MEDBAY,-1));assert(!g.crew[0].heal_accumulator);
 advance(&g,60);assert(g.crew[0].health==hp&&!game_crew_healing(&g,0));
 assert(game_adjust_power(&g,SYS_MEDBAY,1));
 assert(game_order_crew(&g,0,5));advance(&g,GAME_HZ);
 assert(g.crew[0].health==hp&&!g.crew[0].heal_accumulator);
 assert(game_order_crew(&g,0,4));advance(&g,GAME_HZ);
 assert(g.crew[0].room==4);
 g.player.systems[SYS_MEDBAY].health=0;g.player.systems[SYS_MEDBAY].power=0;
 advance(&g,5*GAME_HZ);
 assert(g.player.systems[SYS_MEDBAY].health==1);
 assert(g.crew[0].health==hp&&!g.crew[0].heal_accumulator);
 assert(game_adjust_power(&g,SYS_MEDBAY,1));advance(&g,GAME_HZ);assert(g.crew[0].health==hp+6);
 g.crew[0].health=99;advance(&g,GAME_HZ);assert(g.crew[0].health==100&&!g.crew[0].heal_accumulator);
 g.crew[0].health=0;advance(&g,GAME_HZ);assert(g.crew[0].health==0&&!game_crew_healing(&g,0));
 /* Death during this exact suffocation tick cannot be undone by healing. */
 quiet(&g);power_medbay(&g);g.crew[0].room=g.crew[0].destination=4;
 g.crew[0].health=1;g.crew[0].heal_accumulator=GAME_HZ-1;
 g.player.rooms[4].oxygen=0;g.ticks=GAME_HZ-1;game_step(&g);
 assert(g.crew[0].health==0&&!g.crew[0].heal_accumulator);
 puts("PASS: medbay power, room healing, pause, depower, departure, repair, HP cap and no resurrection.");
 return 0;
}
