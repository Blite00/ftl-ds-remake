#include "boarding.h"
#include "ftl_data.h"
#include "species.h"
#include <stdio.h>
#include <string.h>
static Ship*ship(Game*g,int side){return side?&g->enemy:&g->player;}
static Crew*roster(Game*g,int faction){return faction?g->enemy_crew:g->crew;}
static int count(const Game*g,int faction){return faction?g->enemy_crew_count:g->crew_count;}
static GameRoom*room(Ship*s,int id){int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return NULL;}
static int reject(Game*g,const char*why){snprintf(g->message,sizeof g->message,"%s",why);return 0;}
static int used(const Game*g,int side){int i,n=g->teleporter[side].power+(side?g->enemy_drone.power:g->drones.power);const Ship*s=side?&g->enemy:&g->player;for(i=0;i<GAME_SYSTEM_COUNT;i++)n+=s->systems[i].power;return n;}
int boarding_install(Game*g,int side){int i,r=-1;Teleporter*t;
 if(side<0||side>1)return 0;
 t=&g->teleporter[side];if(t->installed)return reject(g,"Teleporter already installed.");
 if(side)r=1;else r=game_ship_room(g,8);(void)i;
 if(!room(ship(g,side),r))return reject(g,"No teleporter room.");
 memset(t,0,sizeof *t);t->installed=t->capacity=t->health=1;t->room=r;return 1;
}
int boarding_power(Game*g,int side,int delta){Teleporter*t;
 if(side<0||side>1||(delta!=1&&delta!=-1)||g->outcome==GAME_DEFEAT)return 0;
 t=&g->teleporter[side];
 if(!t->installed)return reject(g,"Install a teleporter first.");
 if(t->power+delta<0||t->power+delta+t->bonus_power>t->health)return reject(g,"Teleporter capacity reached.");
 if(delta>0&&used(g,side)>=(side?ship(g,side)->reactor:game_reactor(g)))return reject(g,"No free reactor power.");
 t->power+=delta;return 1;
}
static int ready(Game*g,int side){Teleporter*t=&g->teleporter[side];
 if(!t->installed)return reject(g,"Install a teleporter first.");
 if(t->health<=0||species_teleporter_power(g,side)<=0)return reject(g,"Teleporter needs repair or power.");
 if(t->cooldown_ticks)return reject(g,"Teleporter is recharging.");
 return 1;
}
static int transfer(Game*g,int faction,unsigned mask,int target,int recall){
 int i,n=0,source=-1,occupants=0,launchpower=species_teleporter_power(g,faction),destside=recall?faction:1-faction;Crew*c=roster(g,faction);GameRoom*r=room(ship(g,destside),target);
 int owned=faction?(g->enemy_crew_count<GAME_MAX_ENEMY_CREW?g->enemy_crew_count:GAME_MAX_ENEMY_CREW):g->crew_count;
 if(!mask||(mask>>owned))return reject(g,"Select a boarding party.");
 if(!r||ship(g,destside)->hull<=0)return reject(g,"No valid destination.");
 if(!ready(g,faction))return 0;
 for(i=0;i<count(g,faction);i++){
  if(i<owned&&(mask&(1u<<i))){n++;if(c[i].health<=0)return reject(g,"Dead crew cannot teleport.");
   if(c[i].ship!=(recall?1-faction:faction)||c[i].room!=c[i].destination)return reject(g,"Party must be together and stationary.");
   if(source<0)source=c[i].room;else if(source!=c[i].room)return reject(g,"Select crew in the same room.");
   if(!recall&&source!=g->teleporter[faction].room)return reject(g,"Move party to the teleporter.");
  }else if(c[i].health>0&&c[i].ship==destside&&c[i].room==target)occupants++;
 }
 if(n>2)return reject(g,"Teleporter holds two crew.");
 if(n+occupants>r->w*r->h)return reject(g,"Destination room is full.");
 for(i=0;i<count(g,faction);i++)if(i<owned&&(mask&(1u<<i))){c[i].ship=destside;c[i].room=c[i].destination=target;c[i].move_ticks=c[i].repair_ticks=c[i].heal_accumulator=c[i].combat_ticks=c[i].sabotage_ticks=0;}
 species_refresh_power(g);
 g->teleporter[faction].cooldown_ticks=(25-5*launchpower)*GAME_HZ;
 snprintf(g->message,sizeof g->message,recall?"Boarding party recalled.":"Boarding party deployed.");return 1;
}
int boarding_deploy(Game*g,unsigned mask,int target){if(g->outcome!=GAME_PLAYING)return reject(g,"Encounter has ended.");if(game_enemy_super_shield(g))return reject(g,"Super shield blocks the teleporter.");return transfer(g,0,mask,target,0);}
int boarding_recall(Game*g,unsigned mask){if(g->outcome==GAME_DEFEAT)return 0;if(game_enemy_super_shield(g))return reject(g,"Super shield traps your crew aboard.");return transfer(g,0,mask,g->teleporter[0].room,1);}
int game_away_crew(const Game*g){int i,n=0;for(i=0;i<g->crew_count;i++)n+=g->crew[i].health>0&&g->crew[i].ship==1;return n;}
int game_abandon_crew(Game*g){int i,n=0;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==1){g->crew[i].health=0;n++;}return n;}
int boarding_contested(const Game*g,const Crew*c,int faction){int i;const Crew*op=faction?g->crew:g->enemy_crew;
 for(i=0;i<count(g,!faction);i++)if(op[i].health>0&&op[i].ship==c->ship&&op[i].room==c->room)return 1;
 return 0;
}
void boarding_prepare_departure(Game*g){int i;for(i=0;i<g->crew_count;i++){Crew*c=&g->crew[i];if(c->health<=0&&c->ship==1){c->ship=0;c->room=c->destination=room(&g->player,c->station)?c->station:g->player.rooms[0].id;c->move_ticks=c->repair_ticks=c->heal_accumulator=c->combat_ticks=c->sabotage_ticks=0;}}}
void boarding_destroyed_ships(Game*g){int f,i;if(g->enemy.hull<=0){g->enemy_pending_shots=g->enemy_fire_ticks=g->enemy_weapon.charge_ticks=g->enemy_weapon.pending_shots=0;}for(f=0;f<2;f++)for(i=0;i<count(g,f);i++){Crew*c=&roster(g,f)[i];if(ship(g,c->ship)->hull<=0)c->health=0;}}
static void sabotage(Game*g,Crew*c,int faction){Ship*s=ship(g,c->ship);GameRoom*r=room(s,c->room);int*health=NULL,*power=NULL;int sub;
 if(c->ship==faction||!r){c->sabotage_ticks=0;return;}
 if(r->system>=0){health=&s->systems[r->system].health;power=&s->systems[r->system].power;}
 else if(c->room==g->teleporter[c->ship].room&&g->teleporter[c->ship].installed){health=&g->teleporter[c->ship].health;power=&g->teleporter[c->ship].power;}
 else if(!c->ship&&g->drones.installed&&c->room==g->drones.room){health=&g->drones.health;power=&g->drones.power;}
 else if(c->ship&&c->room==g->enemy_drone.room&&g->enemy_drone.capacity){health=&g->enemy_drone.health;power=&g->enemy_drone.power;}
 else if(c->room==(c->ship?g->enemy_pilot_room:g->pilot_room))health=c->ship?&g->enemy_pilot_health:&g->pilot_health;
 if(!health&&c->ship){int k;for(k=0;k<g->artillery_count;k++)if(g->artillery[k].room==c->room){health=&g->artillery[k].health;power=&g->artillery[k].power;}
  if(!health&&g->enemy_cloak.capacity>0&&g->enemy_cloak.room==c->room){health=&g->enemy_cloak.health;power=&g->enemy_cloak.power;}}
 if(!health)for(sub=0;sub<SUB_COUNT;sub++)if(g->subsystems[c->ship][sub].room==c->room)health=&g->subsystems[c->ship][sub].health;
 if(!health||!*health){c->sabotage_ticks=0;return;}
 if((c->sabotage_ticks+=species_work_rate(c->species,g->ticks))>=5*GAME_HZ){(*health)--;if(power&&*power>*health)*power=*health;c->sabotage_ticks=0;
  if(s->shields>game_shield_capacity(s))s->shields=game_shield_capacity(s);
  if(!s->shields)s->shield_ticks=0;}
}
/* V56 boarding drone: sabotages the room it stands in with the human crew rate.
 * Returns 1 while the room still has something to damage. */
int boarding_unit_sabotage(Game*g,int side,int room_id,int*ticks){Crew c;memset(&c,0,sizeof c);c.ship=side;c.room=c.destination=room_id;c.species=SPEC_HUMAN;c.health=100;c.sabotage_ticks=*ticks;
 sabotage(g,&c,!side);*ticks=c.sabotage_ticks;return c.sabotage_ticks>0||*ticks>0;}
static unsigned rnd(Game*g,unsigned n){g->rng=g->rng*1664525u+1013904223u;return n?(g->rng>>16)%n:0;}
static int alive_slots(const Game*g,int side){int i,n=0;for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)n+=g->enemy_crew[i].health>0&&(side<0||g->enemy_crew[i].ship==side);return n;}
/* ShipAI::OnInit (FTL 1.6.14 0x625485): sabotage requests crew/2 and keeps
 * the rest home; invasion (flagship) requests 3*crew/4. ShipAI::OnLoop then
 * asks for (crew aboard - stay); a negative value requests an evacuation.
 * GetTeleportCommand stops sending after two completed sends. */
int boarding_enemy_request(Game*g){int r;
 if(g->enemy_board_mode<=0||!g->teleporter[1].installed||g->enemy.hull<=0||g->enemy_departed)return 0;
 if(g->enemy_board_stay<0){int n=alive_slots(g,-1);g->enemy_board_stay=n-(g->enemy_board_mode==2?3*n/4:n/2);}
 r=alive_slots(g,1)-g->enemy_board_stay;
 if(r>0&&g->enemy_board_sends>=2)r=0;
 return r;
}
/* CrewAI::PrioritizeTask manning values (normal difficulty); idle is 1000. */
int boarding_manning_priority(const Game*g,const Crew*c){const Ship*s=&g->enemy;int i,station=c->station;
 if(station==g->enemy_pilot_room)return 64;
 for(i=0;i<s->room_count;i++)if(s->rooms[i].id==station&&s->rooms[i].system>=0){int sys=s->rooms[i].system;
  return sys==SYS_SHIELDS?65:sys==SYS_WEAPONS?66:sys==SYS_ENGINES?67:sys==SYS_OXYGEN?68:72;}
 if(g->subsystems[1][SUB_SENSORS].capacity&&station==g->subsystems[1][SUB_SENSORS].room)return 69;
 if(g->subsystems[1][SUB_DOORS].capacity&&station==g->subsystems[1][SUB_DOORS].room)return 70;
 if(g->teleporter[1].installed&&station==g->teleporter[1].room)return 71;
 return 1000;
}
static int bad_fighter(int species){return species_melee_damage(species)<species_melee_damage(SPEC_HUMAN)||species_max_health(species)<100;}
/* CrewAI teleport tasks: 32 when charged, 65 otherwise; 100 for crew below
 * half health or bad at combat (unless all are). AttemptToAssign gives each
 * task to the least-busy eligible crew whose current task is less important. */
unsigned boarding_enemy_party(Game*g,const int*current,int count){
 Teleporter*t=&g->teleporter[1];int want=boarding_enemy_request(g),all_bad=1,i,k,charged,prio[GAME_MAX_ENEMY_CREW];unsigned party=0;
 if(want<=0||!t->installed||t->health<=0||species_teleporter_power(g,1)<=0)return 0;
 if(want>2)want=2; /* DS teleporter pad holds two. */
 if(count>GAME_MAX_ENEMY_CREW)count=GAME_MAX_ENEMY_CREW;
 charged=!t->cooldown_ticks;
 for(i=0;i<count;i++){const Crew*c=&g->enemy_crew[i];if(c->health>0&&c->ship==1&&!bad_fighter(c->species))all_bad=0;}
 for(i=0;i<count;i++){const Crew*c=&g->enemy_crew[i];int max=species_max_health(c->species);
  prio[i]=c->health*2<max||(bad_fighter(c->species)&&!all_bad)?100:charged?32:65;}
 for(k=0;k<want;k++){int best=-1;
  for(i=0;i<count;i++){const Crew*c=&g->enemy_crew[i];
   if(c->health<=0||c->ship!=1||(party&(1u<<i))||prio[i]>=current[i])continue;
   if(best<0||current[i]>current[best])best=i;}
  if(best<0)break;
  party|=1u<<best;
 }
 return party;
}
static int enemy_broken_systems(const Game*g){int i,n=0;const Ship*s=&g->enemy;
 for(i=0;i<GAME_SYSTEM_COUNT;i++)n+=s->systems[i].capacity>0&&s->systems[i].health<=0;
 for(i=0;i<SUB_COUNT;i++)n+=g->subsystems[1][i].capacity>0&&g->subsystems[1][i].health<=0;
 n+=g->teleporter[1].installed&&g->teleporter[1].health<=0;
 n+=g->enemy_drone.capacity>0&&g->enemy_drone.health<=0;
 n+=g->enemy_pilot_room>=0&&!g->enemy_pilot_health;
 for(i=0;i<g->artillery_count;i++)n+=g->artillery[i].health<=0;
 n+=g->enemy_cloak.capacity>0&&g->enemy_cloak.health<=0;
 return n;
}
/* ShipAI::RequiredEvac: escaping with under 15 s left, or three or more
 * completely broken systems aboard the AI ship. */
int boarding_enemy_evac_forced(const Game*g){
 return (g->enemy_escape_active&&g->enemy_escape_ticks>0&&g->enemy_escape_ticks<15*GAME_HZ)||enemy_broken_systems(g)>=3;
}
/* CrewAI::PrioritizeIntruderRoom: danger (fire 100, air below 10% 70), +5
 * when slots differ from the party size, +10 when not adjacent, +20 without
 * a working system. Full or unreachable rooms are excluded. */
static int intruder_score(Game*g,const Crew*c,int id,int boarders){
 GameRoom*r=room(&g->player,id);int score=0,health=0,j,n=0;
 if(!r||game_next_room(g,0,c->room,id)<0)return 10000;
 for(j=0;j<g->enemy_crew_count;j++)if(&g->enemy_crew[j]!=c&&g->enemy_crew[j].health>0&&g->enemy_crew[j].ship==0&&g->enemy_crew[j].destination==id)n++;
 if(n>=r->w*r->h)return 10000;
 if(r->fires&&c->species!=SPEC_ROCK)score+=100;
 if(r->oxygen<100)score+=70;
 if(r->w*r->h!=boarders)score+=5;
 if(r->system>=0)health=g->player.systems[r->system].health;
 else if(id==g->pilot_room)health=g->pilot_health;
 else if(g->teleporter[0].installed&&id==g->teleporter[0].room)health=g->teleporter[0].health;
 else if(g->drones.installed&&id==g->drones.room)health=g->drones.health;
 else for(j=0;j<SUB_COUNT;j++)if(g->subsystems[0][j].room==id)health=g->subsystems[0][j].health;
 if(!health)score+=20;
 if(id!=c->room&&game_next_room(g,0,c->room,id)!=id)score+=10;
 return score;
}
static void intruder_order(Crew*c,int target){
 if(c->destination==target)return;
 c->destination=target;c->move_ticks=c->repair_ticks=c->heal_accumulator=c->sabotage_ticks=0;
}
/* InitiateTeleport with room -1 picks rand()%roomCount (0x409d8b). The DS
 * model is room-level, so choose uniformly among rooms with space. */
static int landing_room(Game*g,int party){int i,free_rooms[GAME_MAX_ROOMS],n=0;
 for(i=0;i<g->player.room_count;i++){GameRoom*r=&g->player.rooms[i];int j,occ=0;
  for(j=0;j<g->crew_count;j++)occ+=g->crew[j].health>0&&g->crew[j].ship==0&&g->crew[j].room==r->id;
  for(j=0;j<g->enemy_crew_count;j++)occ+=g->enemy_crew[j].health>0&&g->enemy_crew[j].ship==0&&g->enemy_crew[j].room==r->id;
  if(occ+party<=r->w*r->h)free_rooms[n++]=r->id;}
 return n?free_rooms[rnd(g,(unsigned)n)]:-1;
}
static void enemy_boarding_ai(Game*g){int i,j,request,boarders=0,cascade=-1,ready;Teleporter*t=&g->teleporter[1];
 if(g->ticks%GAME_HZ||g->enemy.automated||g->enemy.hull<=0)return;
 request=boarding_enemy_request(g);
 ready=t->installed&&t->health>0&&!t->cooldown_ticks&&species_teleporter_power(g,1)>0;
 /* Evacuation: the first boarder needing it names the room; everyone there
  * (up to the two-person pad) returns. */
 if(ready){int forced=request<0||boarding_enemy_evac_forced(g),from=-1,n=0;unsigned mask=0;
  for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW&&from<0;i++){Crew*c=&g->enemy_crew[i];
   if(c->health>0&&c->ship==0&&(forced||(g->enemy_board_mode==1&&c->health*4<species_max_health(c->species))))from=c->room;}
  for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW&&from>=0&&n<2;i++){Crew*c=&g->enemy_crew[i];
   if(c->health>0&&c->ship==0&&c->room==from){c->destination=c->room;mask|=1u<<i;n++;}}
  if(mask&&transfer(g,1,mask,t->room,1))ready=0;
 }
 /* Send once the requested crew (up to the two-person pad) stand on it. */
 if(ready&&request>0){unsigned party=0;int n=0,want=request>2?2:request,target;
  for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW&&n<want;i++){Crew*c=&g->enemy_crew[i];
   if(c->health>0&&c->health*2>=species_max_health(c->species)&&c->ship==1&&c->room==t->room&&c->destination==t->room){party|=1u<<i;n++;}}
  if(n>=want&&(target=landing_room(g,n))>=0&&transfer(g,1,party,target,0)&&request-n<=0)g->enemy_board_sends++;
 }
 for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)boarders+=g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==0;
 /* CrewAI::UpdateIntruders: each boarder's choice is also ordered for every
  * later boarder, who keeps it unless a strictly better room exists. Ties
  * keep the current room; otherwise pick randomly among the best. */
 for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++){Crew*c=&g->enemy_crew[i];int best,current,ties=0,options[GAME_MAX_ROOMS];
  if(c->health<=0||c->ship!=0||boarding_contested(g,c,1))continue;
  if(cascade>=0&&intruder_score(g,c,cascade,boarders)<10000)intruder_order(c,cascade);
  current=best=intruder_score(g,c,c->destination,boarders);
  for(j=0;j<g->player.room_count;j++){int id=g->player.rooms[j].id,score=intruder_score(g,c,id,boarders);
   if(score<best){best=score;ties=0;}
   if(score==best&&score<10000&&id!=c->destination)options[ties++]=id;}
  if(best<current&&ties)intruder_order(c,options[rnd(g,(unsigned)ties)]);
  if(best<10000)cascade=c->destination;
 }
}

void boarding_step(Game*g,int service){int f,i,j,damage[2][GAME_MAX_HOSTILES]={{0}};
 if(g->paused||g->outcome==GAME_DEFEAT)return;
 for(f=0;f<(service?1:2);f++)if(species_teleporter_power(g,f)>0&&g->teleporter[f].health>0&&g->teleporter[f].cooldown_ticks>0)g->teleporter[f].cooldown_ticks--;
 if(!service&&g->outcome!=GAME_PLAYING)return;
 if(!service)enemy_boarding_ai(g);
 for(f=0;f<2;f++)for(i=0;i<count(g,f);i++){Crew*c=&roster(g,f)[i];Crew*op=roster(g,!f);
  if(c->health<=0||(service&&c->ship!=0))continue;
  if(boarding_contested(g,c,f)){
   c->sabotage_ticks=0;
   if(++c->combat_ticks>=GAME_HZ){c->combat_ticks=0;for(j=0;j<count(g,!f);j++)if(op[j].health>0&&op[j].ship==c->ship&&op[j].room==c->room){damage[!f][j]+=species_melee_damage(c->species);break;}}
  }else{c->combat_ticks=0;if(c->room==c->destination)sabotage(g,c,f);else c->sabotage_ticks=0;}
 }
 for(f=0;f<2;f++)for(i=0;i<count(g,f);i++){Crew*c=&roster(g,f)[i];c->health-=damage[f][i];if(c->health<0)c->health=0;}
}
