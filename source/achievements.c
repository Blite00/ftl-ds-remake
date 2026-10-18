#include "achievements.h"
#include "achievement_catalog.h"
#include "drones.h"
#include "subsystems.h"
#include "species.h"
#include <string.h>
typedef char achievement_catalog_matches[IMPORTED_ACHIEVEMENT_COUNT==ACH_COUNT?1:-1];
static int ship_class(const Game*g){return g->player_ship/2;}
const char*achievement_name(int id){return id>=0&&id<ACH_COUNT?imported_achievements[id].name:"";}
const char*achievement_desc(int id){return id>=0&&id<ACH_COUNT?imported_achievements[id].desc:"";}
void achievements_earn(Game*g,int id){if(id>=0&&id<ACH_COUNT&&id/3==ship_class(g))g->ach.earned|=1u<<id;}
int achievements_enemy_alive(const Game*g){int n=0;for(int i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)n+=g->enemy_crew[i].health>0;return n;}
static unsigned enemy_mask(const Game*g){unsigned m=0;for(int i=0;i<g->enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)if(g->enemy_crew[i].health>0)m|=1u<<i;return m;}
static unsigned crew_mask(const Game*g){unsigned m=0;for(int i=0;i<g->crew_count&&i<GAME_MAX_CREW;i++)if(g->crew[i].health>0)m|=1u<<i;return m;}
void achievements_begin_encounter(Game*g){unsigned earned=g->ach.earned;memset(&g->ach,0,sizeof g->ach);g->ach.earned=earned;
 g->ach.enemy_alive=enemy_mask(g);g->ach.crew_alive=crew_mask(g);g->ach.start_shield=g->player_super_shield;g->ach.prev_hull=g->player.hull;}
static const GameRoom*find_room(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
static int installed_systems(const Game*g){int n=1; /* The cockpit is always installed. */
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)n+=g->player.systems[i].capacity>0;
 for(int i=0;i<SUB_COUNT;i++)n+=g->subsystems[0][i].capacity>0;
 return n+g->teleporter[0].installed+g->drones.installed+(g->player_cloak.capacity>0)+(g->player_artillery.capacity>0);}
static int enemy_ioned(const Game*g){int n=g->pilot_ion_ticks[1]>0;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)n+=g->enemy.systems[i].capacity>0&&g->enemy.systems[i].ion_bars>0;
 for(int i=0;i<SUB_COUNT;i++)n+=g->subsystems[1][i].ion_bars>0;
 for(int i=0;i<g->artillery_count;i++)n+=g->artillery[i].ion_bars>0;
 return n+(g->enemy_drone.ion_bars>0)+(g->enemy_cloak.ion_bars>0)+(g->teleporter[1].ion_bars>0);}
static int total_power(const Game*g){int n=g->drones.power+g->drones.bonus_power+g->teleporter[0].power+g->teleporter[0].bonus_power+g->player_cloak.power+g->player_artillery.power;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++){n+=g->player.systems[i].power+g->player.systems[i].bonus_power;}
 return n;}
void achievements_effect_resolved(Game*g,const GameEffect*e,int hull_before,int alive_before){GameAchTrack*a=&g->ach;
 if(e->side==0){if(hull_before>0&&g->enemy.hull<=0)a->killing_origin=e->origin;
  if(e->origin==ORIGIN_BIO&&alive_before-achievements_enemy_alive(g)>=3){achievements_earn(g,ACH_SLUG_BIO);}
  return;}
 if(e->result==EFFECT_MISS&&game_player_cloaked(g)){a->cloak_avoided+=e->damage>0?e->damage:0;if(a->cloak_avoided>=9)achievements_earn(g,ACH_STEALTH_AVOID);}}
void achievements_step(Game*g){GameAchTrack*a=&g->ach;int cls=ship_class(g);unsigned em,cm,dead;
 if(g->player.hull<=0)return;
 if(g->player.hull<a->prev_hull)a->hull_hit=1;
 a->prev_hull=g->player.hull;
 /* Kestrel: back to full hull after dropping to 1 HP (latched across the run). */
 if(g->player.hull==1)g->tough_low=1;
 if(g->tough_low&&g->player.hull>=g->player.max_hull){achievements_earn(g,ACH_KESTREL_TOUGH);if(cls==0)g->tough_low=0;}
 em=enemy_mask(g);cm=crew_mask(g);dead=a->enemy_alive&~em;
 for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(dead&(1u<<i)){const Crew*c=&g->enemy_crew[i];const GameRoom*r;a->enemy_kills++;
  /* Rock: crew kill a burning enemy aboard its own ship. */
  if(c->ship==1&&(r=find_room(&g->enemy,c->room))&&r->fires>0)for(int k=0;k<g->crew_count;k++)if(g->crew[k].health>0&&g->crew[k].ship==1&&g->crew[k].room==c->room)achievements_earn(g,ACH_ROCK_FIRE);}
 if(a->crew_alive&~cm)a->crew_lost=1;
 /* Mantis: the last enemy falls to your last crewmember, aboard their ship. */
 if(dead&&!em&&!g->enemy.automated){int alive=0,away=0;for(int k=0;k<g->crew_count;k++)if(g->crew[k].health>0){alive++;away=g->crew[k].ship==1;}
  if(alive==1&&away)achievements_earn(g,ACH_MANTIS_SURVIVOR);}
 a->enemy_alive=em;a->crew_alive=cm;
 if(a->enemy_kills>=5&&!a->hull_hit&&!a->crew_lost)achievements_earn(g,ACH_MANTIS_SLAUGHTER);
 if(cls==0){int seen=0,n=0;for(int k=0;k<g->crew_count;k++)if(g->crew[k].health>0&&!(seen&(1<<g->crew[k].species))){seen|=1<<g->crew[k].species;n++;}
  if(n>=6)achievements_earn(g,ACH_KESTREL_FEDERATION);
  if(installed_systems(g)>=11)achievements_earn(g,ACH_KESTREL_ARSENAL);}
 if(cls==3){int n=0;for(int i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++)n+=g->drones.slots[i].deployed&&drones_powered(g,i);
  if(n>=3)achievements_earn(g,ACH_ENGI_ROBOTIC);
  if(g->enemy.hull>0&&enemy_ioned(g)>=4)achievements_earn(g,ACH_ENGI_IONED);}
 if(cls==5&&g->enemy.hull>0&&g->enemy.room_count>0&&game_sensor_level(g)<=0){int all=1;for(int i=0;i<g->enemy.room_count&&all;i++)all=game_room_visible(g,1,g->enemy.rooms[i].id);if(all)achievements_earn(g,ACH_SLUG_VISION);}
 if(cls==7&&total_power(g)>=29)achievements_earn(g,ACH_ZOLTAN_POWER);
 if(cls==8)for(int s=0;s<2;s++){const Ship*sh=s?&g->enemy:&g->player;for(int i=0;i<sh->room_count;i++)if(sh->rooms[i].lockdown_ticks>0){int n=0;
  for(int k=0;k<g->enemy_crew_count&&k<GAME_MAX_ENEMY_CREW;k++)n+=g->enemy_crew[k].health>0&&g->enemy_crew[k].ship==s&&g->enemy_crew[k].room==sh->rooms[i].id;
  if(n>=4)achievements_earn(g,ACH_CRYSTAL_LOCKDOWN);}}
 if(a->start_shield>0&&g->player_super_shield<=0)a->zoltan_broken=1;
 for(int k=0;k<g->flagship_drone_count;k++)if(g->flagship_drones[k].kind==FDRONE_DEFENSE&&g->flagship_drones[k].deployed)a->defense_drone=1;
 /* Ship destroyed: checks that look at how the fight was won. */
 if(!a->ended&&g->enemy.hull<=0&&g->enemy.room_count>0){a->ended=1;
  if(game_player_cloaked(g)&&a->cloak_full)achievements_earn(g,ACH_STEALTH_DESTROY);
  if(a->used==USED_DRONE)achievements_earn(g,ACH_ENGI_ONLY_DRONES);
  if(a->used==USED_ARTILLERY&&!a->hull_hit)achievements_earn(g,ACH_FED_PATIENCE);
  if(a->defense_drone&&a->used==USED_MISSILE)achievements_earn(g,ACH_ROCK_MISSILES);
  if(a->start_shield>0&&!a->zoltan_broken)achievements_earn(g,ACH_ZOLTAN_SHIELDS);
  if(a->killing_origin==ORIGIN_SHARD)achievements_earn(g,ACH_CRYSTAL_SHARD);}
}
