/* V60: original beacon hazards (asteroid field, sun, nebula, ion storm), their
 * compiled events and hazard-beacon pools, and fights starting in them. */
#include "campaign.h"
#include "subsystems.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
static Game g;
static void fight(int env){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy_weapon.cooldown_ticks=g.enemy_weapon_secondary.cooldown_ticks=999999;game_set_environment(&g,env);}
static int fires(const Ship*s){int n=0;for(int i=0;i<s->room_count;i++)n+=s->rooms[i].fires>0;return n;}
int main(void){
 /* Compiled events carry all four base-game hazards; hazard beacons have pools. */
 int seen[5]={0};for(int i=0;i<IMPORTED_EVENT_NODE_COUNT;i++){int e=imported_event_nodes[i].env;assert(e>=0&&e<=4);seen[e]++;}
 assert(seen[IE_ENV_ASTEROID]&&seen[IE_ENV_SUN]&&seen[IE_ENV_NEBULA]&&seen[IE_ENV_STORM]);
 int hazard_pools=0;for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++){hazard_pools+=imported_event_pools[i].category==6;}assert(hazard_pools>=10);
 /* Asteroid field: asteroids keep coming at both ships. */
 fight(ENV_ASTEROID);{int launched=0;for(int t=0;t<20*GAME_HZ;t++){game_step(&g);for(int i=0;i<GAME_MAX_EFFECTS;i++)launched+=g.effects[i].active&&g.effects[i].age==1&&g.effects[i].kind==EFFECT_LASER;}assert(launched>=3);}
 /* Sun: the first flare after 20 s sets fires on both ships. */
 fight(ENV_SUN);assert(g.env_ticks==20*GAME_HZ);for(int t=0;t<20*GAME_HZ;t++){game_step(&g);}assert(fires(&g.player)>0&&fires(&g.enemy)>0&&g.env_ticks>=25*GAME_HZ);
 /* Nebula: even manned level-3 sensors cannot see into the enemy ship. */
 fight(ENV_NONE);g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;{int empty=-1;for(int i=0;i<g.enemy.room_count&&empty<0;i++){int crew=0;for(int k=0;k<g.enemy_crew_count;k++)crew|=g.enemy_crew[k].health>0&&g.enemy_crew[k].room==g.enemy.rooms[i].id;if(!crew)empty=g.enemy.rooms[i].id;}
  assert(empty>=0&&game_room_visible(&g,1,empty)&&game_enemy_details_visible(&g));
  game_set_environment(&g,ENV_NEBULA);assert(!game_room_visible(&g,1,empty)&&!game_enemy_details_visible(&g)&&game_room_visible(&g,0,g.player.rooms[0].id));}
 /* Ion storm: both reactors halve and power is cut to fit. */
 fight(ENV_NONE);{int reactor=g.enemy.reactor;game_set_environment(&g,ENV_STORM);assert(game_reactor(&g)==g.player.reactor/2&&game_used_power(&g)<=game_reactor(&g)&&g.enemy.reactor==reactor/2);
  int total=g.enemy_drone.power+g.teleporter[1].power;for(int i=0;i<GAME_SYSTEM_COUNT;i++){total+=g.enemy.systems[i].power;}assert(total<=g.enemy.reactor);
  while(game_used_power(&g)<game_reactor(&g)){assert(game_adjust_power(&g,SYS_SHIELDS,1)||game_adjust_power(&g,SYS_ENGINES,1)||game_adjust_power(&g,SYS_WEAPONS,1));}
  assert(!game_adjust_power(&g,SYS_OXYGEN,1)||g.player.systems[SYS_OXYGEN].power>=g.player.systems[SYS_OXYGEN].capacity);}
 /* Campaign: a hazard event marks the beacon, its fight starts in the hazard, a jump clears it. */
 {int root=-1,env=0;for(int i=0;i<IMPORTED_EVENT_POOL_COUNT&&root<0;i++)if(imported_event_pools[i].category==6&&imported_event_pools[i].sector==SECTOR_CIVILIAN){root=imported_event_pools[i].root;}
  assert(root>=0);campaign_init_seed(&c,6001);assert(campaign_imported_begin(&c,root));env=c.beacon_env;
  for(int s=0;s<30&&!env;s++){campaign_init_seed(&c,6001+(unsigned)s);if(campaign_imported_begin(&c,root))env=c.beacon_env;}
  assert(env>=ENV_ASTEROID&&env<=ENV_STORM);
  if(c.mode==CAM_COMBAT){assert(c.game.environment==env);}
  campaign_init_seed(&c,6002);c.beacon_env=ENV_SUN;c.mode=CAM_MAP;for(int n=0;n<c.node_count;n++)if(campaign_can_jump(&c,n)){assert(campaign_jump(&c,n));break;}
  assert(c.beacon_env!=ENV_SUN||c.node_kind[c.current]==NODE_HAZARD);}
 /* Hazard beacons draw hazard events in current runs. */
 {int hits=0;for(unsigned seed=6100;seed<6140;seed++){campaign_init_seed(&c,seed);c.mode=CAM_MAP;int target=-1;for(int n=0;n<c.node_count;n++)if(campaign_can_jump(&c,n)){target=n;break;}
   if(target<0){continue;}
   c.node_kind[target]=NODE_HAZARD;c.sector_types[c.sector]=SECTOR_CIVILIAN;if(campaign_jump(&c,target)&&c.beacon_env)hits++;}
  assert(hits>0);printf("hazard arrivals with an environment: %d\n",hits);}
 printf("PASS: asteroid field, solar flares, nebula sensor blackout, ion storm reactor halving, hazard events and beacons (%d asteroid, %d sun, %d nebula, %d storm nodes).\n",seen[1],seen[2],seen[3],seen[4]);
 return 0;
}
