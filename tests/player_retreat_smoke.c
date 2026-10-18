#include "campaign.h"
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;
static void ready(void){c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;assert(game_player_ftl_ready(&c.game));}
static void combat(void){campaign_init_seed(&c,2828);assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT);ready();}
static void destination(int n,int kind,int seen){c.links[c.current]|=1<<n;c.links[n]|=1<<c.current;c.node_kind[n]=kind;c.visited[n]=seen;}
static void unchanged_failure(int n,int abandon){before=c;assert(!campaign_retreat(&c,n,abandon));assert(!memcmp(&c,&before,sizeof c));}
static void home_enemy(void){game_init_crew(&c.game,1,0,c.game.player.systems[SYS_WEAPONS].room);c.game.enemy_crew[0].ship=0;c.game.enemy_crew_count=1;}
static void import_combat(int root){campaign_init_seed(&c,2800+root);assert(campaign_imported_begin(&c,root));for(int i=0;i<5&&c.mode==CAM_EVENT;i++)assert(campaign_choice(&c,0));assert(c.mode==CAM_COMBAT);ready();}
int main(void){
 combat();before=c;assert(campaign_can_retreat(&c,0)&&!memcmp(&c,&before,sizeof c));unchanged_failure(c.current,0);unchanged_failure(-1,0);unchanged_failure(CAM_NODE_COUNT,0);unchanged_failure(0,2);
 c.game.player_ftl_charge=0;unchanged_failure(0,0);ready();c.fuel=0;unchanged_failure(0,0);c.fuel=12;c.links[c.current]&=~1;unchanged_failure(0,0);
 combat();c.jumps=1000000;unchanged_failure(0,0);c.jumps=1;c.sector_jumps=1000000;unchanged_failure(0,0);
 combat();home_enemy();Crew invader=c.game.enemy_crew[0];destination(3,NODE_STORE,0);int fuel=c.fuel,jumps=c.jumps,fleet=c.sector_jumps,scrap=c.scrap;
 assert(campaign_retreat(&c,3,0)&&c.mode==CAM_STORE&&c.current==3&&c.fuel==fuel-1&&c.jumps==jumps+1&&c.sector_jumps==fleet+1&&c.scrap==scrap&&c.boss_phase==-1);
 assert(c.game.enemy_crew_count==GAME_MAX_ENEMY_CREW+1&&!memcmp(&c.game.enemy_crew[GAME_MAX_ENEMY_CREW],&invader,sizeof invader)&&!c.game.enemy.hull);
 campaign_continue(&c);assert(c.mode==CAM_MAP);destination(4,NODE_FIGHT,0);c.event_policy=0;assert(campaign_can_jump(&c,4)&&campaign_jump(&c,4)&&c.mode==CAM_COMBAT&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].id==invader.id);
 /* A new enemy's hull result is not blocked by old carried intruders. */
 c.game.enemy.hull=0;campaign_step(&c);assert(c.game.outcome==GAME_VICTORY&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].health>0);c.result_ticks=1000;campaign_step(&c);assert(c.mode==CAM_EVENT&&c.event_resolved);
 int rewarded=c.scrap;campaign_step(&c);assert(c.scrap==rewarded);
 /* Revisits suppress rewards, and authored unresolved arrivals preserve ownership. */
 combat();destination(0,NODE_START,1);scrap=c.scrap;assert(campaign_retreat(&c,0,0)&&c.mode==CAM_EVENT&&c.event_resolved&&c.scrap==scrap);campaign_continue(&c);assert(c.mode==CAM_MAP);
 combat();home_enemy();destination(2,NODE_SUPPLY,0);assert(campaign_retreat(&c,2,0)&&c.mode==CAM_EVENT&&!c.event_resolved&&c.event_id==EVENT_SUPPLY&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].health>0);
 /* Original event arrival uses a dedicated arrival path, not a fake MAP mode. */
 combat();destination(3,NODE_SUPPLY,0);assert(campaign_retreat(&c,3,0)&&c.mode==CAM_EVENT&&c.event_id==EVENT_IMPORTED&&c.imported_ship==-1);
 combat();home_enemy();destination(3,NODE_STORE,0);c.sector_jumps=30;assert(campaign_retreat(&c,3,0)&&c.mode==CAM_COMBAT&&c.pursued&&c.game.enemy_kind==ENEMY_INTERCEPTOR&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].health>0);
 /* Awaiting confirmation does not disable destination selection; cancel is pure. */
 combat();home_enemy();c.game.crew[1].ship=1;c.game.crew[1].room=c.game.crew[1].destination=c.game.enemy.rooms[0].id;unsigned lost=c.game.crew[1].id;
 assert(campaign_can_retreat(&c,0));unchanged_failure(0,0);assert(campaign_retreat(&c,0,1));assert(c.game.crew[1].id==lost&&!c.game.crew[1].health&&!c.game.crew[1].ship&&c.game.crew[1].death_processed&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].health>0);
 /* Fleeing original encounters never dispatches source rewards or pursuit. */
 for(int root=6;root<=8;root++){import_combat(root);destination(3,NODE_STORE,0);fuel=c.fuel;scrap=c.scrap;fleet=c.sector_jumps;int missiles=c.game.missiles,seen=c.event_seen;
  assert(campaign_retreat(&c,3,0)&&c.fuel==fuel-1&&c.scrap==scrap&&c.game.missiles==missiles&&c.sector_jumps==fleet+1&&c.event_seen==seen);
  assert(c.imported_ship==-1&&c.imported_node==-1&&!c.imported_battle);campaign_continue(&c);assert(c.scrap==scrap);
 }
 /* Patrol retreat is ordinary travel, with no emergency-fuel reward. */
 combat();c.pursued=1;fuel=c.fuel;assert(campaign_retreat(&c,0,0)&&c.fuel==fuel-1);
 /* Current authored boss retries its unfinished phase, with no repeat cache. */
 campaign_init_seed(&c,2829);c.sector=c.sector_count-1;c.current=c.selected=c.node_count-1;c.visited[c.current]=1;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;c.game.player.hull=15;
 assert(campaign_choice(&c,0)&&c.boss_phase==0&&c.boss_resume_phase==0);int hull=c.game.player.hull,missiles=c.game.missiles;ready();destination(0,NODE_START,1);
 assert(campaign_retreat(&c,0,0)&&c.boss_phase==-1&&c.boss_resume_phase==0&&c.game.player.hull==hull&&c.game.missiles==missiles);campaign_continue(&c);assert(c.mode==CAM_MAP);
 destination(c.node_count-1,NODE_EXIT,1);assert(campaign_jump(&c,c.node_count-1)&&c.event_id==EVENT_EXIT&&!c.event_resolved);assert(campaign_choice(&c,0)&&c.boss_phase==0&&c.boss_resume_phase==0&&c.game.player.hull==hull&&c.game.missiles==missiles);
 c.game.enemy.hull=0;c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_HULL;c.result_ticks=1000;campaign_step(&c);campaign_continue(&c);assert(c.boss_phase==1&&c.boss_resume_phase==1&&c.game.missiles==missiles+2);
 ready();hull=c.game.player.hull;missiles=c.game.missiles;assert(campaign_retreat(&c,0,0)&&c.boss_resume_phase==1&&c.boss_phase==-1&&c.game.player.hull==hull&&c.game.missiles==missiles);
 campaign_continue(&c);assert(campaign_jump(&c,c.node_count-1)&&campaign_choice(&c,0)&&c.boss_phase==1&&c.game.player.hull==hull&&c.game.missiles==missiles);
 puts("Player retreat: atomic failed/canceled actions, every arrival class, carried intruders, explicit away loss, original/patrol reward suppression and same-phase boss retry without duplicate supplies passed");return 0;
}
