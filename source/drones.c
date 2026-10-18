#include "drones.h"
#include "ftl_data.h"
#include "enemy_catalog.h"
#include "drone_catalog.h"
#include "boarding.h"
#include "breach.h"
#include "species.h"
#include "achievements.h"
#include <string.h>
#include <stdio.h>
static int reject(Game*g,const char*text){snprintf(g->message,sizeof g->message,"%s",text);return 0;}
static int valid_slot(const Game*g,int slot){return g->drones.installed&&slot>=0&&slot<g->drones.slot_count&&slot<GAME_MAX_DRONES;}
static void clear_slot(DroneState*d){memset(d,0,sizeof *d);d->catalog_id=d->target_room=d->unit_room=-1;}
int drones_kind(int id){return id>=0&&id<GAME_DRONE_CATALOG_COUNT?imported_player_drones[id].kind:-1;}
int drones_rarity(int id){return id>=0&&id<GAME_DRONE_CATALOG_COUNT?imported_player_drones[id].rarity:0;}
static unsigned roll(Game*g,unsigned n){g->rng=g->rng*1664525u+1013904223u;return n?(g->rng>>16)%n:0;}
const char*drones_name(int id){return id>=0&&id<GAME_DRONE_CATALOG_COUNT?imported_player_drones[id].name:"Empty";}
int drones_price(int id){return id>=0&&id<GAME_DRONE_CATALOG_COUNT?imported_player_drones[id].cost:-1;}
int drones_power(int id){return id>=0&&id<GAME_DRONE_CATALOG_COUNT?imported_player_drones[id].power:0;}
void drones_init(Game*g){int i;memset(&g->drones,0,sizeof g->drones);g->drones.room=-1;g->drones.slot_count=2;g->drone_parts=0;for(i=0;i<GAME_MAX_DRONES;i++)clear_slot(&g->drones.slots[i]);}
int drones_install(Game*g){int i,j,room=-1;
 if(g->drones.installed||g->outcome==GAME_DEFEAT)return 0;
 room=game_ship_room(g,9);(void)i;
 for(j=0;j<g->player.room_count;j++)if(g->player.rooms[j].id==room)break;
 if(j==g->player.room_count)return reject(g,"No drone control room.");
 {GameLayoutInfo info;g->drones.installed=1;g->drones.room=room;g->drones.capacity=g->drones.health=2;g->drones.power=0;
  g->drones.slot_count=game_layout_info(g->player_ship,&info)&&info.drone_slots>0?(info.drone_slots<GAME_MAX_DRONES?info.drone_slots:GAME_MAX_DRONES):2;}
 return 1;
}
int drones_equip(Game*g,int slot,int id){if(!valid_slot(g,slot)||id<0||id>=GAME_DRONE_CATALOG_COUNT||g->outcome==GAME_DEFEAT)return 0;
 clear_slot(&g->drones.slots[slot]);g->drones.slots[slot].catalog_id=id;return 1;
}
int drones_remove(Game*g,int slot){if(!valid_slot(g,slot)||g->drones.slots[slot].catalog_id<0||g->outcome==GAME_DEFEAT)return 0;clear_slot(&g->drones.slots[slot]);return 1;}
int drones_reorder(Game*g,int from,int to){DroneState d;int i;if(!valid_slot(g,from)||!valid_slot(g,to)||g->outcome==GAME_DEFEAT)return 0;d=g->drones.slots[from];for(i=from;i<to;i++)g->drones.slots[i]=g->drones.slots[i+1];for(i=from;i>to;i--)g->drones.slots[i]=g->drones.slots[i-1];g->drones.slots[to]=d;return 1;}
int drones_effective_power(const Game*g){const DroneControl*d=&g->drones;int n=d->power-d->ion_bars;if(!d->installed)return 0;if(n<0)n=0;n+=d->bonus_power;if(n>d->health)n=d->health;return n>0?n:0;}
int drones_powered(const Game*g,int slot){int i,available=drones_effective_power(g);if(!valid_slot(g,slot)||!g->drones.slots[slot].enabled||g->drones.slots[slot].catalog_id<0)return 0;
 for(i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++){const DroneState*d=&g->drones.slots[i];int power=drones_power(d->catalog_id);if(!d->enabled||power<=0)continue;if(available>=power){if(i==slot)return 1;available-=power;}}
 return 0;
}
int drones_adjust_power(Game*g,int delta){DroneControl*d=&g->drones;
 if(!d->installed||(delta!=1&&delta!=-1)||g->outcome==GAME_DEFEAT)return 0;
 if(d->power+delta<0||d->power+delta+d->bonus_power>d->health)return reject(g,"Drone control capacity reached.");
 if(delta>0&&game_used_power(g)>=game_reactor(g))return reject(g,"No free reactor power.");
 d->power+=delta;snprintf(g->message,sizeof g->message,"Drone power adjusted.");return 1;
}
/* V57: hull and system repair drones also work between fights (crew page, unpaused). */
int drones_outside_combat(int id){int kind=drones_kind(id);return kind==3||kind==4;}
int drones_toggle(Game*g,int slot){DroneState*d;
 if(!valid_slot(g,slot)||g->player.hull<=0)return 0;
 if(!drones_outside_combat(g->drones.slots[slot].catalog_id)&&(g->outcome!=GAME_PLAYING||g->enemy.hull<=0))return 0;
 d=&g->drones.slots[slot];if(d->catalog_id<0)return reject(g,"Install a drone schematic.");
 if(d->enabled){d->enabled=0;if(drones_kind(d->catalog_id)==6&&d->deployed){d->deployed=0;d->unit_room=-1;snprintf(g->message,sizeof g->message,"Boarding drone lost.");return 1;}snprintf(g->message,sizeof g->message,"Drone suspended.");return 1;}
 d->enabled=1;if(!drones_powered(g,slot)){d->enabled=0;return reject(g,"Drone needs control power.");}
 if(!d->deployed){if(g->drone_parts<=0){d->enabled=0;return reject(g,"No drone parts.");}g->drone_parts--;d->deployed=1;d->charge_ticks=0;d->target_room=-1;d->work_ticks=0;d->flight=0;d->unit_room=-1;
  /* V56: hull repair fixes 3-5 hull per part; interior drones start in drone control;
   * boarding drones fly 3 s before breaching an enemy room. */
  switch(drones_kind(d->catalog_id)){case 3:d->remaining=3+(int)roll(g,3u);break;case 4:case 5:d->unit_room=g->drones.room;d->unit_health=d->catalog_id==7?150:100;break;case 6:d->flight=3*GAME_HZ;d->unit_health=150;break;default:break;}}
 snprintf(g->message,sizeof g->message,"Drone active.");return 1;
}
void drones_reset(Game*g){int i;for(i=0;i<GAME_MAX_DRONES;i++){DroneState*d=&g->drones.slots[i];d->enabled=d->deployed=d->charge_ticks=d->work_ticks=d->remaining=d->flight=d->unit_health=0;d->target_room=d->unit_room=-1;}}
int drones_is_defense(int id){return drones_kind(id)==1;}
/* Combat II moves almost twice as fast as Combat I (speed 28 vs 15), so it fires every 6 s;
 * Defense II's cooldown is 880 against Defense I's 1000. */
int drones_cooldown(int id){switch(id){case 0:case 4:return GAME_DRONE_COOLDOWN_TICKS;case 2:return 6*GAME_HZ;case 1:return GAME_DEFENSE_COOLDOWN_TICKS;case 3:return (GAME_DEFENSE_COOLDOWN_TICKS*88+50)/100;default:return 0;}}
/* Authored bounded interception: one second recharge, then earliest arrival.
   No original orbital aiming, turning, projectile speed or blind spots implied.
   Run before effects_step so a caught missile cannot also resolve damage. */
void drones_intercept(Game*g){int i,j;
 if(g->paused||g->outcome!=GAME_PLAYING||g->player.hull<=0||g->enemy.hull<=0)return;
 for(i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++){DroneState*d=&g->drones.slots[i];int best=-1,remaining=0;
  if(!drones_is_defense(d->catalog_id)||!d->deployed||!drones_powered(g,i))continue;
  if(d->charge_ticks<drones_cooldown(d->catalog_id))d->charge_ticks+=1+(game_augment_count(g,AUG_DRONE_SPEED)&&g->ticks%4==0);
  if(d->charge_ticks<drones_cooldown(d->catalog_id))continue;
  for(j=0;j<GAME_MAX_EFFECTS;j++){GameEffect*e=&g->effects[j];int flight=e->duration-e->age;
   if(!e->active||e->result!=EFFECT_PENDING||e->side!=1||flight<=0)continue;
   if(e->kind!=EFFECT_MISSILE&&!(d->catalog_id==3&&(e->kind==EFFECT_LASER||e->kind==EFFECT_ION)))continue; /* Defense II also shoots lasers and ion. */
   if(best<0||flight<remaining){best=j;remaining=flight;}
  }
  if(best>=0){GameEffect*e=&g->effects[best];e->result=EFFECT_INTERCEPT;e->age=e->duration;d->charge_ticks=0;d->target_room=-1;snprintf(g->message,sizeof g->message,e->kind==EFFECT_MISSILE?"Defense drone intercepted missile.":"Defense drone shot down a laser.");}
  /* Defense Drone I also shoots down an incoming boss boarding drone. */
  else{int k;for(k=0;k<g->flagship_drone_count;k++){FlagshipDrone*f=&g->flagship_drones[k];if(f->kind==FDRONE_BOARDER&&f->deployed&&f->flight>0){f->deployed=0;f->flight=0;f->crew=-1;f->charge=10*GAME_HZ;d->charge_ticks=0;snprintf(g->message,sizeof g->message,"Defense drone destroyed a boarding drone.");break;}}}
 }
}
static GameRoom*find_room(Ship*s,int id){int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
/* Damaged player systems the System Repair Drone can fix, in crew repair order. */
static int*repair_target(Game*g,int*room){int k;
 for(k=0;k<GAME_SYSTEM_COUNT;k++){GameSystemState*y=&g->player.systems[k];if(y->capacity>0&&y->room>=0&&y->health<y->capacity){*room=y->room;return &y->health;}}
 for(k=0;k<SUB_COUNT;k++){Subsystem*y=&g->subsystems[0][k];if(y->capacity>0&&y->room>=0&&y->health<y->capacity){*room=y->room;return &y->health;}}
 if(g->drones.installed&&g->drones.health<g->drones.capacity){*room=g->drones.room;return &g->drones.health;}
 if(g->player_cloak.capacity>0&&g->player_cloak.health<g->player_cloak.capacity){*room=g->player_cloak.room;return &g->player_cloak.health;}
 if(g->teleporter[0].installed&&g->teleporter[0].health<g->teleporter[0].capacity){*room=g->teleporter[0].room;return &g->teleporter[0].health;}
 return 0;}
static int hostile_in(const Game*g,int side,int room){int i;for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==side&&g->enemy_crew[i].room==room)return i;return -1;}
/* Interior/boarding unit melee once per second, human strength both ways (the
 * original drones fight like crew). Returns 0 when the unit is destroyed. */
static int unit_fight(Game*g,DroneState*d,int side){int i,target=hostile_in(g,side,d->unit_room);
 if(target<0)return 1;
 if(++d->charge_ticks<GAME_HZ)return 1;
 d->charge_ticks=0;g->enemy_crew[target].health-=species_melee_damage(SPEC_HUMAN);if(g->enemy_crew[target].health<0)g->enemy_crew[target].health=0;
 for(i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++){const Crew*c=&g->enemy_crew[i];if(c->health>0&&c->ship==side&&c->room==d->unit_room)d->unit_health-=species_melee_damage(c->species);}
 if(d->unit_health<=0){d->deployed=d->enabled=0;d->unit_room=-1;snprintf(g->message,sizeof g->message,"%s destroyed.",drones_name(d->catalog_id));return 0;}
 return 1;}
/* Move one room per second toward the target (interior paths are not modelled). */
static int unit_move(DroneState*d,int room){
 if(room<0)return 0;
 if(d->unit_room==room)return 1;
 if(d->target_room!=room){d->target_room=room;d->work_ticks=0;}
 if(++d->work_ticks>=GAME_HZ){d->unit_room=room;d->work_ticks=0;}
 return 0;}
static int beam_line(Game*g,int*x0,int*y0,int*x1,int*y1){const Ship*s=&g->enemy;int a,b,best=-1;long long bd=0;const GameRoom*r;
 if(s->room_count<2)return 0;
 a=(int)roll(g,(unsigned)s->room_count);
 for(b=0;b<s->room_count;b++)if(b!=a){long long dx=(2*s->rooms[b].x+s->rooms[b].w)-(2*s->rooms[a].x+s->rooms[a].w),dy=(2*s->rooms[b].y+s->rooms[b].h)-(2*s->rooms[a].y+s->rooms[a].h),d2=dx*dx+dy*dy;if(best<0||d2<bd){best=b;bd=d2;}}
 r=&s->rooms[a];*x0=(2*r->x+r->w)*GAME_BEAM_UNITS/2;*y0=(2*r->y+r->h)*GAME_BEAM_UNITS/2;
 r=&s->rooms[best];*x1=(2*r->x+r->w)*GAME_BEAM_UNITS/2;*y1=(2*r->y+r->h)*GAME_BEAM_UNITS/2;
 return 1;}
static int boarder_room(Game*g){int list[GAME_MAX_ROOMS],n=0,i;const Ship*s=&g->enemy;
 for(i=0;i<s->room_count&&n<GAME_MAX_ROOMS;i++){int y=s->rooms[i].system;if(y>=0&&y<GAME_SYSTEM_COUNT&&s->systems[y].health>0)list[n++]=s->rooms[i].id;}
 return n?list[roll(g,(unsigned)n)]:s->rooms[roll(g,(unsigned)s->room_count)].id;}
/* Hull Repair: 1 hull every 2 s until its 3-5 repairs are spent. System Repair:
 * crew repair rate, needs no oxygen, fights intruders in its room. */
static void repair_tick(Game*g,DroneState*d,int boost){
 if(drones_kind(d->catalog_id)==3){
  if(g->player.hull>=g->player.max_hull)return;
  if((d->work_ticks+=1+boost)>=2*GAME_HZ){d->work_ticks=0;g->player.hull++;if(--d->remaining<=0){d->deployed=d->enabled=0;snprintf(g->message,sizeof g->message,"Hull repair drone spent.");}}
  return;}
 {int room=-1,*health=repair_target(g,&room);
  if(hostile_in(g,0,d->unit_room)>=0){unit_fight(g,d,0);return;}
  if(!health||!unit_move(d,room))return;
  if((d->remaining+=1+boost)>=5*GAME_HZ){d->remaining=0;(*health)++;}}}
void drones_service_step(Game*g){int i;if(g->paused||g->player.hull<=0)return;
 for(i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++){DroneState*d=&g->drones.slots[i];
  if(drones_outside_combat(d->catalog_id)&&d->deployed&&drones_powered(g,i))repair_tick(g,d,game_augment_count(g,AUG_DRONE_SPEED)&&g->ticks%4==0);}}
void drones_step(Game*g){int i;if(g->paused||g->outcome!=GAME_PLAYING||g->player.hull<=0||g->enemy.hull<=0||g->enemy.room_count<=0)return;
 for(i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++){DroneState*d=&g->drones.slots[i];int kind=drones_kind(d->catalog_id),boost=game_augment_count(g,AUG_DRONE_SPEED)&&g->ticks%4==0; /* V55 Drone Reactor Booster. */
  if(kind==1||!d->deployed||!drones_powered(g,i))continue;
  if(kind==0||kind==2){int cooldown=drones_cooldown(d->catalog_id);
   if(d->charge_ticks<cooldown)d->charge_ticks+=1+boost;
   /* Combat drones keep circling a cloaked ship but do not fire. */
   if(d->charge_ticks<cooldown||game_enemy_cloaked(g))continue;
   if(kind==2){int x0,y0,x1,y1;GameDamage beam={1,1,15,0,0,0,0,0}; /* DRONE_BEAM: 1 damage per room, stopped by any shield layer. */
    g->launch_origin=ORIGIN_DRONE;
    if(beam_line(g,&x0,&y0,&x1,&y1)&&game_launch_beam(g,0,x0,y0,x1,y1,&beam)){d->charge_ticks=0;g->ach.used|=USED_DRONE;}
    g->launch_origin=ORIGIN_NONE;
    continue;}
   {GameDamage damage={imported_combat_drone.damage,imported_combat_drone.damage,imported_combat_drone.damage*15,imported_combat_drone.fire_chance*10,imported_combat_drone.breach_chance*10,0,0,0};
    /* Authored fixed orbit replaces original moving drone timing.
       Choose once; a full effect pool retries without reroll or lost charge. */
    if(d->target_room<0){g->rng=g->rng*1664525u+1013904223u;d->target_room=g->enemy.rooms[(g->rng>>16)%(unsigned)g->enemy.room_count].id;}
    g->launch_origin=ORIGIN_DRONE;
    if(game_launch_effect(g,0,d->target_room,EFFECT_LASER,&damage,0)){d->charge_ticks=0;d->target_room=-1;g->ach.used|=USED_DRONE;}
    g->launch_origin=ORIGIN_NONE;}
   continue;}
  if(kind==3||kind==4){repair_tick(g,d,boost);continue;}
  if(kind==5){int k,room=-1; /* Anti-Personnel: hunts intruders aboard. */
   if(hostile_in(g,0,d->unit_room)>=0){unit_fight(g,d,0);continue;}
   for(k=0;k<g->enemy_crew_count&&k<GAME_MAX_ENEMY_CREW&&room<0;k++)if(g->enemy_crew[k].health>0&&g->enemy_crew[k].ship==0)room=g->enemy_crew[k].room;
   if(room>=0)unit_move(d,room);
   continue;}
  if(kind==6){GameRoom*r;
   if(d->unit_room<0){if(d->flight>0&&--d->flight>0)continue;
    d->unit_room=boarder_room(g);breach_add(&g->enemy,d->unit_room,1);d->work_ticks=0;d->target_room=-1;snprintf(g->message,sizeof g->message,"Boarding drone breached the hull.");continue;}
   if(hostile_in(g,1,d->unit_room)>=0){unit_fight(g,d,1);continue;}
   r=find_room(&g->enemy,d->unit_room);
   if(r&&r->system>=0&&r->system<GAME_SYSTEM_COUNT&&g->enemy.systems[r->system].health>0){boarding_unit_sabotage(g,1,d->unit_room,&d->work_ticks);continue;}
   if(d->target_room<0||d->target_room==d->unit_room)d->target_room=boarder_room(g);
   if(++d->charge_ticks>=GAME_HZ){d->charge_ticks=0;d->unit_room=d->target_room;d->work_ticks=0;}
   continue;}
 }
}
