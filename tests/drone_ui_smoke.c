#include "render.h"
#include "drones.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=17;a.after=b.after=31;render_campaign(c,u,a.p,b.p);assert(a.before==17&&b.before==17&&a.after==31&&b.after==31);}
int main(void){Campaign c;Ui u={0};audio_init();campaign_init_legacy_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.scrap=300;assert(campaign_purchase(&c,11));c.drone_cargo[2]=0;
 ui_drone_open(&u);assert(u.drone_panel);ui_drone_key(&c,&u,4);assert(u.drone_cargo);campaign_touch(&c,&u,50,88);assert(u.drone_selected==2);campaign_touch(&c,&u,50,152);assert(u.drone_sale);draw(&c,&u);ui_drone_key(&c,&u,3);assert(!u.drone_sale&&c.drone_cargo[2]==0);campaign_touch(&c,&u,50,152);int scrap=c.scrap;ui_drone_key(&c,&u,1);ui_drone_key(&c,&u,2);assert(!u.drone_sale&&c.drone_cargo[2]==-1&&c.scrap==scrap+25);
 c.drone_cargo[0]=0;u.drone_selected=0;campaign_touch(&c,&u,50,133);assert(c.drone_cargo[0]==-1&&c.game.drones.slots[1].catalog_id==0);ui_drone_key(&c,&u,4);campaign_touch(&c,&u,180,70);assert(u.drone_selected==1);campaign_touch(&c,&u,180,133);assert(u.drone_selected==0);campaign_touch(&c,&u,50,133);assert(c.drone_cargo[0]==0&&c.game.drones.slots[0].catalog_id==-1);
 c.mode=CAM_COMBAT;u.drone_selected=1;game_adjust_power(&c.game,SYS_ENGINES,-1);game_adjust_power(&c.game,SYS_OXYGEN,-1);ui_drone_key(&c,&u,6);ui_drone_key(&c,&u,6);int parts=c.game.drone_parts;campaign_touch(&c,&u,180,113);assert(c.game.drones.slots[1].enabled&&c.game.drone_parts==parts-1);draw(&c,&u);campaign_touch(&c,&u,180,113);assert(!c.game.drones.slots[1].enabled);campaign_touch(&c,&u,180,113);assert(c.game.drone_parts==parts-1);Game old=c.game;campaign_touch(&c,&u,50,133);assert(!memcmp(&old,&c.game,sizeof old)&&u.feedback_error);
 drones_reset(&c.game);c.game.drone_parts=0;campaign_touch(&c,&u,180,113);assert(u.feedback_error&&strstr(u.feedback,"parts"));draw(&c,&u);ui_drone_key(&c,&u,7);assert(c.game.paused);ui_drone_key(&c,&u,3);assert(!u.drone_panel);campaign_touch(&c,&u,155,180);assert(u.drone_panel);for(int i=0;i<13;i++)ui_drone_key(&c,&u,1);assert(u.drone_focus==0);puts("Drone UI touch/controller, sale, inventory guards and deployment passed.");return 0;}
