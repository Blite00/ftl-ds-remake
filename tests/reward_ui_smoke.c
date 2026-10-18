/* Focused reward UI fixtures; host framebuffers are not emulator evidence. */
#include "render.h"
#include "audio.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,b;static Ui u;
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} top,bottom;
static void draw(void){top.before=bottom.before=0x12345678;top.after=bottom.after=0x87654321;b=c;render_campaign(&c,&u,top.pixels,bottom.pixels);assert(!memcmp(&c,&b,sizeof c));assert(top.before==0x12345678&&bottom.before==0x12345678&&top.after==0x87654321&&bottom.after==0x87654321);}
static void setup(int crew,int full){campaign_init_seed(&c,42);u=(Ui){0};c.mode=CAM_EVENT;c.event_resolved=1;if(crew){if(full){for(int i=c.game.crew_count;i<GAME_MAX_CREW;i++)game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);c.game.crew_count=GAME_MAX_CREW;}assert(rewards_offer_crew(&c.reward,&c.game,SPEC_ZOLTAN));}else{if(full)for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=i;assert(rewards_offer_weapon(&c.reward,5));}draw();}
static void free_accept(void){for(int crew=0;crew<2;crew++)for(int touch=0;touch<2;touch++){setup(crew,0);unsigned id=c.reward.recruit.id;if(touch){campaign_touch(&c,&u,90,34);campaign_touch(&c,&u,190,181);}else ui_reward_key(&c,&u,2);assert(!c.reward.kind&&!u.reward_active&&!u.reward_confirm&&u.feedback_ticks&&!u.feedback_error);if(crew)assert(c.game.crew_count==4&&c.game.crew[3].id==id);else assert(c.cargo[0]==5);draw();}}
static void replacement(void){
 for(int crew=0;crew<2;crew++)for(int touch=0;touch<2;touch++){
  setup(crew,1);b=c;ui_reward_key(&c,&u,2);assert(u.feedback_error&&c.reward.kind&&!memcmp(&b,&c,sizeof c));
  int slot=crew?7:2;unsigned id=c.reward.recruit.id;
  if(touch)campaign_touch(&c,&u,90,48+slot*(crew?14:24)+3);
  else for(int i=0;i<=slot;i++)ui_reward_key(&c,&u,1);
  assert(u.reward_selected==slot);b=c;
  if(touch)campaign_touch(&c,&u,190,182);else ui_reward_key(&c,&u,2);
  assert(u.reward_confirm==1&&!memcmp(&b,&c,sizeof c));draw();
  ui_reward_key(&c,&u,0);assert(u.reward_selected==slot&&!memcmp(&b,&c,sizeof c));
  if(touch)campaign_touch(&c,&u,50,182);else ui_reward_key(&c,&u,3);
  assert(!u.reward_confirm&&c.reward.kind&&!memcmp(&b,&c,sizeof c));
  ui_reward_key(&c,&u,2);assert(u.reward_confirm==1);draw();
  if(touch)campaign_touch(&c,&u,190,182);else ui_reward_key(&c,&u,2);
  assert(!c.reward.kind&&!u.reward_confirm&&!u.reward_active&&!u.feedback_error);
  if(crew){assert(c.game.crew[slot].id==id);for(int i=0;i<GAME_MAX_CREW;i++)if(i!=slot)assert(!memcmp(&c.game.crew[i],&b.game.crew[i],sizeof(Crew)));}
  else for(int i=0;i<CAM_CARGO_COUNT;i++)assert(c.cargo[i]==(i==slot?5:i));
  draw();
 }
}
static void decline(void){for(int crew=0;crew<2;crew++)for(int touch=0;touch<2;touch++){setup(crew,1);b=c;if(touch)campaign_touch(&c,&u,50,182);else ui_reward_key(&c,&u,3);assert(u.reward_confirm==2&&!memcmp(&c,&b,sizeof c));draw();if(touch)campaign_touch(&c,&u,50,182);else ui_reward_key(&c,&u,3);assert(!u.reward_confirm&&c.reward.kind&&!memcmp(&c,&b,sizeof c));ui_reward_key(&c,&u,3);if(touch)campaign_touch(&c,&u,190,182);else ui_reward_key(&c,&u,2);assert(!c.reward.kind&&!u.reward_confirm&&!memcmp(c.game.crew,b.game.crew,sizeof c.game.crew)&&!memcmp(c.cargo,b.cargo,sizeof c.cargo));draw();}}
static void bounds(void){for(int crew=0;crew<2;crew++){setup(crew,1);for(int selected=-1;selected<(crew?8:4);selected++)for(int confirm=0;confirm<=2;confirm++){u.reward_active=1;u.reward_selected=selected;u.reward_confirm=confirm;draw();}}for(int species=0;species<SPEC_COUNT;species++){setup(1,1);assert(rewards_decline(&c.reward));assert(rewards_offer_crew(&c.reward,&c.game,species));draw();}}
int main(void){audio_init();free_accept();replacement();decline();bounds();puts("Reward UI: touch/buttons free acceptance, exact full cargo/crew replacement, confirmation cancel/decline, stable queries and dual framebuffer canaries passed.");return 0;}
