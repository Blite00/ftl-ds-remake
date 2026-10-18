/* Source event integration. Timed escapes use normal combat ticks and starting
 * resources; explicitly marked terminal/threshold fixtures cover rare branches. */
#include "campaign.h"
#include "outcome.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
static void stage(unsigned seed){campaign_init_seed(&c,seed);assert(c.event_policy==6);c.event_policy=5;assert(campaign_imported_begin(&c,8));assert(c.mode==CAM_EVENT&&c.imported_battle==1&&!c.event_resolved);assert(campaign_choice_count(&c)==1&&!strcmp(campaign_choice_name(&c,0),"Engage"));}
static void launch(unsigned seed){stage(seed);assert(campaign_choice(&c,0));assert(c.mode==CAM_EVENT&&c.imported_battle==4);assert(strstr(campaign_text(&c),"If it gets away"));assert(c.game.enemy_escape_total==40*GAME_HZ&&c.game.enemy_escape_ticks==40*GAME_HZ);assert(campaign_choice_count(&c)==1&&!strcmp(campaign_choice_name(&c,0),"Continue"));}
static void resume(void){assert(campaign_choice(&c,0));assert(c.mode==CAM_COMBAT&&c.imported_battle==2);}
static void policies_and_texts(void){
 assert(!strcmp(campaign_imported_id(8),"AUTO_WARNING"));
 for(int policy=0;policy<3;policy++){campaign_init_seed(&c,1);c.event_policy=policy;Campaign before=c;assert(!campaign_imported_begin(&c,8)&&!memcmp(&before,&c,sizeof c));}
 char texts[9][2048];int count=0;
 for(unsigned seed=1;seed<=128;seed++){stage(seed);assert(c.event_seen==256&&strlen(campaign_text(&c))>60);int found=0;for(int i=0;i<count;i++)found|=!strcmp(texts[i],campaign_text(&c));if(!found){assert(count<9);snprintf(texts[count++],sizeof texts[0],"%s",campaign_text(&c));}}
 assert(count==9);
 launch(2);Game before=c.game;for(int i=0;i<100;i++)campaign_step(&c);assert(!memcmp(&before,&c.game,sizeof before));campaign_continue(&c);assert(c.mode==CAM_EVENT&&c.imported_battle==4);assert(!campaign_choice(&c,1));resume();
 c.game.paused=1;before=c.game;for(int i=0;i<10;i++)campaign_step(&c);assert(!memcmp(&before,&c.game,sizeof before));c.game.paused=0;
 c.game.enemy.systems[SYS_ENGINES].power=0;int remaining=c.game.enemy_escape_ticks;for(int i=0;i<10;i++)campaign_step(&c);assert(c.game.enemy_escape_ticks==remaining&&!c.game.enemy_departed);
}
static void timed_escapes(void){unsigned kinds=0;
 for(unsigned seed=1;seed<=8;seed++){
  launch(seed);kinds|=1u<<(c.game.enemy_kind==ENEMY_ASSAULT);resume();int fuel=c.fuel,scrap=c.scrap,ammo=c.game.missiles,parts=c.game.drone_parts;
  c.sector_jumps=8;assert(!campaign_node_captured(&c,0,0));int frame=0;
  for(;frame<40*GAME_HZ&&c.mode==CAM_COMBAT;frame++){campaign_step(&c);if(frame<40*GAME_HZ-1)assert(!c.game.enemy_departed&&c.sector_jumps==8);}
  assert(frame==40*GAME_HZ&&c.mode==CAM_EVENT&&c.event_resolved&&c.imported_battle==3);
  assert(c.game.outcome==GAME_ESCAPE&&c.game.result_reason==RESULT_ENEMY_ESCAPE&&outcome_ship(&c.game)==-1&&outcome_duration(&c.game)==0);
  assert(c.game.player.hull>0&&game_alive_crew(&c.game)>0);assert(strstr(campaign_text(&c),"The scout jumps away"));
  assert(c.sector_jumps==9&&campaign_node_captured(&c,0,0));assert(c.fuel==fuel&&c.scrap==scrap&&c.game.missiles==ammo&&c.game.drone_parts==parts);
  campaign_step(&c);assert(c.sector_jumps==9);assert(!campaign_choice(&c,0));campaign_continue(&c);assert(c.mode==CAM_MAP&&c.imported_ship==-1&&c.imported_battle==0);assert(c.sector_jumps==9);Campaign before=c;assert(!campaign_imported_begin(&c,8)&&!memcmp(&before,&c,sizeof c));
 }
 assert(kinds==3);
}
static void fixtures(void){
 /* Forced hull destruction proves callback separation and death priority. */
 launch(3);resume();int fuel=c.fuel,scrap=c.scrap,ammo=c.game.missiles,jumps=c.sector_jumps;c.game.enemy.hull=0;
 for(int i=0;i<2*GAME_HZ+3&&c.mode==CAM_COMBAT;i++)campaign_step(&c);
 assert(c.mode==CAM_EVENT&&c.event_resolved&&strstr(campaign_text(&c),"feel relief"));assert(c.sector_jumps==jumps);assert(c.fuel>=fuel+1&&c.fuel<=fuel+3&&c.scrap>=scrap+10&&c.scrap<=scrap+20&&c.game.missiles>=ammo+1&&c.game.missiles<=ammo+3);
 launch(3);resume();fuel=c.fuel;scrap=c.scrap;c.game.enemy_escape_ticks=1;c.game.player.hull=0;c.game.enemy.hull=0;
 for(int i=0;i<2*GAME_HZ+3&&c.mode==CAM_COMBAT;i++)campaign_step(&c);
 assert(c.mode==CAM_DEFEAT&&c.sector_jumps==0&&c.fuel==fuel&&c.scrap==scrap);
 /* Force an initially dormant threshold to exercise a later warning exactly once. */
 launch(3);resume();c.imported_escape_warned=0;/* Reset the fixture to a never-warned dormant escape. */c.game.enemy_escape_active=0;c.game.enemy_escape_threshold=c.game.enemy.hull-1;campaign_step(&c);assert(c.mode==CAM_COMBAT);c.game.enemy.hull--;campaign_step(&c);assert(c.mode==CAM_EVENT&&c.imported_battle==4);int remaining=c.game.enemy_escape_ticks;resume();campaign_step(&c);assert(c.mode==CAM_COMBAT&&c.game.enemy_escape_ticks==remaining-1);
 /* Away crew are lost on departure, surviving home crew can finish the event. */
 launch(3);resume();unsigned id=c.game.crew[0].id;c.game.crew[0].ship=1;c.game.crew[0].room=c.game.crew[0].destination=c.game.enemy.rooms[0].id;c.game.enemy_escape_ticks=1;campaign_step(&c);assert(c.mode==CAM_EVENT&&c.sector_jumps==1&&!c.game.crew[0].health&&c.game.crew[0].id==id);campaign_continue(&c);assert(c.mode==CAM_MAP&&!game_away_crew(&c.game));
 /* Defensive fixture: stranded hostile boarders postpone gotaway consequences. */
 launch(3);resume();c.game.enemy_crew_count=1;game_init_crew(&c.game,1,0,c.game.player.rooms[0].id);c.game.enemy_crew[0].ship=0;c.game.enemy_escape_ticks=1;campaign_step(&c);
 assert(c.mode==CAM_COMBAT&&c.game.enemy_departed&&c.game.outcome==GAME_PLAYING&&c.sector_jumps==0);
 c.game.enemy_crew[0].health=1;c.game.crew[1].room=c.game.crew[1].destination=c.game.enemy_crew[0].room;c.game.crew[1].combat_ticks=GAME_HZ-1;campaign_step(&c);assert(c.mode==CAM_EVENT&&c.game.outcome==GAME_ESCAPE&&c.sector_jumps==1);
 /* Loss of the entire away party defeats the campaign before gotaway effects. */
 launch(3);resume();for(int i=0;i<c.game.crew_count;i++){c.game.crew[i].ship=1;c.game.crew[i].room=c.game.crew[i].destination=c.game.enemy.rooms[0].id;}c.game.enemy_escape_ticks=1;campaign_step(&c);assert(c.mode==CAM_DEFEAT&&c.sector_jumps==0);
}
int main(void){policies_and_texts();timed_escapes();fixtures();puts("AUTO_WARNING: nine source introductions, explicit frozen warning, 8 full 40-second combat escapes, exact once-only pursuit/no loot, hull reward, policy/unique and forced threshold/death/away-crew cases passed.");}
