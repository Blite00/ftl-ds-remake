/* Focused door/oxygen smoke; compile with source/game.c and -Igenerated -Isource. */
#include "game.h"
#include <assert.h>
#include <stdio.h>
static void advance(Game *g,int n){while(n-->0)game_step(g);}
static void quiet(Game *g){game_init(g);g->enemy.systems[SYS_WEAPONS].health=0;g->enemy.systems[SYS_WEAPONS].capacity=0;g->enemy.systems[SYS_WEAPONS].power=0;}
static int total(const Game *g){int sum=0,i;for(i=0;i<g->player.room_count;i++){const GameRoom *r=&g->player.rooms[i];sum+=r->oxygen*r->w*r->h;}return sum;}
int main(void){
 Game g;int i,airlocks=0,before,ticks,area=0;
 quiet(&g);assert(g.door_count==26);
 for(i=0;i<g.door_count;i++){assert(!g.doors[i].open);if(g.doors[i].room_a<0||g.doors[i].room_b<0)airlocks++;}
 assert(airlocks==6);
 g.crew[0].room=g.crew[0].destination=16;
 g.crew[1].room=g.crew[1].destination=0;
 assert(game_toggle_door(&g,20));assert(g.player.rooms[16].exposed);
 advance(&g,120);
 assert(g.player.rooms[16].oxygen==0);
 assert(g.player.rooms[14].oxygen==1000&&!g.player.rooms[14].exposed);
 assert(g.crew[0].health<100&&g.crew[1].health==100&&g.crew[2].health==100);
 assert(game_toggle_door(&g,18));assert(g.player.rooms[14].exposed);
 advance(&g,60);assert(g.player.rooms[14].oxygen<1000);
 game_toggle_pause(&g);before=total(&g);ticks=g.ticks;
 advance(&g,60);assert(g.ticks==ticks&&total(&g)==before);
 game_close_all_doors(&g);assert(!g.player.rooms[16].exposed&&!g.player.rooms[14].exposed);
 advance(&g,60);assert(total(&g)==before);
 game_toggle_pause(&g);advance(&g,60);assert(total(&g)>before);
 for(i=0;i<g.player.room_count;i++)area+=g.player.rooms[i].w*g.player.rooms[i].h;
 assert(g.player.oxygen==total(&g)/area);
 /* Exchange preserves area-weighted oxygen apart from uniform O2 loss. */
 quiet(&g);g.player.rooms[16].oxygen=0;
 assert(game_adjust_power(&g,SYS_OXYGEN,-1));assert(game_toggle_door(&g,18));
 before=total(&g);game_step(&g);assert(total(&g)==before-area);
 /* A crew crossing opens a closed door briefly, preserving manual openings. */
 quiet(&g);g.crew[0].room=g.crew[0].destination=0;
 assert(game_order_crew(&g,0,1));advance(&g,GAME_HZ);
 assert(g.crew[0].room==1&&g.doors[0].open&&g.doors[0].transit_ticks==15);
 advance(&g,15);assert(!g.doors[0].open);
 assert(game_toggle_door(&g,0));assert(game_order_crew(&g,0,0));advance(&g,GAME_HZ+15);
 assert(g.doors[0].open&&g.doors[0].transit_ticks==0);
 puts("PASS: airlock isolation, spread, local suffocation, reseal/refill, pause, weighted exchange and crew doors.");
 return 0;
}
