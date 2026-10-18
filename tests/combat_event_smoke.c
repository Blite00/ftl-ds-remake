/* Focused fixtures force terminal states; these are not earned campaign runs. */
#include "campaign.h"
#include "outcome.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { int fuel,scrap,ammo,parts,hull; } Supplies;
static Supplies supplies(const Campaign*c){Supplies s={c->fuel,c->scrap,c->game.missiles,c->game.drone_parts,c->game.player.hull};return s;}
static void unchanged(const Campaign*c,Supplies s){assert(c->fuel==s.fuel&&c->scrap==s.scrap&&c->game.missiles==s.ammo&&c->game.drone_parts==s.parts&&c->game.player.hull==s.hull);}
static void init(Campaign*c,unsigned seed){campaign_init_seed(c,seed);c->sector=3;assert(c->event_policy==6);c->event_policy=2;}
static void stage(Campaign*c,int root,unsigned seed){
 init(c,seed);assert(campaign_imported_begin(c,root));
 assert(c->mode==CAM_EVENT&&!c->event_resolved&&campaign_choice_count(c)==2);
 assert(c->imported_ship>=0&&c->imported_battle==0);
 Supplies s=supplies(c);assert(campaign_choice(c,0));unchanged(c,s);
 assert(c->mode==CAM_EVENT&&!c->event_resolved&&c->imported_battle==1);
 assert(campaign_choice_count(c)==1&&!strcmp(campaign_choice_name(c,0),"Engage"));
 assert(strlen(campaign_text(c))>20&&!campaign_choice_blue(c,0));
 Campaign before=*c;char hint[128];
 for(int i=0;i<4;i++){assert(campaign_choice_enabled(c,0));campaign_choice_hint(c,0,hint,sizeof hint);assert(!hint[0]);}
 assert(!memcmp(c,&before,sizeof *c));
}
static void launch(Campaign*c,int root,unsigned seed){stage(c,root,seed);assert(campaign_choice(c,0));assert(c->mode==CAM_COMBAT&&c->imported_battle==2&&c->result_ticks==0);Campaign before=*c;assert(!campaign_choice(c,0)&&!memcmp(c,&before,sizeof *c));}
static void finish(Campaign*c,int reason){
 c->game.outcome=GAME_VICTORY;c->game.result_reason=reason;c->game.paused=0;
 if(reason==RESULT_ENEMY_HULL)c->game.enemy.hull=0;
 else for(int i=0;i<c->game.enemy_crew_count;i++)c->game.enemy_crew[i].health=0;
 for(int i=0;i<2*GAME_HZ+3&&c->mode==CAM_COMBAT;i++)campaign_step(c);
 assert(c->mode==CAM_EVENT);
}
static void peaceful_and_policy(void){
 assert(campaign_imported_count()>=12);
 assert(!strcmp(campaign_imported_id(6),"AUTO_REFUEL_STATION"));
 assert(!strcmp(campaign_imported_id(7),"SQUAT_REFUEL_STATION"));
 for(int root=6;root<=7;root++){
  Campaign c,b;init(&c,26);c.event_policy=1;b=c;
  assert(!campaign_imported_begin(&c,root)&&!memcmp(&c,&b,sizeof c));
  c.event_policy=2;Supplies s=supplies(&c);assert(campaign_imported_begin(&c,root));
  assert(campaign_choice(&c,1)&&c.event_resolved&&c.mode==CAM_EVENT);unchanged(&c,s);
  assert(c.imported_battle==0);campaign_continue(&c);unchanged(&c,s);
  assert(c.mode==CAM_MAP&&c.imported_node==-1&&c.imported_ship==-1&&c.imported_battle==0);
  assert(c.event_seen&(1<<root));b=c;assert(!campaign_imported_begin(&c,root)&&!memcmp(&c,&b,sizeof c));
 }
 Campaign c,b;init(&c,26);c.game.crew[0].ship=1;b=c;
 assert(!campaign_imported_begin(&c,6)&&!memcmp(&c,&b,sizeof c));
}
static void pools_and_launch_guard(void){
 unsigned seen_auto=0,seen_rebel=0;
 for(unsigned seed=1;seed<=64;seed++)for(int root=6;root<=7;root++){
  Campaign c;launch(&c,root,seed);
  if(root==6){assert(c.game.enemy_kind==ENEMY_SCOUT||c.game.enemy_kind==ENEMY_ASSAULT);assert(c.game.enemy.automated);seen_auto|=1u<<(c.game.enemy_kind==ENEMY_ASSAULT);}
  else{assert(c.game.enemy_kind==ENEMY_REBEL_FAT||c.game.enemy_kind==ENEMY_REBEL);assert(!c.game.enemy.automated&&c.game.enemy_crew_count>0);seen_rebel|=1u<<(c.game.enemy_kind==ENEMY_REBEL_FAT);}
 }
 assert(seen_auto==3&&seen_rebel==3);
 Campaign c,b;stage(&c,7,1);c.game.crew[0].ship=1;b=c;
 assert(!campaign_choice_enabled(&c,0)&&!campaign_choice(&c,0)&&!memcmp(&c,&b,sizeof c));
 c.game.crew[0].ship=0;assert(campaign_choice(&c,0)&&c.mode==CAM_COMBAT);
}
static void outcomes(void){
 for(unsigned seed=1;seed<=32;seed++)for(int path=0;path<3;path++){
  int root=path?7:6,reason=path==2?RESULT_ENEMY_CREW:RESULT_ENEMY_HULL;
  Campaign c;launch(&c,root,seed);Supplies s=supplies(&c);finish(&c,reason);
  assert(!c.event_resolved&&campaign_choice_count(&c)==1&&!strcmp(campaign_choice_name(&c,0),"Continue..."));
  assert(strstr(campaign_text(&c),path==2?"With the crew dead":"The ship breaks apart"));
  assert(c.fuel>=s.fuel+1&&c.fuel<=s.fuel+3&&c.game.missiles>=s.ammo+1&&c.game.missiles<=s.ammo+3);
  assert(c.scrap>=s.scrap+(path==0?10:path==1?20:30)&&c.scrap<=s.scrap+(path==0?20:path==1?30:45));
  assert(c.game.drone_parts==s.parts&&c.game.player.hull==s.hull);
  assert(outcome_ship(&c.game)==(path==2?-1:1));
  Supplies first=supplies(&c);campaign_continue(&c);unchanged(&c,first);assert(!c.event_resolved);
  campaign_step(&c);unchanged(&c,first);assert(campaign_choice(&c,0)&&c.event_resolved);
  assert(c.fuel>=first.fuel+(root==6?1:3)&&c.fuel<=first.fuel+(root==6?3:6));
  assert(c.scrap>=first.scrap+(root==6?20:15)&&c.scrap<=first.scrap+(root==6?30:25));
  if(root==6)assert(c.game.missiles>=first.ammo+1&&c.game.missiles<=first.ammo+3);
  else assert(c.game.missiles==first.ammo);
  Supplies last=supplies(&c);Campaign b=c;assert(!campaign_choice(&c,0)&&!memcmp(&b,&c,sizeof c));
  campaign_step(&c);unchanged(&c,last);campaign_continue(&c);assert(c.mode==CAM_MAP&&c.imported_ship==-1&&c.imported_battle==0);unchanged(&c,last);
  campaign_step(&c);unchanged(&c,last);
 }
}
static void timing_defeat_and_away(void){
 Campaign c;launch(&c,6,4);Supplies s=supplies(&c);
 c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_HULL;c.game.enemy.hull=0;c.game.paused=1;
 for(int i=0;i<3*GAME_HZ;i++)campaign_step(&c);
 assert(c.mode==CAM_COMBAT&&c.result_ticks==0);unchanged(&c,s);c.game.paused=0;
 for(int i=0;i<outcome_duration(&c.game);i++){campaign_step(&c);assert(c.mode==CAM_COMBAT);unchanged(&c,s);}
 campaign_step(&c);assert(c.mode==CAM_EVENT&&c.scrap>s.scrap);
 launch(&c,7,4);s=supplies(&c);c.game.player.hull=0;c.game.enemy.hull=0;
 for(int i=0;i<2*GAME_HZ+3&&c.mode==CAM_COMBAT;i++)campaign_step(&c);
 assert(c.mode==CAM_DEFEAT&&c.game.outcome==GAME_DEFEAT);assert(c.fuel==s.fuel&&c.scrap==s.scrap&&c.game.missiles==s.ammo);
 launch(&c,7,5);c.game.crew[0].ship=1;finish(&c,RESULT_ENEMY_CREW);
 assert(c.game.crew[0].ship==1&&c.game.crew[0].health>0);assert(campaign_choice(&c,0)&&c.event_resolved);
 s=supplies(&c);campaign_continue(&c);assert(c.mode==CAM_EVENT&&game_away_crew(&c.game));unchanged(&c,s);
 c.game.crew[0].ship=0;campaign_continue(&c);assert(c.mode==CAM_MAP);unchanged(&c,s);
}
static void storage_cap(void){
 for(int root=6;root<=7;root++){
  Campaign c;launch(&c,root,8);c.fuel=c.scrap=c.game.missiles=c.game.drone_parts=1000000;
  finish(&c,RESULT_ENEMY_HULL);assert(!c.event_resolved&&campaign_choice_count(&c)==1);
  assert(c.fuel==1000000&&c.scrap==1000000&&c.game.missiles==1000000&&c.game.drone_parts==1000000);
  assert(strstr(campaign_text(&c),"Storage limit reached"));
  assert(campaign_choice_enabled(&c,0)&&campaign_choice(&c,0)&&c.event_resolved);
  assert(strstr(campaign_text(&c),"Storage limit reached"));
  assert(c.fuel==1000000&&c.scrap==1000000&&c.game.missiles==1000000&&c.game.drone_parts==1000000);
  campaign_continue(&c);assert(c.mode==CAM_MAP);
 }
}
static void placement_and_missing_callback(void){
 for(int policy=1;policy<=2;policy++)for(int sector=0;sector<=1;sector++){
  Campaign c;init(&c,31);c.event_policy=policy;c.sector=sector;c.node_kind[2]=NODE_FIGHT;
  assert(campaign_jump(&c,2));
  if(policy==2&&sector==1)assert(c.mode==CAM_EVENT&&c.event_id==EVENT_IMPORTED&&(c.event_seen&192));
  else assert(c.mode==CAM_COMBAT&&!c.event_seen);
 }
 Campaign c;init(&c,31);assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT&&!c.event_seen);
 init(&c,31);c.node_kind[2]=NODE_FIGHT;c.event_seen=192;assert(campaign_jump(&c,2)&&c.mode==CAM_COMBAT&&c.event_seen==192);
 init(&c,31);c.node_kind[2]=NODE_FIGHT;c.sector_jumps=30;assert(campaign_jump(&c,2)&&c.mode==CAM_COMBAT&&c.pursued&&!c.event_seen);
 launch(&c,6,31);Supplies before=supplies(&c);finish(&c,RESULT_ENEMY_CREW); /* Impossible auto-crew fixture exercises absent callback guard. */
 assert(c.event_resolved);unchanged(&c,before);campaign_continue(&c);assert(c.mode==CAM_MAP);unchanged(&c,before);
 launch(&c,7,31);c.fuel=c.scrap=c.game.missiles=999999;finish(&c,RESULT_ENEMY_HULL);
 assert(c.fuel==1000000&&c.scrap==1000000&&c.game.missiles==1000000);
 assert(strstr(campaign_text(&c),"fuel +1, scrap +1, missiles +1"));
 assert(strstr(campaign_text(&c),"Storage limit reached"));
}
int main(void){peaceful_and_policy();pools_and_launch_guard();outcomes();timing_defeat_and_away();storage_cap();placement_and_missing_callback();puts("Original combat events: peaceful/Engage paths, exact two-ship pools, distinct once-only hull/crew rewards, timing/defeat/away-crew guards, policy and capped-reward completion passed (forced terminal fixtures).");return 0;}
