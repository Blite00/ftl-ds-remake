#include "save.h"
#include "boarding.h"
#include "species.h"
#include "drones.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#define SAVE_CAP 16384
/* Historical payload fits8192; carried appendix is4+48*128=6148 bytes. */
#if SAVE_CAP < 8192 + 4 + GAME_MAX_INTRUDERS * 128
#error Save capacity does not cover the historical payload and carried roster
#endif
#define SAVE_HEADER 20
/* Different magic/path protects v9 saves. Explicit integer fields only. */
static const unsigned char magic[8]={'F','T','L','D','C','M','P',0};
typedef struct {unsigned char bytes[SAVE_CAP];unsigned size,pos;int ok,version;} Buffer;
static Buffer work_payload,work_header;
static Campaign work_campaign;
static void put(Buffer*b,uint32_t v){if(b->size+4>SAVE_CAP){b->ok=0;return;}for(unsigned i=0;i<4;i++)b->bytes[b->size++]=(unsigned char)(v>>(i*8));}
static uint32_t get(Buffer*b){uint32_t v=0;if(b->pos+4>b->size){b->ok=0;return 0;}for(unsigned i=0;i<4;i++)v|=(uint32_t)b->bytes[b->pos++]<<(i*8);return v;}
static int value(Buffer*b,int lo,int hi){int64_t v=(int32_t)get(b);if(v<lo||v>hi){b->ok=0;return lo;}return (int)v;}
static uint32_t checksum(const unsigned char*p,unsigned n){uint32_t h=2166136261u;for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static int room_exists(const Game*g,int id){for(int i=0;i<g->player.room_count;i++)if(g->player.rooms[i].id==id)return 1;return 0;}
static uint32_t layout_id(const Game*g){
 uint32_t h=2166136261u;
 for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];h=(h^(unsigned)(r->id+31*r->x+977*r->y+37*r->w+73*r->h+103*r->system))*16777619u;}
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];h=(h^(unsigned)(d->x+31*d->y+977*d->room_a+37*d->room_b+73*d->vertical))*16777619u;}
 return h;
}
/* V57: a save names its layout by the hash of that layout's starting rooms and doors,
 * so systems installed later never change it (Kestrel A keeps its old id). */
static Game layout_probe;
static uint32_t ship_layout_id(int layout){if(!game_init_ship(&layout_probe,layout))return 0;return layout_id(&layout_probe);}
static int find_layout(uint32_t id){for(int k=0;k<game_layout_count();k++)if(game_layout_supported(k)&&ship_layout_id(k)==id)return k;return -1;}
static int encode(const Campaign*c,Buffer*b){
 const Game*g=&c->game;
 if(c->mode!=CAM_MAP||c->reward.kind!=REWARD_NONE||c->imported_battle||g->crew_count<1||g->crew_count>GAME_MAX_CREW||g->weapon_count<0||g->weapon_count>GAME_MAX_WEAPONS||g->player.room_count<1||g->player.room_count>GAME_MAX_ROOMS||g->door_count<0||g->door_count>GAME_MAX_DOORS)return 0;
 /* Current encounter home crew need explicit promotion before a map save. */
 if(g->enemy_crew_count<0||g->enemy_crew_count>GAME_MAX_HOSTILES)return 0;
 for(int i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++){
  const Crew*r=&g->enemy_crew[i];
  if(r->ship==0&&(r->health>0||(r->id&&!r->death_processed)))return 0;
 }
 memset(b,0,sizeof(*b));b->ok=1;b->version=32;
 put(b,ship_layout_id(g->player_ship));put(b,g->player.room_count);put(b,g->door_count);
 put(b,c->seed);put(b,c->rng);put(b,c->sector);put(b,c->current);put(b,c->selected);put(b,c->fuel);put(b,c->scrap);put(b,c->jumps);put(b,c->sector_jumps);put(b,c->quest);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++){put(b,c->visited[i]);put(b,c->links[i]);put(b,c->node_kind[i]);put(b,c->node_y[i]);for(int j=0;j<8;j++)put(b,c->stock[i][j]);}
 for(int i=0;i<CAM_CARGO_COUNT;i++)put(b,c->cargo[i]);
 put(b,g->player.hull);put(b,g->player.reactor);put(b,g->player.oxygen);put(b,g->player.shields);put(b,g->player.shield_ticks);
 put(b,g->missiles);put(b,g->ticks);put(b,g->rng);put(b,g->pilot_health);put(b,g->paused);
 for(int i=0;i<GAME_SYSTEM_COUNT;i++){const GameSystemState*s=&g->player.systems[i];put(b,s->capacity);put(b,s->health);put(b,s->power);}
 /* V58: Federation layouts (the only ones with artillery) add its level, health and power. */
 if(game_ship_start_level(g,11)>0){put(b,g->player_artillery.capacity);put(b,g->player_artillery.health);put(b,g->player_artillery.power);}
 for(int i=0;i<g->player.room_count;i++){put(b,g->player.rooms[i].oxygen);put(b,g->player.rooms[i].exposed);}
 for(int i=0;i<g->door_count;i++){put(b,g->doors[i].open);put(b,g->doors[i].transit_ticks);}
 put(b,g->crew_count);for(int i=0;i<g->crew_count;i++){const Crew*r=&g->crew[i];put(b,r->room);put(b,r->destination);put(b,r->health);put(b,r->move_ticks);put(b,r->repair_ticks);put(b,r->heal_accumulator);for(int j=0;j<4;j++)put(b,r->skill[j]);}
 put(b,g->weapon_count);for(int i=0;i<g->weapon_count;i++){put(b,g->weapons[i].catalog_id);put(b,g->weapons[i].enabled);}
 for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];put(b,r->fires);put(b,r->fire_ticks);put(b,r->firefight_ticks);}
 put(b,g->next_crew_id);
 for(int i=0;i<g->crew_count;i++){const Crew*r=&g->crew[i];put(b,r->id);put(b,r->station);put(b,r->ship);for(int k=0;k<GAME_CREW_NAME_LEN;k++)put(b,(unsigned char)r->name[k]);}
 {const Teleporter*t=&g->teleporter[0];put(b,t->installed);put(b,t->room);put(b,t->capacity);put(b,t->health);put(b,t->power);put(b,t->cooldown_ticks);}
 for(int i=0;i<g->crew_count;i++){put(b,g->crew[i].combat_ticks);put(b,g->crew[i].sabotage_ticks);}
 for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];put(b,r->breaches);put(b,r->breach_repair_ticks);}
 for(int i=0;i<SUB_COUNT;i++){const Subsystem*s=&g->subsystems[0][i];put(b,s->room);put(b,s->capacity);put(b,s->health);}
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];put(b,d->force_ticks);put(b,d->force_idle_ticks);put(b,d->jam_ticks);}
 for(int i=0;i<g->crew_count;i++){const Crew*r=&g->crew[i];put(b,r->species);put(b,r->ability_ticks);put(b,r->death_processed);}
 for(int i=0;i<g->player.room_count;i++)put(b,g->player.rooms[i].lockdown_ticks);
 for(int i=0;i<GAME_SYSTEM_COUNT;i++){put(b,g->player.systems[i].ion_bars);put(b,g->player.systems[i].ion_ticks);}
 for(int i=0;i<SUB_COUNT;i++){put(b,g->subsystems[0][i].ion_bars);put(b,g->subsystems[0][i].ion_ticks);}
 put(b,g->teleporter[0].ion_bars);put(b,g->teleporter[0].ion_ticks);put(b,g->pilot_ion_ticks[0]);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)put(b,c->stock[i][8]);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)put(b,c->stock[i][9]);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)put(b,c->stock[i][10]);
 /* V13: 48-byte drone appendix; launch state and species bonus are derived. */
 {const DroneControl*d=&g->drones;
  put(b,g->drone_parts);put(b,d->installed);put(b,d->room);put(b,d->capacity);put(b,d->health);put(b,d->power);
  put(b,d->ion_bars);put(b,d->ion_ticks);put(b,d->slot_count);
  for(int i=0;i<GAME_MAX_DRONES;i++)put(b,d->slots[i].catalog_id);
 }
 /* V14: four drone cargo entries and three appended offers per beacon. */
 for(int i=0;i<CAM_CARGO_COUNT;i++)put(b,c->drone_cargo[i]);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)for(int j=11;j<14;j++)put(b,c->stock[i][j]);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)put(b,c->stock[i][14]);
 /* V16: preserve the entire V15 prefix; append campaign policy and extra beacons. */
 put(b,c->node_count);put(b,c->sector_count);
 for(int i=CAM_LEGACY_NODE_COUNT;i<CAM_NODE_COUNT;i++){
  put(b,c->visited[i]);put(b,c->links[i]);put(b,c->node_kind[i]);put(b,c->node_y[i]);
  for(int j=0;j<CAM_SAVED_OFFERS;j++)put(b,c->stock[i][j]);
 }
 /* V17: route policy, visited sector identities and the two next routes. */
 put(b,c->route_policy);
 for(int i=0;i<CAM_SECTOR_COUNT;i++)put(b,c->sector_types[i]);
 for(int i=0;i<2;i++)put(b,c->next_sector[i]);
 /* V18: event policy and encountered-root mask; imported event cursor is never persisted. */
 put(b,c->event_policy);put(b,c->event_seen);
 /* V21: compact records preserve physical slot ownership and every Crew field.
  * Each record is32 words:15 scalar fields,4 skills and13 name bytes. */
 int intruders=0;
 for(int i=GAME_MAX_ENEMY_CREW;i<g->enemy_crew_count;i++){
  const Crew*r=&g->enemy_crew[i];
  if(!r->id&&(r->health>0||r->ship!=0))return 0;
  if(r->id&&(r->health>0||!r->death_processed))intruders++;
 }
 put(b,intruders);
 for(int i=GAME_MAX_ENEMY_CREW;i<g->enemy_crew_count;i++){
  const Crew*r=&g->enemy_crew[i];if(!r->id||(r->health<=0&&r->death_processed))continue;
  /* Disk slots keep the historical 8..55 numbering for the carried roster. */
  put(b,i-GAME_MAX_ENEMY_CREW+GAME_MAX_CREW);put(b,r->id);put(b,r->room);put(b,r->destination);put(b,r->station);put(b,r->ship);put(b,r->species);put(b,r->health);
  put(b,r->move_ticks);put(b,r->repair_ticks);put(b,r->heal_accumulator);put(b,r->combat_ticks);put(b,r->sabotage_ticks);put(b,r->ability_ticks);put(b,r->death_processed);
  for(int k=0;k<4;k++)put(b,r->skill[k]);
  for(int k=0;k<GAME_CREW_NAME_LEN;k++)put(b,(unsigned char)r->name[k]);
 }
 put(b,c->boss_resume_phase);
 /* V25: original final-sector state (flagship route, flags, rebel beacons). */
 put(b,c->finale_policy);put(b,c->boss_path_len);put(b,c->boss_loc);put(b,c->boss_move);put(b,c->boss_reversed);put(b,c->base_arrived);put(b,c->base_greeted);
 for(int i=0;i<CAM_BOSS_PATH_MAX;i++)put(b,c->boss_path[i]);
 put(b,c->fleet_nodes);put(b,c->fleet_pending);
 /* V26: surviving flagship crew per room (BossShip::crewCounts). */
 put(b,c->boss_crew_valid);for(int i=0;i<GAME_FLAGSHIP_ROOMS;i++)put(b,c->boss_crew[i]);
 /* V27: finale difficulty chosen for this run. */
 put(b,c->flagship_original);
 /* V53 (save28): seen bits for original sector-list events beyond root 30. */
 for(int i=0;i<8;i++)put(b,c->event_seen_ext[i]);
 /* V55 (save29): cloaking, augments and the two new store offers. */
 put(b,c->game.player_cloak.capacity);put(b,c->game.player_cloak.health);put(b,c->game.player_cloak.power);put(b,c->game.player_cloak.room);
 for(int i=0;i<GAME_MAX_AUGMENTS;i++)put(b,(uint32_t)c->game.augments[i]);
 for(int i=0;i<CAM_NODE_COUNT;i++){put(b,c->stock[i][15]);put(b,c->stock[i][16]);}
 /* V59 (save30): run-long achievement progress and the Kestrel 1-HP latch. */
 for(int i=0;i<4;i++)put(b,(uint32_t)c->ach_counter[i]);
 put(b,c->ach_flags);put(b,(uint32_t)c->game.tough_low);
 /* V61 (save31): quest beacons, tied to this build's event catalog. */
 put(b,campaign_event_catalog_hash());for(int q=0;q<CAM_QUESTS;q++){put(b,(uint32_t)c->quest_event[q]);put(b,(uint32_t)c->quest_node[q]);}
 /* V62 (save32): unmarked quest slots and homeworld specials placed this run. */
 put(b,c->quest_hidden);put(b,c->specials_done);
 return b->ok;
}
static int intruder_name_valid(const char*name){
 int end=0;while(end<GAME_CREW_NAME_LEN&&name[end])end++;
 if(!end||end>=GAME_CREW_NAME_LEN||name[0]==' '||name[end-1]==' ')return 0;
 for(int i=0;i<end;i++){unsigned char ch=(unsigned char)name[i];if(!((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||ch==' '||ch=='-'||ch==39))return 0;}
 for(int i=end;i<GAME_CREW_NAME_LEN;i++)if(name[i])return 0;
 return 1;
}
static int decode_intruders(Buffer*b,Game*g){
 unsigned char occupied[GAME_MAX_HOSTILES]={0};int count=value(b,0,GAME_MAX_INTRUDERS);
 memset(g->enemy_crew,0,sizeof g->enemy_crew);g->enemy_crew_count=0;
 for(int n=0;n<count;n++){
  int slot=value(b,GAME_MAX_CREW,GAME_MAX_CREW+GAME_MAX_INTRUDERS-1)-GAME_MAX_CREW+GAME_MAX_ENEMY_CREW;Crew*r=&g->enemy_crew[slot];
  if(occupied[slot])return 0;
  occupied[slot]=1;r->id=get(b);r->room=value(b,0,GAME_MAX_ROOMS-1);r->destination=value(b,0,GAME_MAX_ROOMS-1);
  /* Station belongs to the departed ship and is never an intruder order. */
  r->station=value(b,0,GAME_MAX_ROOMS-1);r->ship=value(b,0,0);r->species=value(b,0,SPEC_COUNT-1);r->health=value(b,0,species_max_health(r->species));
  r->move_ticks=value(b,0,species_move_ticks(r->species)-1);r->repair_ticks=value(b,0,5*GAME_HZ-1);r->heal_accumulator=value(b,0,GAME_HZ-1);
  r->combat_ticks=value(b,0,GAME_HZ-1);r->sabotage_ticks=value(b,0,5*GAME_HZ-1);r->ability_ticks=value(b,0,CRYSTAL_COOLDOWN_TICKS);r->death_processed=value(b,0,1);
  for(int k=0;k<4;k++)r->skill[k]=value(b,0,120*GAME_HZ);
  for(int k=0;k<GAME_CREW_NAME_LEN;k++)r->name[k]=(char)value(b,0,127);
  if((r->species!=SPEC_CRYSTAL&&r->ability_ticks)||r->death_processed||!r->id||r->id>=g->next_crew_id||!intruder_name_valid(r->name)||!room_exists(g,r->room)||!room_exists(g,r->destination)||game_next_room(g,0,r->room,r->destination)<0)return 0;
  for(int k=0;k<g->crew_count;k++)if(g->crew[k].id==r->id)return 0;
  for(int k=GAME_MAX_ENEMY_CREW;k<GAME_MAX_HOSTILES;k++)if(k!=slot&&occupied[k]&&g->enemy_crew[k].id==r->id)return 0;
  if(g->enemy_crew_count<=slot)g->enemy_crew_count=slot+1;
 }
 for(int n=0;n<g->player.room_count;n++){
  const GameRoom*room=&g->player.rooms[n];int live=0;
  for(int k=GAME_MAX_ENEMY_CREW;k<g->enemy_crew_count;k++)if(g->enemy_crew[k].health>0&&g->enemy_crew[k].room==room->id)live++;
  if(live>room->w*room->h)return 0;
 }
 return b->ok;
}
static int decode(Buffer*b,Campaign*c){
 int layout;b->pos=0;b->ok=1;layout=find_layout(get(b));if(layout<0)return 0;
 campaign_init_legacy_layout(c,layout);Game*g=&c->game;
 /* Starting optional systems are installed by the layout; their power comes from the save. */
 g->drones.power=g->teleporter[0].power=g->player_cloak.power=g->player_artillery.power=0;
 if(get(b)!=(unsigned)g->player.room_count||get(b)!=(unsigned)g->door_count)return 0;
 c->seed=get(b);c->rng=get(b);c->sector=value(b,0,CAM_SECTOR_COUNT-1);c->current=value(b,0,CAM_NODE_COUNT-1);c->selected=value(b,0,CAM_NODE_COUNT-1);
 c->fuel=value(b,0,1000000);c->scrap=value(b,0,1000000);c->jumps=value(b,0,1000000);c->sector_jumps=value(b,0,1000000);c->quest=value(b,0,2);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++){c->visited[i]=value(b,0,1);c->links[i]=value(b,1,(1<<CAM_NODE_COUNT)-1);c->node_kind[i]=value(b,0,NODE_EXIT);c->node_y[i]=value(b,50,135);for(int j=0;j<8;j++)c->stock[i][j]=value(b,0,j==7?99:j<3?3:1);}
 for(int i=0;i<CAM_CARGO_COUNT;i++)c->cargo[i]=value(b,-1,b->version>=11?GAME_CATALOG_COUNT-1:b->version>=10?5:b->version>=9?4:3);
 g->player.hull=value(b,1,g->player.max_hull);g->player.reactor=value(b,g->player.reactor<8?g->player.reactor:8,20);g->player.oxygen=value(b,0,1000);g->player.shields=value(b,0,4);g->player.shield_ticks=value(b,0,60);
 g->missiles=value(b,0,1000000);g->ticks=value(b,0,INT_MAX-1000000);g->rng=get(b);g->pilot_health=value(b,0,1);g->paused=value(b,0,1);
 for(int i=0;i<GAME_SYSTEM_COUNT;i++){GameSystemState*s=&g->player.systems[i];int installed=s->capacity>0;s->capacity=value(b,installed?1:0,i<3?8:3);s->health=value(b,0,s->capacity);s->power=value(b,0,s->health);
  /* V57: a system bought later sits in the room its layout reserves for it. */
  if(s->capacity>0&&!installed){int room=game_ship_room(g,i);s->room=room;for(int r=0;r<g->player.room_count;r++)if(g->player.rooms[r].id==room)g->player.rooms[r].system=i;if(room<0)return 0;}}
 if(game_ship_start_level(g,11)>0){Artillery*a=&g->player_artillery;a->capacity=value(b,1,4);a->health=value(b,0,a->capacity);a->power=value(b,0,a->health);}
 if(game_used_power(g)>g->player.reactor||g->player.shields>g->player.systems[SYS_SHIELDS].capacity/2)return 0;
 for(int i=0;i<g->player.room_count;i++){g->player.rooms[i].oxygen=value(b,0,1000);g->player.rooms[i].exposed=value(b,0,1);}
 for(int i=0;i<g->door_count;i++){g->doors[i].open=value(b,0,1);g->doors[i].transit_ticks=value(b,0,15);if(!g->doors[i].open&&g->doors[i].transit_ticks)return 0;}
 g->crew_count=value(b,1,b->version>=8?GAME_MAX_CREW:6);for(int i=0;i<g->crew_count;i++){Crew*r=&g->crew[i];r->room=value(b,0,31);r->destination=value(b,0,31);r->health=value(b,0,b->version>=8?150:100);r->move_ticks=value(b,0,b->version>=8?2*GAME_HZ-1:GAME_HZ-1);r->repair_ticks=value(b,0,5*GAME_HZ-1);r->heal_accumulator=value(b,0,GAME_HZ-1);for(int j=0;j<4;j++)r->skill[j]=value(b,0,120*GAME_HZ);if(!room_exists(g,r->room)||!room_exists(g,r->destination)||game_crew_next_room(g,i)<0)return 0;}
 int count=value(b,b->version>=12?0:1,GAME_MAX_WEAPONS);g->weapon_count=0;memset(g->weapons,0,sizeof g->weapons);memset(g->autofire,0,sizeof g->autofire);for(int i=0;i<count;i++){int id=value(b,0,b->version>=11?GAME_CATALOG_COUNT-1:b->version>=10?5:b->version>=9?4:3),enabled=value(b,0,1);if(!game_equip(g,i,id))return 0;g->weapons[i].enabled=enabled;}
 if(b->version<12&&g->weapon_count>=2)game_reorder_weapon(g,1,0); /* Preserve historical slot-1 priority as displayed order. */
 if(b->version>=3)for(int i=0;i<g->player.room_count;i++){GameRoom*r=&g->player.rooms[i];r->fires=value(b,0,r->w*r->h);r->fire_ticks=value(b,0,GAME_HZ-1);r->firefight_ticks=value(b,0,3*GAME_HZ-1);}
 if(b->version>=4){
  g->next_crew_id=get(b);if(!g->next_crew_id||g->next_crew_id>1000000000u)return 0;
  for(int i=0;i<g->crew_count;i++){
   Crew*r=&g->crew[i];r->id=get(b);r->station=value(b,0,31);r->ship=value(b,0,0);
   char name[GAME_CREW_NAME_LEN];for(int k=0;k<GAME_CREW_NAME_LEN;k++)name[k]=(char)value(b,0,127);
   if(!r->id||r->id>=g->next_crew_id||!room_exists(g,r->station)||name[GAME_CREW_NAME_LEN-1])return 0;
   for(int j=0;j<i;j++)if(g->crew[j].id==r->id)return 0;
   if(!game_rename_crew(g,i,name)||strcmp(name,r->name))return 0;
  }
 }else{
  /* Legacy formats have no identity. Derive stable local names/stations without changing simulation state. */
  g->next_crew_id=(unsigned)g->crew_count+1u;
  for(int i=0;i<g->crew_count;i++){Crew*r=&g->crew[i];r->id=(unsigned)i+1u;r->station=r->room;r->ship=0;
   char name[32];snprintf(name,sizeof name,"CREW %d",i+1);game_rename_crew(g,i,name);}
 }
 if(b->version>=5){
  int installed=value(b,0,1),room=(int32_t)get(b),capacity=value(b,0,3),health=value(b,0,capacity),power=value(b,0,health),cooldown=value(b,0,BOARDING_COOLDOWN);
  if(installed){
   if(!capacity||(!g->teleporter[0].installed&&!boarding_install(g,0))||room!=g->teleporter[0].room)return 0;
   g->teleporter[0].capacity=capacity;g->teleporter[0].health=health;g->teleporter[0].power=power;g->teleporter[0].cooldown_ticks=cooldown;
  }else if(capacity||health||power||cooldown||room!=g->teleporter[0].room)return 0;
  for(int i=0;i<g->crew_count;i++){g->crew[i].combat_ticks=value(b,0,GAME_HZ-1);g->crew[i].sabotage_ticks=value(b,0,5*GAME_HZ-1);}
  if(game_used_power(g)>g->player.reactor)return 0;
 }
 if(b->version>=6)for(int i=0;i<g->player.room_count;i++){
  GameRoom*r=&g->player.rooms[i];int limit=r->w*r->h;if(limit>4)limit=4;
  r->breaches=value(b,0,limit);r->breach_repair_ticks=value(b,0,5*GAME_HZ-1);
  if(!r->breaches&&r->breach_repair_ticks)return 0;
 }
 if(b->version>=7){
  /* V57: layouts without sensors or doors may have bought them into their reserved room. */
  for(int i=0;i<SUB_COUNT;i++){Subsystem*s=&g->subsystems[0][i];int room=(int32_t)get(b),installed=s->capacity>0;
   if(room!=s->room&&(installed||room!=game_ship_room(g,i==SUB_DOORS?7:6)))return 0;
   s->room=room;s->capacity=value(b,installed?1:0,3);s->health=value(b,0,s->capacity);if(!s->capacity&&!installed)s->room=-1;}
  for(int i=0;i<g->door_count;i++){GameDoor*d=&g->doors[i];d->force_ticks=value(b,0,4*GAME_HZ-1);d->force_idle_ticks=value(b,0,GAME_HZ-1);d->jam_ticks=value(b,0,5*GAME_HZ);if(d->jam_ticks&&!d->open)return 0;}
 }
 if(b->version>=8){
  for(int i=0;i<g->crew_count;i++){Crew*r=&g->crew[i];r->species=value(b,0,SPEC_COUNT-1);r->ability_ticks=value(b,0,40*GAME_HZ);r->death_processed=value(b,0,1);
   if(r->health>species_max_health(r->species)||r->move_ticks>=species_move_ticks(r->species)||(r->species!=SPEC_CRYSTAL&&r->ability_ticks)||(r->health>0&&r->death_processed))return 0;
  }
  for(int i=0;i<g->player.room_count;i++)g->player.rooms[i].lockdown_ticks=value(b,0,12*GAME_HZ);
 }
 if(b->version>=9){
  for(int i=0;i<GAME_SYSTEM_COUNT;i++){GameSystemState*s=&g->player.systems[i];s->ion_bars=value(b,0,8);s->ion_ticks=value(b,0,40*GAME_HZ);if((s->ion_bars==0)!=(s->ion_ticks==0))return 0;}
  for(int i=0;i<SUB_COUNT;i++){Subsystem*s=&g->subsystems[0][i];s->ion_bars=value(b,0,8);s->ion_ticks=value(b,0,40*GAME_HZ);if((s->ion_bars==0)!=(s->ion_ticks==0))return 0;}
  g->teleporter[0].ion_bars=value(b,0,8);g->teleporter[0].ion_ticks=value(b,0,40*GAME_HZ);
  if((g->teleporter[0].ion_bars==0)!=(g->teleporter[0].ion_ticks==0)||(!g->teleporter[0].installed&&g->teleporter[0].ion_ticks))return 0;
  g->pilot_ion_ticks[0]=value(b,0,40*GAME_HZ);
  for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][8]=value(b,0,1);
 }else for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][8]=1;
 if(b->version>=10){for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][9]=value(b,0,1);}
 else for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][9]=1;
 if(b->version>=11){for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][10]=value(b,0,1);}
 else for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][10]=1;
 if(b->version<13)g->drone_parts=0; /* Pre-drone saves had no parts (V57 layouts start with their blueprint count). */
 if(b->version>=13){
  DroneControl*d=&g->drones;
  g->drone_parts=value(b,0,1000000);
  int installed=value(b,0,1),room=(int32_t)get(b),capacity=value(b,0,8),health=value(b,0,capacity),power=value(b,0,health);
  int ion_bars=value(b,0,8),ion_ticks=value(b,0,40*GAME_HZ),slot_count=value(b,0,GAME_MAX_DRONES);
  if((ion_bars==0)!=(ion_ticks==0))return 0;
  if(installed){
   if(capacity<2||(!d->installed&&!drones_install(g))||room!=d->room||slot_count!=d->slot_count)return 0;
   d->capacity=capacity;d->health=health;d->power=power;d->ion_bars=ion_bars;d->ion_ticks=ion_ticks;
  }else if(room!=d->room||slot_count!=d->slot_count||capacity||health||power||ion_bars||ion_ticks)return 0;
  for(int i=0;i<GAME_MAX_DRONES;i++){
   int id=value(b,-1,b->version>=15?GAME_DRONE_CATALOG_COUNT-1:0);
   if(id>=0&&(!installed||i>=slot_count))return 0;
   d->slots[i].catalog_id=id;
  }
  if(game_used_power(g)>g->player.reactor)return 0;
 }
 if(b->version>=14){
  for(int i=0;i<CAM_CARGO_COUNT;i++)c->drone_cargo[i]=value(b,-1,b->version>=15?GAME_DRONE_CATALOG_COUNT-1:0);
  for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)for(int j=11;j<14;j++)c->stock[i][j]=value(b,0,j==13?3:1);
 }else{
  for(int i=0;i<CAM_CARGO_COUNT;i++)c->drone_cargo[i]=-1;
  for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)for(int j=11;j<14;j++)c->stock[i][j]=j==13?3:1;
 }
 if(b->version>=15){for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][14]=value(b,0,1);}
 else for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)c->stock[i][14]=1;
 if(b->version>=16){
  c->node_count=value(b,1,CAM_NODE_COUNT);c->sector_count=value(b,1,CAM_SECTOR_COUNT);
  if(!((c->node_count==CAM_LEGACY_NODE_COUNT&&c->sector_count==CAM_LEGACY_SECTOR_COUNT)||(c->node_count==23&&c->sector_count==8)))return 0;
  for(int i=CAM_LEGACY_NODE_COUNT;i<CAM_NODE_COUNT;i++){
   c->visited[i]=value(b,0,1);c->links[i]=value(b,0,(1<<CAM_NODE_COUNT)-1);
   c->node_kind[i]=value(b,0,NODE_EXIT);c->node_y[i]=value(b,0,135);
   for(int j=0;j<CAM_SAVED_OFFERS;j++)c->stock[i][j]=value(b,0,j==7?99:(j<3||j==13)?3:1);
  }
 }
 if(b->version>=17){
  c->route_policy=value(b,0,1);
  for(int i=0;i<CAM_SECTOR_COUNT;i++)c->sector_types[i]=value(b,SECTOR_UNCHARTED,SECTOR_TYPE_COUNT-1);
  for(int i=0;i<2;i++)c->next_sector[i]=value(b,SECTOR_UNCHARTED,SECTOR_TYPE_COUNT-1);
 }
 /* V19/V20/V23/V24 extend policy values without changing the V18 payload layout. */
 c->event_policy=b->version>=18?value(b,0,b->version>=28?6:b->version>=24?5:b->version>=23?4:b->version>=20?3:b->version>=19?2:1):0;
 c->event_seen=b->version>=18?value(b,0,c->event_policy==6?0x7fffffff:c->event_policy==5?4095:c->event_policy==4?2047:c->event_policy==3?511:c->event_policy==2?255:63):0;
 if(b->version>=21){
  if(!decode_intruders(b,g))return 0;
  if(g->enemy_crew_count){
   /* A home-intruder checkpoint has no remote ship to revive on loading. */
   g->enemy.hull=0;g->enemy_departed=1;g->enemy_weapon.enabled=0;g->enemy_weapon.pending_shots=0;
   g->enemy_pending_shots=g->enemy_fire_ticks=g->enemy_shot_ticks=0;g->enemy_drone.power=0;g->teleporter[1].power=0;
  }
 }
 else {memset(g->enemy_crew,0,sizeof g->enemy_crew);g->enemy_crew_count=0;}
 c->boss_resume_phase=b->version>=22?value(b,-1,2):-1;
 if(c->boss_resume_phase>=0&&c->sector!=c->sector_count-1)return 0;
 if(b->version>=25){
  c->finale_policy=value(b,0,1);c->boss_path_len=value(b,0,CAM_BOSS_PATH_MAX);c->boss_loc=value(b,0,CAM_BOSS_PATH_MAX-1);c->boss_move=value(b,0,1);c->boss_reversed=value(b,0,1);c->base_arrived=value(b,0,CAM_BASE_TURNS);c->base_greeted=value(b,0,1);
  for(int i=0;i<CAM_BOSS_PATH_MAX;i++)c->boss_path[i]=value(b,0,CAM_NODE_COUNT-1);
  c->fleet_nodes=get(b);c->fleet_pending=get(b);
  {int final=c->finale_policy&&c->route_policy==1&&c->sector==c->sector_count-1;unsigned all=c->node_count>=32?~0u:(1u<<c->node_count)-1u;

   if(!final&&(c->boss_path_len||c->boss_loc||c->boss_move||c->boss_reversed||c->base_arrived||c->base_greeted||c->fleet_nodes||c->fleet_pending))return 0;
   if(final&&c->boss_path_len&&(c->boss_path_len<2||c->boss_loc>=c->boss_path_len))return 0;
   if((c->fleet_nodes|c->fleet_pending)&~all||((c->fleet_nodes|c->fleet_pending)>>(c->node_count-1))&1u)return 0;
   for(int i=0;i<CAM_BOSS_PATH_MAX;i++){if(i>=c->boss_path_len?c->boss_path[i]!=0:c->boss_path[i]>=c->node_count)return 0;
    if(i>0&&i<c->boss_path_len&&!campaign_linked(c,c->boss_path[i-1],c->boss_path[i]))return 0;}
   if(c->boss_path_len&&c->boss_path[c->boss_reversed?0:c->boss_path_len-1]!=c->node_count-1)return 0;}
 }else{
  /* Older runs already in the final sector keep the interim finale; others adopt V41 on arrival. */
  c->finale_policy=c->route_policy==1&&c->sector<c->sector_count-1;
  c->boss_path_len=c->boss_loc=c->boss_move=c->boss_reversed=c->base_arrived=c->base_greeted=0;memset(c->boss_path,0,sizeof c->boss_path);c->fleet_nodes=c->fleet_pending=0;
 }
 if(b->version>=26){
  c->boss_crew_valid=value(b,0,1);for(int i=0;i<GAME_FLAGSHIP_ROOMS;i++)c->boss_crew[i]=value(b,0,4);
  if(!c->boss_crew_valid)for(int i=0;i<GAME_FLAGSHIP_ROOMS;i++)if(c->boss_crew[i])return 0;
  if(c->boss_crew_valid&&c->sector!=c->sector_count-1)return 0;
 }else{c->boss_crew_valid=0;memset(c->boss_crew,0,sizeof c->boss_crew);}
 c->flagship_original=b->version>=27?value(b,0,1):0; /* Older runs take the DS tuning. */
 memset(c->event_seen_ext,0,sizeof c->event_seen_ext);
 if(b->version>=28)for(int i=0;i<8;i++)c->event_seen_ext[i]=(unsigned)get(b);
 memset(&c->game.player_cloak,0,sizeof c->game.player_cloak);for(int i=0;i<GAME_MAX_AUGMENTS;i++)c->game.augments[i]=-1;
 for(int i=0;i<CAM_NODE_COUNT;i++){c->stock[i][15]=c->stock[i][16]=i<c->node_count;}
 if(b->version>=29){EnemyCloak*k=&c->game.player_cloak;k->capacity=value(b,0,3);k->health=value(b,0,k->capacity);k->power=value(b,0,k->health);k->room=value(b,k->capacity?0:-1,31);if(!k->capacity)k->room=0;
  for(int i=0;i<GAME_MAX_AUGMENTS;i++){c->game.augments[i]=value(b,-1,AUG_COUNT-1);}
  for(int i=0;i<CAM_NODE_COUNT;i++){c->stock[i][15]=value(b,0,1);c->stock[i][16]=value(b,0,1);}}
 memset(c->ach_counter,0,sizeof c->ach_counter);c->ach_flags=0;c->game.tough_low=0;
 if(b->version>=30){for(int i=0;i<4;i++){c->ach_counter[i]=value(b,0,1000000);}
  c->ach_flags=(unsigned)value(b,0,7);c->game.tough_low=value(b,0,1);}
 for(int q=0;q<CAM_QUESTS;q++){c->quest_event[q]=c->quest_node[q]=-1;}
 if(b->version>=31){unsigned hash=get(b);
  for(int q=0;q<CAM_QUESTS;q++){int event=value(b,-1,campaign_event_node_count()-1),node=value(b,-1,CAM_NODE_COUNT-1);
   if(event<0&&node>=0)return 0;
   /* A save from another event catalog drops its open quests rather than misreading them. */
   if(hash==campaign_event_catalog_hash()&&event>=0&&node<c->node_count){c->quest_event[q]=event;c->quest_node[q]=node;}}}
 c->quest_hidden=c->specials_done=0;
 if(b->version>=32){unsigned hidden=get(b),done=get(b);if(hidden>>CAM_QUESTS||done>>16)return 0;
  for(int q=0;q<CAM_QUESTS;q++)if(c->quest_event[q]>=0&&(hidden&(1u<<q)))c->quest_hidden|=1u<<q;
  c->specials_done=done;}
 if(!c->event_policy&&c->event_seen)return 0;
 if(c->event_policy&&(!c->route_policy||c->node_count!=CAM_GENERATED_NODE_COUNT||c->sector_count!=CAM_SECTOR_COUNT))return 0;
 if(!c->route_policy){
  for(int i=0;i<CAM_SECTOR_COUNT;i++)if(c->sector_types[i])return 0;
  if(c->next_sector[0]||c->next_sector[1])return 0;
 }else{
  if(c->node_count!=CAM_GENERATED_NODE_COUNT||c->sector_count!=CAM_SECTOR_COUNT||c->sector_types[0]!=SECTOR_CIVILIAN)return 0;
  for(int i=0;i<CAM_SECTOR_COUNT;i++){
   int type=c->sector_types[i];
   if(i>c->sector){if(type)return 0;}
   else if(i==CAM_SECTOR_COUNT-1){if(type!=SECTOR_FINAL)return 0;}
   /* V63: the hidden Crystal sector (entered by an event in an original-events run). */
   else if((type<SECTOR_CIVILIAN||type>SECTOR_PIRATE)&&!(type==SECTOR_SECRET&&i>0&&c->event_policy==6))return 0;
  }
  if(c->sector<CAM_SECTOR_COUNT-2){
   if(c->next_sector[0]<SECTOR_CIVILIAN||c->next_sector[0]>SECTOR_PIRATE||c->next_sector[1]<SECTOR_CIVILIAN||c->next_sector[1]>SECTOR_PIRATE||c->next_sector[0]==c->next_sector[1])return 0;
  }else if(c->sector==CAM_SECTOR_COUNT-2){if(c->next_sector[0]!=SECTOR_FINAL||c->next_sector[1])return 0;}
  else if(c->next_sector[0]||c->next_sector[1])return 0;
 }
 if(c->current>=c->node_count||c->selected>=c->node_count||c->sector>=c->sector_count)return 0;
 if(!c->visited[0]||!c->visited[c->current]||c->node_kind[0]!=NODE_START||c->node_kind[c->node_count-1]!=NODE_EXIT)return 0;
 unsigned mask=(1u<<c->node_count)-1u,reached=1,seen=1;
 for(int i=0;i<CAM_NODE_COUNT;i++){
  if(i>=c->node_count){
   if(c->visited[i]||c->links[i]||c->node_kind[i]||c->node_y[i])return 0;
   for(int j=0;j<CAM_OFFER_COUNT;j++)if(c->stock[i][j])return 0;
  }else{
   if(!c->links[i]||((unsigned)c->links[i]&~mask)||(c->links[i]&(1<<i))||c->node_y[i]<50||c->node_y[i]>135)return 0;
   if(i>0&&i<c->node_count-1&&(c->node_kind[i]==NODE_START||c->node_kind[i]==NODE_EXIT))return 0;
   for(int j=0;j<c->node_count;j++)if(!!(c->links[i]&(1<<j))!=!!(c->links[j]&(1<<i)))return 0;
  }
 }
 for(int pass=0;pass<c->node_count;pass++)for(int i=0;i<c->node_count;i++){
  if(reached&(1u<<i))reached|=(unsigned)c->links[i];
  if(seen&(1u<<i))for(int j=0;j<c->node_count;j++)if(c->visited[j]&&(c->links[i]&(1<<j)))seen|=1u<<j;
 }
 if(reached!=mask)return 0;
 int visit_count=0;for(int i=0;i<c->node_count;i++)if(c->visited[i]){visit_count++;if(!(seen&(1u<<i)))return 0;}
 if(visit_count>c->sector_jumps+1)return 0;
 drones_reset(g);
 species_refresh_power(g);
 if(!game_alive_crew(g)||!b->ok||b->pos!=b->size)return 0;
 c->imported_node=-1;c->imported_ship=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;c->mode=CAM_MAP;c->event_id=EVENT_NONE;c->event_resolved=1;c->boss_phase=-1;c->pursued=0;g->outcome=GAME_PLAYING;
 snprintf(c->text,sizeof c->text,g->enemy_crew_count?"Campaign restored. Sector %d/%d. Hostile intruders remain aboard; open SHIP to defend your crew.":"Campaign restored. Sector %d/%d. Choose a route or manage the ship.",c->sector+1,c->sector_count);return 1;
}
/* 0 absent, 1 owned magic, -1 foreign/unreadable. Never replace foreign files. */
static int owned(const char *path) {
    unsigned char bytes[12];FILE *f=fopen(path,"rb");size_t n;
    if(!f)return errno==ENOENT?0:-1;
    n=fread(bytes,1,sizeof bytes,f);fclose(f);
    if(n<8||memcmp(bytes,magic,sizeof magic))return -1;
    if(n==12){unsigned version=(unsigned)bytes[8]|((unsigned)bytes[9]<<8)|((unsigned)bytes[10]<<16)|((unsigned)bytes[11]<<24);if(version>32)return -1;}
    return 1;
}
static int read_save(const char *path,Buffer *b) {
    Buffer *header=&work_header;FILE *f;uint32_t size,hash,version;int extra,ok;
    f=fopen(path,"rb");if(!f)return 0;
    header->size=(unsigned)fread(header->bytes,1,SAVE_HEADER,f);
    if(header->size!=SAVE_HEADER || memcmp(header->bytes,magic,8)){fclose(f);return 0;}
    header->pos=8;
    version=get(header);if(version<2||version>32){fclose(f);return 0;}
    size=get(header);hash=get(header);
    if(size>SAVE_CAP || size%4){fclose(f);return 0;}
    memset(b,0,sizeof(*b));b->size=size;b->ok=1;b->version=(int)version;
    ok=fread(b->bytes,1,size,f)==size;extra=fgetc(f);
    if(ferror(f))ok=0;
    if(fclose(f)!=0)ok=0;
    return ok && extra==EOF && checksum(b->bytes,size)==hash;
}
static int valid_save(const char *path) {
    return read_save(path,&work_payload) && decode(&work_payload,&work_campaign);
}
int campaign_load(Campaign *c,const char *path) {
    char backup[300];
    if(!c || !path || !*path || strlen(path)>290)return 0;
    if(valid_save(path)){*c=work_campaign;return 1;}
    snprintf(backup,sizeof backup,"%s.bak",path);
    if(!valid_save(backup))return 0;
    *c=work_campaign;
    snprintf(c->text,sizeof c->text,c->game.enemy_crew_count?"Recovered campaign backup. Hostile intruders remain aboard; open SHIP to defend your crew.":"Recovered the previous valid campaign backup.");
    return 1;
}
int campaign_save(const Campaign *c,const char *path) {
    Buffer *b=&work_payload,*header=&work_header;FILE *f;
    char temp[300],backup[300],bad[300];int existing,ok,primary_valid;
    if(!c || !path || !*path || strlen(path)>290)return 0;
    primary_valid=valid_save(path);
    if(!encode(c,b) || !decode(b,&work_campaign))return 0;
    snprintf(temp,sizeof temp,"%s.tmp",path);snprintf(backup,sizeof backup,"%s.bak",path);snprintf(bad,sizeof bad,"%s.bad",path);
    existing=owned(path);
    if(existing<0 || owned(temp)<0 || owned(backup)<0 || owned(bad)<0)return 0;
    memcpy(header->bytes,magic,8);header->size=8;
    put(header,32);put(header,b->size);put(header,checksum(b->bytes,b->size));
    f=fopen(temp,"wb");if(!f)return 0;
    ok=fwrite(header->bytes,1,header->size,f)==header->size && fwrite(b->bytes,1,b->size,f)==b->size;
    if(fflush(f)!=0)ok=0;
    if(fclose(f)!=0)ok=0;
    if(!ok){remove(temp);return 0;}
    if(existing && !primary_valid) {
        if(owned(bad)==1 && remove(bad)!=0){remove(temp);return 0;}
        if(rename(path,bad)!=0){remove(temp);return 0;}
    } else if(existing) {
        if(owned(backup)==1 && remove(backup)!=0){remove(temp);return 0;}
        if(rename(path,backup)!=0){remove(temp);return 0;}
    }
    if(rename(temp,path)!=0) {
        if(existing)rename(primary_valid?backup:bad,path);
        remove(temp);return 0;
    }
    return 1;
}
