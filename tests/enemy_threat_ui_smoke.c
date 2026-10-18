/* Isolated renderer fixtures, not campaign or hardware acceptance. */
#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,previous;static Ui u;
static struct {unsigned pre;uint16_t pixels[256*192];unsigned post;} top,bottom;
static uint16_t saved[256*192];
static void draw(void){previous=g;top.pre=bottom.pre=17;top.post=bottom.post=31;render_game(&g,&u,top.pixels,bottom.pixels);assert(!memcmp(&previous,&g,sizeof g));assert(top.pre==17&&bottom.pre==17&&top.post==31&&bottom.post==31);}
static int panel_diff(void){for(int y=41;y<48;y++)if(memcmp(saved+y*256,top.pixels+y*256,256*sizeof(uint16_t)))return 1;return 0;}
static void snapshot(void){draw();memcpy(saved,top.pixels,sizeof saved);}
int main(void){
 audio_init();game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy.systems[SYS_WEAPONS].capacity=g.enemy.systems[SYS_WEAPONS].health=g.enemy.systems[SYS_WEAPONS].power=3;
 int ids[]={2,GAME_ENEMY_ARTEMIS};assert(game_enemy_set_loadout(&g,ids,2,10));snapshot();
 g.enemy_weapon.charge_ticks=60;g.enemy_weapon_secondary.charge_ticks=210;g.enemy_missiles=0;g.enemy.systems[SYS_WEAPONS].power=1;draw();assert(!panel_diff());
 g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;snapshot();g.enemy_missiles=10;g.enemy.systems[SYS_WEAPONS].power=3;draw();assert(panel_diff());
 snapshot();g.enemy_weapon_secondary.charge_ticks=30;draw();assert(panel_diff());
 snapshot();g.enemy_weapon_secondary.pending_shots=1;draw();assert(panel_diff());
 g.enemy_weapon_secondary.pending_shots=0;u.aiming=1;draw();u.aiming=0;
 for(int a=1;a<=4;a++)for(int b=1;b<=4;b++){assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy.systems[SYS_WEAPONS].capacity=g.enemy.systems[SYS_WEAPONS].health=g.enemy.systems[SYS_WEAPONS].power=4;int pair[]={a,b};assert(game_enemy_set_loadout(&g,pair,2,0));draw();g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;draw();}
 /* Hull results must remain visible even if the killing shot left a modal open. */
 static Campaign campaign,original;campaign_init_seed(&campaign,42);campaign.mode=CAM_COMBAT;
 for(int side=0;side<2;side++){
  campaign.game.outcome=side?GAME_VICTORY:GAME_DEFEAT;
  campaign.game.result_reason=side?RESULT_ENEMY_HULL:RESULT_PLAYER_HULL;
  campaign.game.player.hull=side?30:0;campaign.game.enemy.hull=side?0:10;
  for(int age=0;age<2*GAME_HZ;age++){
   campaign.result_ticks=age;u=(Ui){0};original=campaign;
   render_campaign(&campaign,&u,top.pixels,bottom.pixels);memcpy(saved,top.pixels,sizeof saved);
   u=(Ui){.aiming=1,.drone_panel=1,.actions=1,.retreat=1,.boarding=1,.crew_details=1,.feedback_ticks=20,.tap_ticks=8};
   render_campaign(&campaign,&u,top.pixels,bottom.pixels);
   assert(!memcmp(saved,top.pixels,sizeof saved));assert(!memcmp(&campaign,&original,sizeof campaign));
   assert(top.pre==17&&bottom.pre==17&&top.post==31&&bottom.post==31);
  }
 }
 puts("Enemy threat UI: public weapon families, hidden charge/power/ammo, sensor-gated independent charge and empty/disabled/firing state, pure rendering and framebuffer bounds passed.");return 0;
}
