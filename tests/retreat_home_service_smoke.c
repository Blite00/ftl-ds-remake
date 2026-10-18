#include "campaign.h"
#include "species.h"
#include "fire.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;
static void same_story(void){size_t offset=offsetof(Campaign,mode);assert(!memcmp((const unsigned char*)&c+offset,(const unsigned char*)&before+offset,sizeof c-offset));}
static Crew*intruder(void){game_init_crew(&c.game,1,55,c.game.pilot_room);c.game.enemy_crew[55].ship=0;c.game.enemy_crew_count=56;return &c.game.enemy_crew[55];}
static void arrival(int original){
 /* Fix the fixture narrative policy: authored arrival or historical original pool.
  * New reward roots must not randomly replace this unresolved-choice scenario. */
 campaign_init_seed(&c,2830+original);c.event_policy=original?3:0;assert(campaign_jump(&c,1));intruder();c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;
 int n=original?3:2;c.links[1]|=1<<n;c.links[n]|=1<<1;c.visited[n]=0;c.node_kind[n]=NODE_SUPPLY;
 assert(campaign_retreat(&c,n,0)&&c.mode==CAM_EVENT&&!c.event_resolved);
 assert(c.event_id==(original?EVENT_IMPORTED:EVENT_SUPPLY));
}
int main(void){
 for(int original=0;original<=1;original++){
  arrival(original);assert(campaign_can_service_home(&c)&&campaign_home_threat(&c));int choices=campaign_choice_count(&c),enabled[4];char labels[4][115];
  assert(choices>0&&choices<=4);for(int i=0;i<choices;i++){enabled[i]=campaign_choice_enabled(&c,i);snprintf(labels[i],sizeof labels[i],"%s",campaign_choice_name(&c,i));}
  before=c;assert(!campaign_upgrade(&c,SYS_WEAPONS)&&!campaign_purchase(&c,0)&&!campaign_install_teleporter(&c));assert(!memcmp(&c,&before,sizeof c));campaign_step(&c);assert(!memcmp(&c,&before,sizeof c));
  c.game.enemy_crew[55].combat_ticks=c.game.crew[0].combat_ticks=GAME_HZ-1;
  c.game.player_ftl_charge=123;c.game.enemy_escape_ticks=456;c.game.enemy_weapon.charge_ticks=789;before=c;
  assert(campaign_service_home(&c));same_story();assert(c.game.ticks==before.game.ticks+1&&c.game.crew[0].health<before.game.crew[0].health&&c.game.enemy_crew[55].health<before.game.enemy_crew[55].health);
  assert(c.game.player_ftl_charge==123&&c.game.enemy_escape_ticks==456&&c.game.enemy_weapon.charge_ticks==789&&!memcmp(&c.game.enemy,&before.game.enemy,sizeof c.game.enemy));
  assert(campaign_choice_count(&c)==choices);for(int i=0;i<choices;i++)assert(campaign_choice_enabled(&c,i)==enabled[i]&&!strcmp(labels[i],campaign_choice_name(&c,i)));
  c.game.paused=1;before=c;assert(!campaign_service_home(&c)&&!memcmp(&c,&before,sizeof c));c.game.paused=0;
  /* Service remains available after the intruder dies, without resolving story. */
  c.game.enemy_crew[55].health=0;c.game.enemy_crew[55].death_processed=1;c.game.player.systems[SYS_WEAPONS].health=1;c.game.crew[2].repair_ticks=5*GAME_HZ-1;
  before=c;assert(!campaign_home_threat(&c)&&campaign_can_service_home(&c)&&campaign_service_home(&c));same_story();assert(c.game.player.systems[SYS_WEAPONS].health==2);
 }
 /* Home fire continues during explicit defense; narrative rewards remain held. */
 arrival(0);int room=c.game.player.rooms[0].id;fire_ignite(&c.game.player,room,1);c.game.player.rooms[0].fire_ticks=GAME_HZ-1;before=c;
 assert(campaign_service_home(&c));same_story();assert(c.game.player.rooms[0].oxygen<before.game.player.rooms[0].oxygen);
 /* Active combat and its frozen warning are not arrival-service contexts. */
 campaign_init_seed(&c,2832);assert(campaign_jump(&c,1));before=c;assert(!campaign_can_service_home(&c)&&!campaign_service_home(&c)&&!memcmp(&c,&before,sizeof c));
 campaign_init_seed(&c,2833);assert(campaign_imported_begin(&c,8)&&campaign_choice(&c,0));assert(c.imported_battle==4);before=c;assert(!campaign_can_service_home(&c)&&!campaign_service_home(&c)&&!memcmp(&c,&before,sizeof c));
 arrival(0);c.game.crew[1].ship=1;before=c;assert(!campaign_can_service_home(&c)&&!campaign_service_home(&c)&&!memcmp(&c,&before,sizeof c));
 /* Home defeat overrides an unresolved event, without executing its choices. */
 arrival(1);c.game.crew_count=1;c.game.crew[0].health=1;c.game.enemy_crew[55].combat_ticks=GAME_HZ-1;int node=c.imported_node,ship=c.imported_ship,seen=c.event_seen,fuel=c.fuel,scrap=c.scrap;
 assert(campaign_service_home(&c)&&c.mode==CAM_DEFEAT&&c.game.outcome==GAME_DEFEAT&&c.game.result_reason==RESULT_PLAYER_CREW);
 assert(c.imported_node==node&&c.imported_ship==ship&&c.event_seen==seen&&c.fuel==fuel&&c.scrap==scrap&&!campaign_choice_count(&c));before=c;assert(!campaign_service_home(&c)&&!memcmp(&c,&before,sizeof c));
 arrival(0);c.game.player.hull=0;c.game.paused=1;assert(campaign_service_home(&c)&&c.mode==CAM_DEFEAT&&c.game.result_reason==RESULT_PLAYER_HULL);
 puts("Arrival home service: authored/original context and choices preserved, explicit clock without fleet/rewards/remote/FTL advancement, pause, repairs after intruders, fire, scope guards and defeat precedence passed");return 0;
}
