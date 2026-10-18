/* Original REBEL closure fixtures; earned combat is covered separately. */
#include "campaign.h"
#include "save.h"
#include "outcome.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;
static unsigned seeds[4];
static void begin(unsigned seed){campaign_init_seed(&c,seed);c.sector=1;assert(campaign_imported_begin(&c,11));assert(c.imported_ship==4&&c.imported_battle==1);assert(campaign_choice(&c,0));assert(c.mode==CAM_COMBAT&&c.imported_battle==2);}
static void offer(unsigned seed){begin(seed);assert(c.imported_surrender_threshold>=0);c.game.enemy.hull=c.imported_surrender_threshold;campaign_step(&c);assert(campaign_surrender_pending(&c));assert(c.imported_surrender_threshold==-1&&campaign_choice_count(&c)==2);}
int main(void){
 int combinations=0;for(unsigned seed=1;seed<=256;seed++){
  begin(seed);int mask=(c.imported_surrender_threshold>=0?1:0)|(c.game.enemy_escape_total?2:0);combinations|=1<<mask;seeds[mask]=seed;
  if(mask&1)assert(c.imported_surrender_threshold>=2&&c.imported_surrender_threshold<=3);
  if(mask&2)assert(c.game.enemy_escape_total==40*GAME_HZ&&c.game.enemy_escape_threshold>=3&&c.game.enemy_escape_threshold<=4);
 }assert(combinations==15);
 /* Event/queries hold every clock and do not consume RNG. */
 offer(seeds[3]);before=c;char hint[160];for(int i=0;i<20;i++){
  assert(campaign_choice_enabled(&c,0)&&campaign_choice_enabled(&c,1));campaign_choice_hint(&c,0,hint,sizeof hint);
  assert(!campaign_can_service_home(&c)&&!campaign_service_home(&c));assert(!campaign_can_jump(&c,1)&&!campaign_can_retreat(&c,1));campaign_continue(&c);campaign_step(&c);
 }assert(!memcmp(&before,&c,sizeof c));assert(!campaign_save(&c,"tests/v32-pending.sav"));
 int fuel=c.fuel,ammo=c.game.missiles,parts=c.game.drone_parts,scrap=c.scrap,hull=c.game.enemy.hull;
 assert(campaign_choice(&c,0)&&c.game.outcome==GAME_SURRENDER&&c.game.result_reason==RESULT_ENEMY_SURRENDER);
 assert(c.mode==CAM_EVENT&&c.event_resolved&&c.imported_battle==3&&c.game.enemy.hull==hull);
 assert(c.fuel>=fuel+1&&c.fuel<=fuel+4&&c.game.missiles>=ammo+1&&c.game.missiles<=ammo+4&&c.game.drone_parts>=parts+1&&c.game.drone_parts<=parts+3&&c.scrap==scrap);
 assert(outcome_duration(&c.game)==0&&!c.game.enemy_escape_total);before=c;assert(!campaign_choice(&c,0)&&!memcmp(&before,&c,sizeof c));campaign_continue(&c);assert(c.mode==CAM_MAP&&c.imported_surrender_threshold==-1&&!c.imported_escape_warned);
 /* Refusal is the original empty event: no reinitialization or second offer. */
 offer(seeds[3]);Game enemy_before=c.game;fuel=c.fuel;scrap=c.scrap;ammo=c.game.missiles;parts=c.game.drone_parts;
 assert(campaign_choice(&c,1)&&c.mode==CAM_COMBAT&&c.imported_battle==2);assert(!memcmp(&enemy_before,&c.game,sizeof c.game));assert(c.fuel==fuel&&c.scrap==scrap&&c.game.missiles==ammo&&c.game.drone_parts==parts);
 campaign_step(&c);assert(c.imported_battle==4&&c.imported_escape_warned);assert(c.game.enemy_escape_ticks<enemy_before.enemy_escape_ticks);
 before=c;for(int i=0;i<10;i++)campaign_step(&c);assert(!memcmp(&before,&c,sizeof c));assert(campaign_choice(&c,0));for(int i=0;i<10;i++){campaign_step(&c);assert(!campaign_surrender_pending(&c));}
 /* Positive ceasefire loot is visibly bounded, even at exact capacity. */
 offer(seeds[1]);c.fuel=c.game.missiles=c.game.drone_parts=1000000;assert(campaign_choice(&c,0));assert(c.fuel==1000000&&c.game.missiles==1000000&&c.game.drone_parts==1000000&&strstr(c.text,"Storage limit")&&strstr(c.text,"fuel +0"));
 /* Unsafe party exchange disables acceptance; refusal remains valid. */
 offer(seeds[1]);c.game.crew_count=2;c.game.crew[1].ship=1;c.game.crew[1].room=c.game.crew[1].destination=c.game.enemy.rooms[0].id;c.game.player.room_count=1;c.game.player.rooms[0].w=c.game.player.rooms[0].h=1;c.game.crew[0].room=c.game.crew[0].destination=c.game.player.rooms[0].id;
 assert(!campaign_choice_enabled(&c,0)&&campaign_choice_enabled(&c,1));campaign_choice_hint(&c,0,hint,sizeof hint);assert(strstr(hint,"exchange"));before=c;assert(!campaign_choice(&c,0)&&!memcmp(&before,&c,sizeof c));assert(campaign_choice(&c,1));
 /* Pause blocks new triggers; all terminal outcomes precede surrender. */
 begin(seeds[3]);c.game.enemy.hull=2;c.game.paused=1;before=c;campaign_step(&c);assert(!memcmp(&before,&c,sizeof c));
 begin(seeds[3]);c.game.enemy.hull=0;campaign_step(&c);assert(c.game.outcome==GAME_VICTORY&&c.game.result_reason==RESULT_ENEMY_HULL&&!campaign_surrender_pending(&c));
 begin(seeds[3]);c.game.enemy.hull=2;for(int i=0;i<c.game.enemy_crew_count;i++)c.game.enemy_crew[i].health=0;campaign_step(&c);assert(c.game.outcome==GAME_VICTORY&&c.game.result_reason==RESULT_ENEMY_CREW&&!campaign_surrender_pending(&c));
 begin(seeds[3]);c.game.enemy.hull=c.game.player.hull=0;campaign_step(&c);assert(c.game.outcome==GAME_DEFEAT&&!campaign_surrender_pending(&c));
 begin(seeds[3]);c.game.enemy.hull=2;c.game.enemy_escape_active=1;c.game.enemy_escape_ticks=1;assert(game_enemy_escape_ready(&c.game));campaign_step(&c);assert(c.game.outcome==GAME_ESCAPE&&!campaign_surrender_pending(&c));assert(c.mode==CAM_EVENT&&c.event_resolved&&c.scrap==20&&strstr(c.text,"No further reward"));
 /* Old policy4 remains its exact 11-root contract. */
 campaign_init_seed(&c,1);c.event_policy=4;before=c;assert(!campaign_imported_begin(&c,11)&&!memcmp(&before,&c,sizeof c));
 puts("Surrender events: 256 chance fixtures, original accept/refuse, frozen clocks, one-time bounded loot, exchange preflight, warning overlap, terminal priority and old-policy exclusion passed.");return 0;
}
