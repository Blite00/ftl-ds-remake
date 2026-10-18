#include "species.h"
#include <stdio.h>
static int valid(int s){return s>=0&&s<SPEC_COUNT;}
const char*species_name(int s){static const char*n[]={"Human","Engi","Mantis","Rock","Slug","Zoltan","Crystal"};return valid(s)?n[s]:"Unknown";}
int species_max_health(int s){return s==SPEC_ROCK?150:s==SPEC_ZOLTAN?70:s==SPEC_CRYSTAL?125:100;}
int species_move_ticks(int s){return s==SPEC_ROCK?2*GAME_HZ:s==SPEC_MANTIS?25:s==SPEC_CRYSTAL?38:GAME_HZ;}
int species_work_rate(int s,int ticks){return s==SPEC_ENGI?2:s==SPEC_MANTIS?(ticks%2==0):1;}
int species_melee_damage(int s){return s==SPEC_ENGI?3:s==SPEC_MANTIS?9:6;}
int species_fire_damage(int s,int fires){return s==SPEC_ROCK?0:2*(fires<3?fires:3);} /* V55: 2 HP per fire per second, at most 6 (fires now spread within rooms). */
int species_suffocation_damage(int s,int ticks){return s==SPEC_CRYSTAL?2+(ticks/GAME_HZ)%2:5;}
static GameRoom*room(Ship*s,int id){int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return NULL;}
int game_set_crew_species(Game*g,int faction,int index,int species){Crew*c;
 if(faction<0||faction>1||index<0||index>=(faction?GAME_MAX_HOSTILES:GAME_MAX_CREW)||!valid(species))return 0;
 c=faction?&g->enemy_crew[index]:&g->crew[index];c->species=species;c->health=species_max_health(species);c->ability_ticks=c->death_processed=c->heal_accumulator=c->move_ticks=c->repair_ticks=0;species_refresh_power(g);return 1;
}
void species_refresh_power(Game*g){int side,k,i;
 for(side=0;side<2;side++){Ship*s=side?&g->enemy:&g->player;Crew*c=side?g->enemy_crew:g->crew;int count=side?g->enemy_crew_count:g->crew_count;
  for(k=0;k<GAME_SYSTEM_COUNT;k++)s->systems[k].bonus_power=0;
  g->teleporter[side].bonus_power=0;
  if(!side)g->drones.bonus_power=0;
  for(i=0;i<count;i++)if(c[i].health>0&&c[i].species==SPEC_ZOLTAN&&c[i].ship==side){
   for(k=0;k<GAME_SYSTEM_COUNT;k++)if(s->systems[k].room==c[i].room&&s->systems[k].capacity>0)s->systems[k].bonus_power++;
   if(!side&&g->drones.installed&&g->drones.room==c[i].room)g->drones.bonus_power++;
   if(g->teleporter[side].installed&&g->teleporter[side].room==c[i].room)g->teleporter[side].bonus_power++;
  }
  for(k=0;k<GAME_SYSTEM_COUNT;k++){GameSystemState*d=&s->systems[k];if(d->bonus_power>d->health)d->bonus_power=d->health;if(d->power>d->health-d->bonus_power)d->power=d->health-d->bonus_power;}
  if(!side){DroneControl*d=&g->drones;if(d->bonus_power>d->health)d->bonus_power=d->health;if(d->power>d->health-d->bonus_power)d->power=d->health-d->bonus_power;}
  {Teleporter*t=&g->teleporter[side];if(t->bonus_power>t->health)t->bonus_power=t->health;if(t->power>t->health-t->bonus_power)t->power=t->health-t->bonus_power;}
 }
}
int species_teleporter_power(const Game*g,int side){const Teleporter*t;if(side<0||side>1)return 0;t=&g->teleporter[side];int manual=t->power-t->ion_bars;int p=(manual>0?manual:0)+t->bonus_power;return p<t->health?p:t->health;}
int species_door_locked(const Game*g,int side,const GameDoor*d){const Ship*s;if(side<0||side>1||!d)return 0;s=side?&g->enemy:&g->player;int i;
 for(i=0;i<s->room_count;i++)if((s->rooms[i].id==d->room_a||s->rooms[i].id==d->room_b)&&s->rooms[i].lockdown_ticks>0)return 1;
 return 0;
}
int game_crystal_lockdown(Game*g,int index){Crew*c;GameRoom*r;int i,n;GameDoor*d;
 if(index<0||index>=g->crew_count||g->outcome==GAME_DEFEAT)return 0;
 c=&g->crew[index];
 if(c->species!=SPEC_CRYSTAL||c->health<=0||c->room!=c->destination){snprintf(g->message,sizeof g->message,"Select a stationary living Crystal.");return 0;}
 if(c->ability_ticks){snprintf(g->message,sizeof g->message,"Crystal ability is recharging.");return 0;}
 r=room(c->ship?&g->enemy:&g->player,c->room);if(!r)return 0;r->lockdown_ticks=CRYSTAL_LOCKDOWN_TICKS;c->ability_ticks=CRYSTAL_COOLDOWN_TICKS;
 d=game_ship_doors(g,c->ship,&n);for(i=0;i<n;i++)if(d[i].room_a==c->room||d[i].room_b==c->room){d[i].open=0;d[i].transit_ticks=0;}
 snprintf(g->message,sizeof g->message,"Crystal lockdown activated.");return 1;
}
int species_slug_senses(const Game*g,int side,int id){int i,j,slug=0;const Ship*s=side?&g->enemy:&g->player;const unsigned char(*adj)[GAME_MAX_ROOMS]=side?g->enemy_adjacency:g->adjacency;int target=-1;
 for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)target=i;
 if(target<0)return 0;
 for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].species==SPEC_SLUG){
  slug=1;if(g->crew[i].ship==side)for(j=0;j<s->room_count;j++)if(s->rooms[j].id==g->crew[i].room&&(j==target||adj[j][target]))return 1;
 }
 if(slug)for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==side&&g->enemy_crew[i].room==id)return 1;
 return 0;
}
void species_step(Game*g,int service){int side,i,j,again=1;
 if(g->paused||g->outcome==GAME_DEFEAT)return;
 /* Service follows physical location, not faction: carried intruders remain
  * active at home while a detached enemy hull stays frozen. */
 int remote_active=!service||(g->enemy.hull>0&&game_away_crew(g));
 for(side=0;side<2;side++){Ship*s=side?&g->enemy:&g->player;Crew*c=side?g->enemy_crew:g->crew;int count=side?g->enemy_crew_count:g->crew_count;
  if(!side||remote_active)for(i=0;i<s->room_count;i++)if(s->rooms[i].lockdown_ticks>0)s->rooms[i].lockdown_ticks--;
  for(i=0;i<count;i++)if((!c[i].ship||remote_active)&&c[i].ability_ticks>0)c[i].ability_ticks--;
 }
 /* Process each death once, including chains; dismissal removes roster slots
  * before calling this function and therefore never produces a death burst. */
 while(again){again=0;for(side=0;side<2;side++){Crew*c=side?g->enemy_crew:g->crew,*op=side?g->crew:g->enemy_crew;int count=side?g->enemy_crew_count:g->crew_count,opcount=side?g->crew_count:g->enemy_crew_count;
  for(i=0;i<count;i++)if(c[i].health<=0&&!c[i].death_processed&&(!c[i].ship||remote_active)){c[i].death_processed=1;again=1;
   if(c[i].species==SPEC_ZOLTAN)for(j=0;j<opcount;j++)if(op[j].health>0&&op[j].ship==c[i].ship&&op[j].room==c[i].room){op[j].health-=15;if(op[j].health<0)op[j].health=0;}
  }
 }}
 species_refresh_power(g);
}
