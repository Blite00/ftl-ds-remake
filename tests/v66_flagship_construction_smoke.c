/* V66: the flagship under construction (Rebel sector special) unlocks the Federation Cruiser. */
#include "campaign.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
int main(void){
 int root=-1,pool=0;
 for(int i=0;i<IMPORTED_EVENT_ROOT_COUNT;i++)if(!strcmp(imported_event_roots[i].id,"FLAGSHIP_CONSTRUCTION"))root=i;
 for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++)pool|=imported_event_pools[i].category==7&&imported_event_pools[i].root==root&&imported_event_pools[i].sector==SECTOR_REBEL;
 assert(root>=0&&pool);
 {int ship=-1;for(int i=0;i<game_ship_count();i++)if(!strcmp(game_ship_blueprint(i),"BOSS_SPECIAL"))ship=i;assert(ship>=0);
  /* V67: its two power-1 artillery mounts (ARTILLERY_BOSS_1/2) fire, crewless, at the player. */
  static Game g;game_init(&g);g.enemy_ship=ship;assert(game_begin_encounter(&g,ENEMY_SHIP,3,0)&&g.artillery_count==2);
  for(int i=0;i<2;i++){assert(g.artillery[i].weapon==i&&g.artillery[i].power==1&&game_artillery_manned(&g,i));}
  g.player.systems[SYS_SHIELDS].power=0;g.player.shields=0;g.player.systems[SYS_ENGINES].power=0;
  {int hull=g.player.hull;for(int t=0;t<60*GAME_HZ&&g.outcome==GAME_PLAYING;t++){game_step(&g);}assert(g.player.hull<hull);}}
 int unlocked=0;
 for(unsigned seed=6601;seed<6640&&!unlocked;seed++){
  campaign_init_seed(&c,seed);c.sector=3;c.sector_types[3]=SECTOR_REBEL;c.mode=CAM_MAP;
  if(!campaign_imported_begin(&c,root))continue;
  /* Accept the fight, destroy the half-built flagship, then continue through the aftermath. */
  for(int guard=0;guard<20&&c.mode==CAM_EVENT&&!c.event_resolved;guard++){if(!campaign_choice(&c,0))break;}
  if(c.mode!=CAM_COMBAT)continue;
  assert(!strcmp(game_enemy_name(&c.game),"Rebel Flagship")||game_enemy_import_index(&c.game)>=0);
  c.game.enemy.hull=0;for(int t=0;t<5*GAME_HZ&&c.mode==CAM_COMBAT;t++){campaign_step(&c);}
  for(int guard=0;guard<20&&(c.reward.kind||(c.mode==CAM_EVENT&&!c.event_resolved));guard++){
   if(c.reward.kind){rewards_decline(&c.reward);continue;}
   if(!campaign_choice(&c,0))break;}
  unlocked=!!(c.unlock_pending&(1u<<8));
 }
 assert(unlocked);
 printf("PASS: flagship under construction: Rebel special, BOSS_SPECIAL hull and artillery, win unlocks the Federation Cruiser (layout 8).\n");
 return 0;
}
