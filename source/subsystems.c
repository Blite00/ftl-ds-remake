#include "subsystems.h"
#include "ftl_data.h"
#include "species.h"
#include "boarding.h"
#include <string.h>
void subsystems_init(Game*g,int side){int i;if(side<0||side>1)return;
 for(i=0;i<SUB_COUNT;i++){Subsystem*s=&g->subsystems[side][i];s->room=-1;s->capacity=s->health=s->ion_bars=s->ion_ticks=0;}
 /* Current imported enemy blueprints do not define either subsystem. */
 if(side)return;
 for(i=0;i<SUB_COUNT;i++){int code=i==SUB_DOORS?7:6,level=game_ship_start_level(g,code);
  if(level>0){g->subsystems[0][i].room=game_ship_room(g,code);g->subsystems[0][i].capacity=g->subsystems[0][i].health=level;}}
}
int subsystem_level(const Game*g,int side,int kind){const Subsystem*s;int level,cap;if(side<0||side>1||kind<0||kind>=SUB_COUNT)return 0;s=&g->subsystems[side][kind];level=s->health>s->ion_bars?s->health-s->ion_bars:0;
 cap=g->status_cap[side][kind==SUB_DOORS?7:6];return cap>=0&&level>cap?cap:level;} /* V62: event status caps sensors and doors. */
int game_room_visible(const Game*g,int side,int room){const Ship*s;int i,found=0,level;if(side<0||side>1)return 0;s=side?&g->enemy:&g->player;
 for(i=0;i<s->room_count;i++)found|=s->rooms[i].id==room;
 if(!found)return 0;
 if(species_slug_senses(g,side,room))return 1;
 /* V55 Advanced Scanners: the enemy interior is visible without sensors. */
 if(side&&game_augment_count(g,AUG_ADV_SCANNERS))return 1;
 level=game_sensor_level(g);if(level>=(side?2:1)&&!(side&&game_sensors_blinded(g)))return 1; /* V60: nebulas blind sensors to the enemy. */
 for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==side&&g->crew[i].room==room)return 1;
 return 0;
}
/* Original manned-sensors bonus: a working, manned sensors room reads one level higher. */
int game_sensor_level(const Game*g){int level=subsystem_level(g,0,SUB_SENSORS),i;const Subsystem*s=&g->subsystems[0][SUB_SENSORS];
 if(level<=0||s->room<0)return level;
 for(i=0;i<g->crew_count;i++){const Crew*c=&g->crew[i];if(c->health>0&&c->ship==0&&c->room==s->room&&c->destination==s->room&&!boarding_contested(g,c,0))return level+1;}
 return level;
}
int game_enemy_details_visible(const Game*g){return game_sensor_level(g)>=3&&!game_sensors_blinded(g);}
void subsystem_damage(Game*g,int side,int room,int amount){int i;if(side<0||side>1||amount<=0)return;
 for(i=0;i<SUB_COUNT;i++){Subsystem*s=&g->subsystems[side][i];if(s->room==room){s->health-=amount;if(s->health<0)s->health=0;}}
}
int subsystem_door_pass(Game*g,int faction,int side,int index){int n;GameDoor*d;int level;
 if(side<0||side>1||faction<0||faction>1||g->paused||g->outcome==GAME_DEFEAT)return 0;
 n=side?g->enemy_door_count:g->door_count;
 if(index<0||index>=n)return 0;
 d=side?&g->enemy_doors[index]:&g->doors[index];
 if(species_door_locked(g,side,d))return 0;
 if(d->open||faction==side)return 1;
 level=subsystem_level(g,side,SUB_DOORS);if(level<2)return 1;
 d->force_idle_ticks=0;
 if(++d->force_ticks>=(level-1)*2*GAME_HZ){d->open=1;d->transit_ticks=0;d->jam_ticks=5*GAME_HZ;d->force_ticks=0;return 1;}
 return 0;
}
void subsystem_step(Game*g,int service){int side,i;if(g->paused||g->outcome==GAME_DEFEAT||(!service&&g->outcome!=GAME_PLAYING))return;
 for(side=0;side<2;side++){int n=side?g->enemy_door_count:g->door_count;GameDoor*doors=side?g->enemy_doors:g->doors;
  if(service&&side&&!(g->enemy.hull>0&&game_away_crew(g)))continue;
  for(i=0;i<n;i++){GameDoor*d=&doors[i];
   if(species_door_locked(g,side,d)){d->open=0;d->transit_ticks=0;continue;}
   if(d->jam_ticks>0){d->open=1;d->transit_ticks=0;if(--d->jam_ticks==0){d->open=0;d->force_ticks=d->force_idle_ticks=0;}}
   if(d->force_ticks>0&&++d->force_idle_ticks>=GAME_HZ)d->force_ticks=d->force_idle_ticks=0;
  }
 }
}
