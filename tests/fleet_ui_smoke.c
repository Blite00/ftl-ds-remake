#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} top,bottom;
static uint16_t saved[256*192];
static void draw(Campaign*c,Ui*u){top.before=bottom.before=17;top.after=bottom.after=31;render_campaign(c,u,top.p,bottom.p);assert(top.before==17&&bottom.before==17&&top.after==31&&bottom.after==31);}
int main(void){Campaign c;Ui u={0};audio_init();campaign_init_seed(&c,42);assert(c.node_count==23&&c.sector_count==8);draw(&c,&u);
 /* A fleet still off-map must not paint red capture hatching at x=0. */
 assert(campaign_fleet_x(&c,0)==0);for(int y=40;y<141;y++)assert(bottom.p[y*256]!=(uint16_t)(0x8000|(245>>3)|((108>>3)<<5)|((97>>3)<<10)));
 u.map_overview=1;for(int i=0;i<c.node_count;i++){int px,py;assert(ui_map_node_position(&c,&u,i,&px,&py));campaign_touch(&c,&u,px,py);assert(c.selected==i);draw(&c,&u);}
 u=(Ui){0};c.selected=0;for(int i=0;i<c.node_count;i++){ui_map_key(&c,&u,1);assert(c.selected==(i+1)%c.node_count);}ui_map_key(&c,&u,0);assert(c.selected==22);
 int selected=c.selected;campaign_touch(&c,&u,254,142);assert(c.selected==selected);
 u=(Ui){0};c.selected=6;c.sector_jumps=0;draw(&c,&u);memcpy(saved,bottom.p,sizeof saved);c.sector_jumps=14;draw(&c,&u);assert(memcmp(saved+40*256,bottom.p+40*256,101*256*sizeof(uint16_t)));
 campaign_init_seed(&c,42);u=(Ui){0};int fuel=c.fuel;ui_map_key(&c,&u,2);assert(c.sector_jumps==1&&c.fuel==fuel);assert(strstr(u.feedback,"fleet advanced"));campaign_touch(&c,&u,220,10);assert(c.sector_jumps==2&&c.fuel==fuel);
 c.mode=CAM_STORE;Campaign before=c;ui_map_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));c.mode=CAM_MAP;u.page=1;before=c;ui_map_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));
 campaign_init_seed(&c,42);c.node_count=12;c.sector_count=6;c.current=0;u=(Ui){0};for(int i=0;i<12;i++)c.node_y[i]=(i&1)?67:123;c.node_y[0]=c.node_y[11]=94;
 u.map_overview=1;for(int i=0;i<12;i++){int px,py;assert(ui_map_node_position(&c,&u,i,&px,&py));campaign_touch(&c,&u,px,py);assert(c.selected==i);draw(&c,&u);}ui_map_key(&c,&u,1);assert(c.selected==0);ui_map_key(&c,&u,0);assert(c.selected==11);
 puts("Fleet UI: all 23 and legacy 12 beacons reachable by touch/controller, fleet preview, wait and framebuffer bounds passed.");return 0;}
