/* Partition/transition/service fixtures; no player retreat UI is claimed. */
#include "game.h"
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void fresh(void){game_init(&g);game_begin_encounter(&g,ENEMY_REBEL,0,0);}
static int cell_room(int cell){for(int r=0;r<g.player.room_count;r++){int n=g.player.rooms[r].w*g.player.rooms[r].h;if(cell<n)return g.player.rooms[r].id;cell-=n;}assert(0);return -1;}
static void home(int index,int cell){game_init_crew(&g,1,index,cell_room(cell));g.enemy_crew[index].ship=0;}
int main(void){
 assert(GAME_MAX_CREW==8&&GAME_MAX_ENEMY_CREW==12&&GAME_MAX_INTRUDERS==48&&GAME_MAX_HOSTILES==60);fresh();int cells=0;for(int i=0;i<g.player.room_count;i++)cells+=g.player.rooms[i].w*g.player.rooms[i].h;assert(cells==48);
 /* Six full groups fit the actual Kestrel floor capacity, preserving every byte. */
 Crew expected[48];
 for(int group=0;group<6;group++){
  for(int i=0;i<8;i++){int n=group*8+i;home(i,n);Crew*c=&g.enemy_crew[i];c->health=50+n%40;c->species=n%SPEC_COUNT;c->move_ticks=n%10;c->combat_ticks=n%GAME_HZ;c->sabotage_ticks=n%GAME_HZ;c->ability_ticks=7;c->skill[0]=n;snprintf(c->name,sizeof c->name,"Intruder%u",(unsigned)n%48);expected[n]=*c;}
  if(g.enemy_crew_count<8)g.enemy_crew_count=8;
  assert(game_carry_intruders(&g));assert(g.enemy_crew_count==GAME_MAX_ENEMY_CREW+(group+1)*8);assert(!game_alive_enemy_crew(&g));
  for(int n=0;n<(group+1)*8;n++)assert(!memcmp(&g.enemy_crew[n+GAME_MAX_ENEMY_CREW],&expected[n],sizeof(Crew)));
  game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0);assert(game_alive_enemy_crew(&g)==3);
  for(int n=0;n<(group+1)*8;n++)assert(!memcmp(&g.enemy_crew[n+GAME_MAX_ENEMY_CREW],&expected[n],sizeof(Crew)));
 }
 assert(g.enemy_crew_count==GAME_MAX_HOSTILES);for(int i=0;i<GAME_MAX_HOSTILES;i++)for(int j=i+1;j<GAME_MAX_HOSTILES;j++)if(g.enemy_crew[i].id&&g.enemy_crew[j].id)assert(g.enemy_crew[i].id!=g.enemy_crew[j].id);
 /* Defensive invalid 49th-intruder fixture fails atomically, without truncation. */
 home(0,0);Game before=g;assert(!game_carry_intruders(&g)&&!memcmp(&before,&g,sizeof g));
 /* Pending deaths cannot be overwritten; processed dead slots can be reused. */
 g.enemy_crew[GAME_MAX_HOSTILES-1].health=0;g.enemy_crew[GAME_MAX_HOSTILES-1].species=SPEC_ZOLTAN;g.enemy_crew[GAME_MAX_HOSTILES-1].death_processed=0;before=g;assert(!game_carry_intruders(&g)&&!memcmp(&before,&g,sizeof g));g.enemy_crew[GAME_MAX_HOSTILES-1].death_processed=1;Crew carried=g.enemy_crew[0];assert(game_carry_intruders(&g)&&!memcmp(&carried,&g.enemy_crew[GAME_MAX_HOSTILES-1],sizeof carried));
 g.enemy_crew[GAME_MAX_HOSTILES-1].ship=1;before=g;assert(!game_carry_intruders(&g)&&!memcmp(&before,&g,sizeof g));int oldkind=g.enemy_kind;game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_kind==oldkind&&g.enemy_crew[GAME_MAX_HOSTILES-1].ship==1);
 /* Current boarder guard remains; explicit promotion makes a new encounter valid. */
 fresh();home(1,0);oldkind=g.enemy_kind;game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_kind==oldkind);assert(game_carry_intruders(&g));game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_kind==ENEMY_SCOUT&&g.enemy_crew_count==GAME_MAX_ENEMY_CREW+1&&g.enemy_crew[GAME_MAX_ENEMY_CREW].health>0&&!game_alive_enemy_crew(&g));
 /* A pending local Zoltan death survives both ownership promotion and arrival. */
 fresh();home(1,0);g.enemy_crew[1].species=SPEC_ZOLTAN;g.enemy_crew[1].health=0;g.enemy_crew[1].death_processed=0;oldkind=g.enemy_kind;game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_kind==oldkind);
 Crew pending=g.enemy_crew[1];assert(game_carry_intruders(&g));assert(!memcmp(&pending,&g.enemy_crew[GAME_MAX_ENEMY_CREW],sizeof pending));game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_crew_count==GAME_MAX_ENEMY_CREW+1&&!memcmp(&pending,&g.enemy_crew[GAME_MAX_ENEMY_CREW],sizeof pending));g.crew[1].room=g.crew[1].destination=pending.room;int before_burst=g.crew[1].health;game_service_step(&g);assert(g.crew[1].health==before_burst-15&&g.enemy_crew[GAME_MAX_ENEMY_CREW].death_processed);game_service_step(&g);assert(g.crew[1].health==before_burst-15);
 /* Carried intruders never inherit the replacement enemy teleporter. */
 fresh();home(1,0);assert(game_carry_intruders(&g));game_begin_encounter(&g,ENEMY_REBEL_BOARDER,0,0);g.enemy_crew[GAME_MAX_ENEMY_CREW].health=20;g.teleporter[1].cooldown_ticks=0;g.ticks=GAME_HZ-1;game_step(&g);assert(g.enemy_crew[GAME_MAX_ENEMY_CREW].ship==0&&g.enemy_crew[GAME_MAX_ENEMY_CREW].health>0);
 /* Slot55 moves, fights and sabotages at peaceful service checkpoints. Remote
  * current enemy crew remain frozen and cannot receive service melee damage. */
 fresh();memset(g.enemy_crew,0,sizeof g.enemy_crew);g.enemy_crew_count=GAME_MAX_HOSTILES;home(GAME_MAX_HOSTILES-1,0);int start=g.enemy_crew[GAME_MAX_HOSTILES-1].room,next=-1;for(int r=0;r<g.player.room_count;r++)if(g.player.rooms[r].id!=start&&game_next_room(&g,0,start,g.player.rooms[r].id)>=0){next=g.player.rooms[r].id;break;}assert(next>=0);g.enemy_crew[GAME_MAX_HOSTILES-1].destination=next;
 game_init_crew(&g,1,0,g.enemy_pilot_room);Crew remote=g.enemy_crew[0];
 for(int i=0;i<150&&g.enemy_crew[GAME_MAX_HOSTILES-1].room==start;i++)game_service_step(&g);
 assert(g.enemy_crew[GAME_MAX_HOSTILES-1].room!=start&&!memcmp(&remote,&g.enemy_crew[0],sizeof remote));
 int room=g.player.systems[SYS_WEAPONS].room;g.enemy_crew[GAME_MAX_HOSTILES-1].room=g.enemy_crew[GAME_MAX_HOSTILES-1].destination=room;g.enemy_crew[GAME_MAX_HOSTILES-1].sabotage_ticks=5*GAME_HZ-1;for(int i=0;i<g.crew_count;i++)g.crew[i].room=g.crew[i].destination=g.pilot_room;int health=g.player.systems[SYS_WEAPONS].health;game_service_step(&g);assert(g.player.systems[SYS_WEAPONS].health==health-1);
 g.crew[1].room=g.crew[1].destination=room;g.crew[1].combat_ticks=GAME_HZ-1;g.enemy_crew[GAME_MAX_HOSTILES-1].combat_ticks=GAME_HZ-1;int ally=g.crew[1].health,hostile=g.enemy_crew[GAME_MAX_HOSTILES-1].health;game_service_step(&g);assert(g.crew[1].health<ally&&g.enemy_crew[GAME_MAX_HOSTILES-1].health<hostile&&!memcmp(&remote,&g.enemy_crew[0],sizeof remote));
 g.paused=1;before=g;game_service_step(&g);assert(!memcmp(&before,&g,sizeof g));
 puts("Carried intruders: 48-cell capacity, exact Crew preservation, atomic failure, pending deaths, partition IDs/ownership/teleport and slot55 service movement/melee/sabotage passed.");
}
