#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=17;a.after=b.after=31;render_campaign(c,u,a.p,b.p);assert(a.before==17&&b.before==17&&a.after==31&&b.after==31);}
int main(void){Campaign c;Ui u={.page=3};audio_init();campaign_init_legacy(&c);c.mode=CAM_STORE;c.current=5;c.cargo[0]=3;c.cargo[2]=6;
 /* Exact cargo sale with cancellation and confirmation. */
 campaign_touch(&c,&u,60,85);assert(u.cargo_selected==2);int scrap=c.scrap;campaign_touch(&c,&u,70,132);assert(u.sale_confirm&&c.cargo[2]==6);draw(&c,&u);campaign_touch(&c,&u,50,140);assert(!u.sale_confirm&&c.scrap==scrap);campaign_touch(&c,&u,70,132);campaign_touch(&c,&u,180,140);assert(c.cargo[0]==3&&c.cargo[2]==-1&&c.scrap==scrap+game_weapon_price(6)/2);
 /* Installed reordering and removal target exact slots. */
 campaign_touch(&c,&u,190,29);assert(u.equipment_installed);int first=c.game.weapons[0].catalog_id,second=c.game.weapons[1].catalog_id;campaign_touch(&c,&u,60,65);assert(u.equipment_slot==1);campaign_touch(&c,&u,140,156);assert(c.game.weapons[0].catalog_id==second&&c.game.weapons[1].catalog_id==first&&u.equipment_slot==0);draw(&c,&u);campaign_touch(&c,&u,220,156);assert(c.game.weapons[0].catalog_id==first&&u.equipment_slot==1);campaign_touch(&c,&u,50,156);assert(c.game.weapon_count==1&&c.cargo[1]==second);
 campaign_touch(&c,&u,60,44);campaign_touch(&c,&u,70,132);assert(u.sale_confirm);scrap=c.scrap;campaign_touch(&c,&u,180,140);assert(c.game.weapon_count==0&&c.scrap==scrap+game_weapon_price(first)/2);draw(&c,&u);
 /* Zero-equipped combat and every touch region remain bounded and reject fire. */
 c.mode=CAM_COMBAT;u=(Ui){0};draw(&c,&u);ui_fire(&c.game,&u);assert(u.feedback_error);u.aiming=1;draw(&c,&u);for(int y=0;y<192;y+=7)for(int x=0;x<256;x+=11){ui_touch(&c.game,&u,x,y);draw(&c,&u);}assert(c.game.weapon_count==0);ui_select_weapon(&c.game,&u,0);ui_bomb_toggle_side(&c.game,&u);ui_beam_pick_room(&c.game,&u);
 c.mode=CAM_MAP;u=(Ui){.page=3};campaign_touch(&c,&u,60,44);campaign_touch(&c,&u,30,156);assert(c.game.weapon_count==1&&c.game.weapons[0].catalog_id==3);draw(&c,&u);
 /* Full cargo removal rejects without changing campaign state. */
 for(int i=0;i<CAM_CARGO_COUNT;i++){c.cargo[i]=2;}campaign_touch(&c,&u,190,29);Campaign before=c;campaign_touch(&c,&u,50,156);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error);campaign_touch(&c,&u,70,132);assert(!u.sale_confirm&&u.feedback_error);
 /* Store manage offer opens selection and never sells an arbitrary item. */
 c.mode=CAM_STORE;u=(Ui){.shop_page=1};before=c;campaign_touch(&c,&u,90,130);assert(u.page==3&&!memcmp(&before,&c,sizeof c));draw(&c,&u);
 puts("Equipment UI: exact sales/cancel, reorder/remove/install, full-cargo refusal, zero-weapon touch bounds and store navigation passed");return 0;}
