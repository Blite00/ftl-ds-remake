/* Focused source-closure fixtures: terminal combat states are injected.
 * This test proves event integration, not an earned combat/campaign run. */
#include "campaign.h"
#include "outcome.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,b,branch;
static void init(unsigned seed){campaign_init_seed(&c,seed);assert(c.event_policy==6);c.event_policy=5;}
static void pending_stable(void){
 b=c;
 for(int i=0;i<8;i++){
  char hint[128];campaign_choice_hint(&c,0,hint,sizeof hint);
  (void)campaign_choice_count(&c);(void)campaign_choice_name(&c,0);
  assert(!campaign_choice_enabled(&c,0));assert(!campaign_choice(&c,0));
  (void)rewards_can_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,-1,0);
  assert(!campaign_imported_begin(&c,9));assert(!campaign_jump(&c,1));
  assert(!campaign_wait(&c));campaign_continue(&c);campaign_step(&c);
 }
 assert(!memcmp(&b,&c,sizeof c));
}
static void full_crew(void){for(int i=c.game.crew_count;i<GAME_MAX_CREW;i++)game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);c.game.crew_count=GAME_MAX_CREW;}
static void find_weapon(void){
 unsigned seen=0,intros=0;
 const ImportedEventNode*root=&imported_event_nodes[imported_event_roots[9].node];
 for(unsigned seed=1;seed<=256;seed++){
  init(seed);assert(campaign_imported_begin(&c,9));
  assert(c.mode==CAM_EVENT&&c.event_resolved&&c.reward.kind==REWARD_WEAPON);
  seen|=1u<<c.reward.weapon;
  for(int i=0;i<root->alt_count;i++)if(c.imported_node==imported_event_alternatives[root->first_alt+i])intros|=1u<<i;
  pending_stable();int offered=c.reward.weapon;assert(rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,-1,0));
  assert(c.cargo[0]==offered&&!c.reward.kind);campaign_continue(&c);assert(c.mode==CAM_MAP);
 }
 assert(seen==127&&intros==63);
 init(42);for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=i;
 assert(campaign_imported_begin(&c,9));pending_stable();b=c;
 assert(!rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,-1,0));assert(!memcmp(&b,&c,sizeof c));
 assert(!rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,2,0));assert(!memcmp(&b,&c,sizeof c));
 int offered=c.reward.weapon;assert(rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,2,1));assert(c.cargo[2]==offered&&c.cargo[0]==0&&c.cargo[1]==1&&c.cargo[3]==3);
}
static int rescue(unsigned seed,int reason){
 init(seed);c.game.player.hull=20;
 assert(campaign_imported_begin(&c,10)&&campaign_choice_count(&c)==2);
 assert(campaign_choice(&c,0)&&c.imported_battle==1);assert(campaign_choice(&c,0)&&c.mode==CAM_COMBAT);
 c.game.outcome=GAME_VICTORY;c.game.result_reason=reason;c.game.paused=0;
 if(reason==RESULT_ENEMY_HULL)c.game.enemy.hull=0;
 for(int i=0;i<outcome_duration(&c.game)+3&&c.mode==CAM_COMBAT;i++)campaign_step(&c);
 assert(c.mode==CAM_EVENT&&campaign_choice_count(&c)==1&&!c.event_resolved);
 const ImportedEventShip*ship=&imported_event_ships[c.imported_ship];
 const ImportedEventNode*out=&imported_event_nodes[reason==RESULT_ENEMY_HULL?ship->destroyed:ship->dead_crew];
 const ImportedEventNode*list=&imported_event_nodes[imported_event_choices[out->first_choice].next];
 int scrap=c.scrap,fuel=c.fuel,ammo=c.game.missiles;
 assert(campaign_choice(&c,0));
 int alt=-1;for(int i=0;i<list->alt_count;i++)if(c.imported_node==imported_event_alternatives[list->first_alt+i])alt=i;
 assert(alt>=0&&alt<6);
 if(alt==1||alt==2){assert(c.scrap>=scrap+(alt==1?20:10)&&c.scrap<=scrap+(alt==1?30:20));assert(c.fuel>=fuel+1&&c.fuel<=fuel+3&&c.game.missiles>=ammo+1&&c.game.missiles<=ammo+3);}
 else assert(c.scrap==scrap&&c.fuel==fuel&&c.game.missiles==ammo);
 assert(c.game.player.hull==(alt==4?25:20));return alt;
}
static void civilian(void){
 unsigned seen[2]={0,0},species=0;int full_checked=0;
 for(int path=0;path<2;path++)for(unsigned seed=1;seed<=256;seed++){
  int alt=rescue(seed,path?RESULT_ENEMY_CREW:RESULT_ENEMY_HULL);seen[path]|=1u<<alt;
  if(alt==0){
   assert(campaign_choice_count(&c)==2);branch=c;
   assert(campaign_choice(&c,1)&&c.event_resolved&&!c.reward.kind);assert(c.game.next_crew_id==branch.game.next_crew_id);
   campaign_continue(&c);assert(c.mode==CAM_MAP);c=branch;
   if(!full_checked)full_crew();
   unsigned id=c.game.next_crew_id;assert(campaign_choice(&c,0)&&c.reward.kind==REWARD_CREW);
   assert(c.reward.recruit.id==id&&c.game.next_crew_id==id+1&&c.reward.recruit.species<6);species|=1u<<c.reward.recruit.species;pending_stable();
   if(!full_checked){
    b=c;assert(!rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,-1,0));assert(!memcmp(&b,&c,sizeof c));
    assert(!rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,0,0));assert(!memcmp(&b,&c,sizeof c));
    assert(rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,0,1)&&c.game.crew[0].id==id);full_checked=1;
   }else{assert(rewards_accept(&c.reward,&c.game,c.cargo,CAM_CARGO_COUNT,-1,0));assert(c.game.crew[3].id==id);}
  }else if(alt==3){assert(campaign_choice_count(&c)==1&&campaign_choice(&c,0)&&c.reward.kind==REWARD_WEAPON);pending_stable();assert(rewards_decline(&c.reward));}
  else assert(c.event_resolved&&!c.reward.kind);
  int scrap=c.scrap;unsigned next=c.game.next_crew_id;campaign_continue(&c);assert(c.mode==CAM_MAP);assert(c.scrap==scrap&&c.game.next_crew_id==next);
 }
 assert(seen[0]==63&&seen[1]==63&&species==63&&full_checked);
 init(8);b=c;assert(campaign_imported_begin(&c,10));assert(campaign_choice(&c,1)&&c.event_resolved&&!c.reward.kind);assert(c.scrap==b.scrap&&c.fuel==b.fuel&&c.game.next_crew_id==b.game.next_crew_id);campaign_continue(&c);assert(c.mode==CAM_MAP);
}
static void policy(void){
 assert(campaign_imported_count()>=12);
 for(int root=9;root<=10;root++){
  init(4);c.event_policy=3;b=c;assert(!campaign_imported_begin(&c,root)&&!memcmp(&b,&c,sizeof c));
  c.event_policy=4;assert(campaign_imported_begin(&c,root));assert(c.event_seen&(1<<root));
 }
}
int main(void){policy();find_weapon();civilian();puts("Reward events: original six intros and all six civilian outcomes/both callbacks; recruit yes/no, full inventory/crew confirmation, stable pending offers and policy guards passed (injected terminal fixtures, not earned runs).");return 0;}
