#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} top,bottom;
static uint16_t saved[256*192];
static void draw(Campaign*c,Ui*u){top.before=bottom.before=17;top.after=bottom.after=31;render_campaign(c,u,top.p,bottom.p);assert(top.before==17&&bottom.before==17&&top.after==31&&bottom.after==31);}
static void exit_event(Campaign*c){c->mode=CAM_EVENT;c->event_id=EVENT_EXIT;c->event_resolved=0;c->current=c->node_count-1;}
int main(void){Campaign c;Ui u={0};audio_init();campaign_init_seed(&c,42);exit_event(&c);assert(campaign_route_count(&c)==2);draw(&c,&u);memcpy(saved,bottom.p,sizeof saved);
 ui_event_key(&c,&u,1);assert(u.event_selected==1);draw(&c,&u);assert(memcmp(saved+42*256,bottom.p+42*256,113*256*sizeof(uint16_t)));
 int chosen=c.next_sector[1],scrap=c.scrap,fuel=c.fuel;ui_event_key(&c,&u,2);assert(c.sector==1&&c.sector_types[1]==chosen&&c.mode==CAM_MAP);assert(c.scrap==scrap+12&&c.fuel==fuel+3);Campaign before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));campaign_touch(&c,&u,250,112);assert(c.sector==1&&c.scrap==before.scrap&&c.fuel==before.fuel);
 campaign_init_seed(&c,42);u=(Ui){0};exit_event(&c);chosen=c.next_sector[1];campaign_touch(&c,&u,80,112);assert(c.sector==1&&c.sector_types[1]==chosen);draw(&c,&u);
 campaign_init_seed(&c,42);u=(Ui){0};exit_event(&c);before=c;campaign_touch(&c,&u,80,98);assert(!memcmp(&before,&c,sizeof c));
 c.game.crew[0].ship=1;before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error&&strstr(u.feedback,"away crew"));draw(&c,&u);campaign_touch(&c,&u,80,112);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error);
 campaign_init_seed(&c,42);u=(Ui){0};c.mode=CAM_EVENT;c.event_id=EVENT_DISTRESS;c.event_resolved=0;c.fuel=0;draw(&c,&u);before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error);ui_event_key(&c,&u,1);assert(u.event_selected==1);ui_event_key(&c,&u,2);assert(c.event_resolved);before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));
 campaign_init_seed(&c,42);u=(Ui){0};exit_event(&c);u.page=1;before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));
 campaign_init_seed(&c,42);u=(Ui){0};c.sector=6;c.sector_types[6]=SECTOR_REBEL;c.next_sector[0]=SECTOR_FINAL;c.next_sector[1]=SECTOR_UNCHARTED;exit_event(&c);assert(campaign_route_count(&c)==1);ui_event_key(&c,&u,1);assert(u.event_selected==0);draw(&c,&u);ui_event_key(&c,&u,2);assert(c.sector==7&&c.sector_types[7]==SECTOR_FINAL);exit_event(&c);assert(campaign_route_count(&c)==0&&campaign_choice_count(&c)==1);ui_event_key(&c,&u,2);assert(c.mode==CAM_COMBAT&&c.boss_phase==0);before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));
 campaign_init_seed(&c,42);u=(Ui){0};exit_event(&c);c.next_sector[0]=SECTOR_UNCHARTED;before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error&&strstr(u.feedback,"route is unavailable"));
 campaign_init_legacy_seed(&c,42);u=(Ui){0};exit_event(&c);assert(campaign_choice_count(&c)==1&&campaign_route_count(&c)==0);draw(&c,&u);ui_event_key(&c,&u,2);assert(c.sector==1&&c.node_count==12);
 puts("Sector route UI: both inputs, selected frame, disabled choices, stale actions, modal guard and legacy decisions passed.");return 0;}
