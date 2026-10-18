/* Explicit low-hull fixtures exercise UI; earned combat is tested separately. */
#include "render.h"
#include "audio.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,b,accepted;
static Ui u;
static struct {unsigned first;uint16_t pixels[256*192];unsigned last;} top,bottom;
static void offer(void){
 for(unsigned seed=1;seed<1024;seed++){
  campaign_init_seed(&c,seed);assert(campaign_imported_begin(&c,11));assert(campaign_choice(&c,0));
  if(c.imported_surrender_threshold>=0){c.game.enemy.hull=c.imported_surrender_threshold;campaign_step(&c);if(campaign_surrender_pending(&c))return;}
 }
 assert(!"No seeded surrender offer");
}
static void draw(void){b=c;top.first=bottom.first=17;top.last=bottom.last=31;render_campaign(&c,&u,top.pixels,bottom.pixels);assert(!memcmp(&b,&c,sizeof c));assert(top.first==17&&bottom.first==17&&top.last==31&&bottom.last==31);}
static void modals(void){u=(Ui){.page=1,.actions=1,.retreat=1,.home_defense=1,.boarding=1,.crew_details=1,.renaming=1,.drone_panel=1,.sale_confirm=1,.aiming=1};}
int main(void){
 audio_init();offer();assert(campaign_choice_count(&c)==2&&!campaign_can_service_home(&c));
 modals();ui_event_text_key(&c,&u,1);assert(!u.page&&!u.boarding&&!u.drone_panel&&!u.crew_details);modals();draw();b=c;for(int i=0;i<120;i++)ui_campaign_step(&c,&u);assert(!memcmp(&b,&c,sizeof c));
 ui_event_key(&c,&u,1);assert(u.event_selected==1&&!u.page&&!u.actions&&!u.retreat&&!u.home_defense&&!u.boarding&&!u.crew_details&&!u.drone_panel);
 b=c;ui_event_key(&c,&u,2);assert(c.mode==CAM_COMBAT&&!campaign_surrender_pending(&c)&&c.game.outcome==GAME_PLAYING);assert(c.game.enemy.hull==b.game.enemy.hull&&c.scrap==b.scrap&&c.fuel==b.fuel&&c.game.missiles==b.game.missiles);assert(c.game.enemy_weapon.charge_ticks==b.game.enemy_weapon.charge_ticks);
 offer();modals();b=c;ui_event_key(&c,&u,2);assert(c.game.outcome==GAME_SURRENDER&&!campaign_surrender_pending(&c)&&c.mode==CAM_EVENT);accepted=c;ui_event_key(&c,&u,2);assert(!memcmp(&accepted,&c,sizeof c));
 c=b;modals();campaign_touch(&c,&u,20,55);assert(!memcmp(&accepted,&c,sizeof c));campaign_touch(&c,&u,20,55);assert(!memcmp(&accepted,&c,sizeof c));draw();
 puts("Surrender UI: frozen negotiation, explicit accept/refuse, modal precedence, touch/button equivalence, once-only rewards and framebuffer purity/bounds passed (low-hull fixtures).");return 0;
}

