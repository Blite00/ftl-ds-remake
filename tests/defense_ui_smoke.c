#include "render.h"
#include "drones.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} top,bottom;
static uint16_t saved[256*192];
static void draw(Campaign*c,Ui*u){top.before=bottom.before=17;top.after=bottom.after=31;render_campaign(c,u,top.p,bottom.p);assert(top.before==17&&bottom.before==17&&top.after==31&&bottom.after==31);}
int main(void){Campaign c;Ui u={.shop_page=3,.shop_selected=2};audio_init();campaign_init_legacy_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.scrap=300;
 assert(campaign_offer_count(&c)==17&&campaign_offer_price(&c,14)==50);assert(strstr(campaign_offer_name(&c,14),"Defense"));draw(&c,&u);
  Campaign original=c;u.shop_page=4;campaign_touch(&c,&u,100,130);assert(!memcmp(&c,&original,sizeof c));u.shop_page=3; /* V55: the last page has an empty fourth row */
 campaign_touch(&c,&u,100,97);assert(c.scrap==250&&c.drone_cargo[0]==1&&!u.feedback_error);
 assert(campaign_purchase(&c,11));assert(campaign_drone_swap(&c,0,0));ui_drone_open(&u);u.feedback_ticks=0;draw(&c,&u);memcpy(saved,bottom.p,sizeof saved);
 c.game.drones.slots[0].catalog_id=0;draw(&c,&u);assert(memcmp(saved+62*256,bottom.p+62*256,18*256*sizeof(uint16_t)));c.game.drones.slots[0].catalog_id=1;
 c.mode=CAM_COMBAT;game_adjust_power(&c.game,SYS_ENGINES,-1);game_adjust_power(&c.game,SYS_OXYGEN,-1);ui_drone_key(&c,&u,6);ui_drone_key(&c,&u,6);campaign_touch(&c,&u,180,113);assert(c.game.drones.slots[0].enabled);u.feedback_ticks=0;
 c.game.drones.slots[0].charge_ticks=drones_cooldown(1);draw(&c,&u);memcpy(saved,top.p,sizeof saved);c.game.drones.slots[0].charge_ticks=0;draw(&c,&u);assert(memcmp(saved+143*256,top.p+143*256,8*256*sizeof(uint16_t)));assert(memcmp(saved+160*256,top.p+160*256,5*256*sizeof(uint16_t)));
 c.game.effects[0]=(GameEffect){.active=1,.side=1,.kind=EFFECT_MISSILE,.result=EFFECT_INTERCEPT,.duration=18,.age=18,.room=0};draw(&c,&u);memcpy(saved,top.p,sizeof saved);c.game.effects[0].result=EFFECT_HIT;draw(&c,&u);assert(memcmp(saved+50*256,top.p+50*256,48*256*sizeof(uint16_t)));
 /* Interceptions do not trigger the lower-screen hull impact overlay. */
 c.game.effects[0].result=EFFECT_INTERCEPT;u.drone_panel=0;draw(&c,&u);memcpy(saved,bottom.p,sizeof saved);c.game.effects[0].active=0;draw(&c,&u);assert(!memcmp(saved,bottom.p,sizeof saved));
 campaign_init_legacy_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.scrap=300;for(int i=0;i<4;i++)c.drone_cargo[i]=0;u=(Ui){.shop_page=3};original=c;campaign_touch(&c,&u,100,97);assert(!memcmp(&original,&c,sizeof c));assert(u.feedback_error&&strstr(u.feedback,"Drone cargo full"));draw(&c,&u);
 puts("Defense UI offer, names, reload status, interception, cargo rejection and framebuffer bounds passed.");return 0;}
