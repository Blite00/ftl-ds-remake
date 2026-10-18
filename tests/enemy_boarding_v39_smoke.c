/* V39 original boarding AI: party size/stay-behind, two-send limit, fighter
 * preference, uniform landing rooms, evacuation rules and grouped targets. */
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void game_enemy_crew_orders(Game*g);
static Game g;
static int pad(void){return g.teleporter[1].room;}
/* n healthy humans: crew 0 pilots, the rest stand on the charged pad. */
static void setup(int n){int i;game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0));assert(g.enemy_board_mode==1&&g.enemy_board_stay==-1&&!g.enemy_board_sends);
 g.ticks=GAME_HZ;g.teleporter[1].cooldown_ticks=0;g.enemy_crew_count=n;
 for(i=0;i<n;i++){game_init_crew(&g,1,i,i?pad():g.enemy_pilot_room);g.enemy_crew[i].ship=1;g.enemy_crew[i].station=i?pad():g.enemy_pilot_room;}
}
static int away(void){int i,n=0;for(i=0;i<g.enemy_crew_count;i++)n+=g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0;return n;}
static void tick(void){g.ticks=GAME_HZ;boarding_step(&g,0);}
int main(void){int i,rooms[GAME_MAX_ROOMS]={0},distinct=0;
 /* Sabotage: four crew send floor(4/2)=2 and keep 2 home; five send 2. */
 setup(4);assert(boarding_enemy_request(&g)==2&&g.enemy_board_stay==2);tick();assert(away()==2&&g.enemy_board_sends==1&&!boarding_enemy_request(&g));
 setup(5);assert(boarding_enemy_request(&g)==2&&g.enemy_board_stay==3);
 setup(3);assert(boarding_enemy_request(&g)==1);tick();assert(away()==1&&g.enemy_board_sends==1);
 /* Invasion (flagship mode) sends floor(3n/4). */
 setup(4);g.enemy_board_mode=2;assert(boarding_enemy_request(&g)==3);
 /* Two completed sends end teleporting, even after boarders return. */
 setup(3);tick();g.enemy_board_sends=2;for(i=0;i<3;i++)if(g.enemy_crew[i].ship==0){g.enemy_crew[i].ship=1;g.enemy_crew[i].room=g.enemy_crew[i].destination=pad();}
 assert(!boarding_enemy_request(&g));g.teleporter[1].cooldown_ticks=0;tick();assert(!away());
 /* Landing rooms are uniformly random, no longer always weapons. */
 for(unsigned seed=1;seed<=60;seed++){setup(3);g.rng=seed*2654435761u;tick();for(i=0;i<3;i++)if(g.enemy_crew[i].ship==0)for(int k=0;k<g.player.room_count;k++)if(g.player.rooms[k].id==g.enemy_crew[i].room&&!rooms[k]++)distinct++;}
 assert(distinct>=g.player.room_count/2);
 /* Evacuation: sabotage boarders below 25% return; healthier ones stay. */
 setup(3);tick();for(i=0;i<3;i++)if(g.enemy_crew[i].ship==0){int b=i;g.teleporter[1].cooldown_ticks=0;g.enemy_crew[b].health=30;tick();assert(g.enemy_crew[b].ship==0);g.enemy_crew[b].health=20;tick();assert(g.enemy_crew[b].ship==1);}
 /* Invasion boarders have no minimum health. */
 setup(3);tick();g.enemy_board_mode=2;for(i=0;i<3;i++)if(g.enemy_crew[i].ship==0){g.teleporter[1].cooldown_ticks=0;g.enemy_crew[i].health=5;tick();assert(g.enemy_crew[i].ship==0);}
 /* Forced: three completely broken systems recall a healthy boarder. */
 setup(3);tick();assert(away()==1);g.teleporter[1].cooldown_ticks=0;g.enemy.systems[SYS_ENGINES].health=0;g.enemy.systems[SYS_OXYGEN].health=0;tick();assert(away()==1);
 g.teleporter[1].cooldown_ticks=0;g.enemy.systems[SYS_MEDBAY].health=0;assert(boarding_enemy_evac_forced(&g));tick();assert(!away());
 /* Forced: escaping with under 15 s left. */
 setup(3);tick();g.teleporter[1].cooldown_ticks=0;g.enemy_escape_active=1;g.enemy_escape_ticks=20*GAME_HZ;tick();assert(away()==1);g.enemy_escape_ticks=14*GAME_HZ;tick();assert(!away());
 /* Losing home crew below the stay-behind count requests a boarder back. */
 setup(4);tick();assert(away()==2);g.teleporter[1].cooldown_ticks=0;for(i=0;i<4;i++)if(g.enemy_crew[i].ship==1&&i){g.enemy_crew[i].health=0;break;}
 assert(boarding_enemy_request(&g)<0);tick();assert(away()<2);
 /* Selection: fighters board ahead of bad fighters who are manning; the
  * pilot (64) is chosen after other stations (66-68). */
 setup(3);g.enemy_crew[1].station=g.enemy.systems[SYS_OXYGEN].room>=0?g.enemy.systems[SYS_OXYGEN].room:g.enemy.systems[SYS_ENGINES].room;g.enemy_crew[2].station=g.enemy.systems[SYS_WEAPONS].room;
 for(i=1;i<3;i++)g.enemy_crew[i].room=g.enemy_crew[i].destination=g.enemy_crew[i].station;
 {int current[3]={64,boarding_manning_priority(&g,&g.enemy_crew[1]),boarding_manning_priority(&g,&g.enemy_crew[2])};
  assert(current[2]==66&&current[1]>66);
  assert(boarding_enemy_party(&g,current,3)==(1u<<1));
  assert(game_set_crew_species(&g,1,1,SPEC_ENGI));assert(boarding_enemy_party(&g,current,3)==(1u<<2));
  assert(game_set_crew_species(&g,1,2,SPEC_ZOLTAN));assert(boarding_enemy_party(&g,current,3)==1u);
  /* Below half health never takes a teleport task while busy. */
  assert(game_set_crew_species(&g,1,1,SPEC_HUMAN));g.enemy_crew[1].health=40;assert(boarding_enemy_party(&g,current,3)!=(1u<<1));
  /* An uncharged teleporter (65) does not pull the pilot (64) or shields work. */
  g.enemy_crew[1].health=100;g.teleporter[1].cooldown_ticks=GAME_HZ;assert(boarding_enemy_party(&g,current,3)==(1u<<1));
  current[1]=1;assert(!(boarding_enemy_party(&g,current,3)&(1u<<1)));}
 /* Scheduler integration: the chosen crew walk to the pad. */
 setup(3);g.enemy_crew[1].station=g.enemy_crew[1].room=g.enemy_crew[1].destination=g.enemy.systems[SYS_WEAPONS].room;g.enemy_crew[2].station=g.enemy_crew[2].room=g.enemy_crew[2].destination=g.enemy.systems[SYS_ENGINES].room;
 /* V40 manning first moves engines (67) to the vacant shields console (65),
  * leaving weapons (66) as the least-busy crewmember to board. */
 game_enemy_crew_orders(&g);assert(g.enemy_crew[1].destination==pad()&&g.enemy_crew[2].station==g.enemy.systems[SYS_SHIELDS].room&&g.enemy_crew[0].destination!=pad());
 /* Grouped intruders: later boarders join the first boarder's room. */
 setup(4);tick();assert(away()==2);{int a=-1,b=-1;for(i=0;i<4;i++)if(g.enemy_crew[i].ship==0){if(a<0)a=i;else b=i;}
  for(i=0;i<g.crew_count;i++)g.crew[i].room=g.crew[i].destination=g.pilot_room;
  /* A: two-slot working system room (best possible score for a pair); B: an
   * adjacent room without a system. The later boarder takes A, since no
   * room is strictly better, rather than a random equally good one. */
  int A=-1,B=-1;for(int x=0;x<g.player.room_count&&B<0;x++){GameRoom*r=&g.player.rooms[x];if(r->system<0||r->w*r->h!=2||r->id==g.pilot_room)continue;
   for(int y=0;y<g.player.room_count;y++){GameRoom*q=&g.player.rooms[y];if(q->system<0&&q->id!=g.pilot_room&&game_next_room(&g,0,q->id,r->id)==r->id){A=r->id;B=q->id;break;}}}
  assert(A>=0&&B>=0);
  for(unsigned seed=1;seed<=20;seed++){g.rng=seed*2246822519u;
   g.enemy_crew[a].room=g.enemy_crew[a].destination=A;g.enemy_crew[b].room=g.enemy_crew[b].destination=B;
   tick();assert(g.enemy_crew[a].destination==A&&g.enemy_crew[b].destination==A);}}
 puts("PASS: V39 sabotage/invasion party sizes, two-send limit, uniform landing, evacuation triggers, fighter selection and grouped intruders.");return 0;
}
