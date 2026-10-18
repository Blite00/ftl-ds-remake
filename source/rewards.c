#include "rewards.h"
#include "species.h"
#include "boarding.h"
#include <stdio.h>
#include <string.h>
static int game_valid(const Game*g){return g&&g->outcome!=GAME_DEFEAT&&g->player.hull>0&&g->crew_count>=1&&g->crew_count<=GAME_MAX_CREW&&g->enemy_crew_count>=0&&g->enemy_crew_count<=GAME_MAX_HOSTILES&&g->player.room_count>=1&&g->player.room_count<=GAME_MAX_ROOMS&&game_alive_crew(g)>0;}
static int id_used(const Game*g,unsigned id){for(int i=0;i<g->crew_count;i++)if(g->crew[i].id==id)return 1;for(int i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].id==id)return 1;return 0;}
int rewards_offer_weapon(PendingReward*r,int catalog){
 if(!r||r->kind!=REWARD_NONE||!game_weapon_player(catalog))return 0;
 *r=(PendingReward){.kind=REWARD_WEAPON,.weapon=catalog};return 1;
}
int rewards_offer_crew(PendingReward*r,Game*g,int species){
 static const char*const names[]={"Alex","Sam","Jordan","Morgan","Casey","Robin","Taylor","Riley","Avery","Blake","Quinn","Reese"};
 if(!r||r->kind!=REWARD_NONE||!game_valid(g)||species<0||species>=SPEC_COUNT||!g->next_crew_id||g->next_crew_id>=REWARD_CREW_ID_LIMIT||id_used(g,g->next_crew_id))return 0;
 PendingReward staged={.kind=REWARD_CREW,.weapon=-1};Crew*c=&staged.recruit;
 c->id=g->next_crew_id;c->species=species;c->health=species_max_health(species);c->room=c->destination=c->station=-1;
 snprintf(c->name,sizeof c->name,"%s",names[(c->id-1)%12]);
 g->next_crew_id++;*r=staged;return 1;
}
static int reusable(const Crew*c){return c->ship==0&&c->health<=0&&(!c->id||c->death_processed);}
static int recruit_room(const Game*g,int replacing){
 int preferred=g->player.systems[SYS_MEDBAY].room;
 for(int pass=-1;pass<g->player.room_count;pass++){
  int candidate=pass<0?preferred:g->player.rooms[pass].id,area=0,n=0;
  for(int j=0;j<g->player.room_count;j++)if(g->player.rooms[j].id==candidate){const GameRoom*r=&g->player.rooms[j];if(r->w<1||r->h<1||r->w>32||r->h>32)return -1;area=r->w*r->h;break;}
  for(int i=0;i<g->crew_count;i++)if(i!=replacing&&g->crew[i].ship==0&&g->crew[i].health>0&&g->crew[i].room==candidate)n++;
  if(n<area)return candidate;
 }
 return -1;
}
static int plan(const PendingReward*r,const Game*g,const int*cargo,int count,int slot,int replace,int*room){
 if(!r||!game_valid(g)||(replace!=0&&replace!=1)||slot< -1||(slot==-1&&replace))return -1;
 if(r->kind==REWARD_WEAPON){
  if(!cargo||count<1||count>REWARD_MAX_CARGO||!game_weapon_player(r->weapon))return -1;
  for(int i=0;i<count;i++)if(cargo[i]<-1||(cargo[i]>=0&&!game_weapon_player(cargo[i])))return -1;
  if(slot==-1){for(int i=0;i<count;i++)if(cargo[i]==-1)return i;return -1;}
  if(slot>=count||(!replace&&cargo[slot]>=0))return -1;
  return slot;
 }
 if(r->kind!=REWARD_CREW||r->recruit.species<0||r->recruit.species>=SPEC_COUNT||r->recruit.health!=species_max_health(r->recruit.species)||!r->recruit.id||r->recruit.id>=g->next_crew_id||id_used(g,r->recruit.id)||!r->recruit.name[0]||r->recruit.name[GAME_CREW_NAME_SIZE-1])return -1;
 if(slot==-1){for(int i=0;i<g->crew_count;i++)if(reusable(&g->crew[i])){slot=i;break;}if(slot==-1)slot=g->crew_count;}
 if(slot>g->crew_count||slot>=GAME_MAX_CREW)return -1;
 if(slot<g->crew_count&&!reusable(&g->crew[slot])){
  const Crew*c=&g->crew[slot];if(!replace||c->health<=0||c->ship!=0||boarding_contested(g,c,0))return -1;
 }
 *room=recruit_room(g,slot);return *room<0?-1:slot;
}
int rewards_can_accept(const PendingReward*r,const Game*g,const int*cargo,int count,int slot,int replace){int room=-1;return plan(r,g,cargo,count,slot,replace,&room)>=0;}
int rewards_accept(PendingReward*r,Game*g,int*cargo,int count,int slot,int replace){int room=-1,index=plan(r,g,cargo,count,slot,replace,&room);
 if(index<0)return 0;
 if(r->kind==REWARD_WEAPON)cargo[index]=r->weapon;
 else {Crew recruited=r->recruit;recruited.ship=0;recruited.room=recruited.destination=recruited.station=room;g->crew[index]=recruited;if(index==g->crew_count)g->crew_count++;species_refresh_power(g);}
 memset(r,0,sizeof *r);return 1;
}
int rewards_decline(PendingReward*r){if(!r||(r->kind!=REWARD_WEAPON&&r->kind!=REWARD_CREW))return 0;memset(r,0,sizeof *r);return 1;}
