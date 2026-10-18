#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=0x12345678;a.after=b.after=0xabcdef12;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==0x12345678&&a.after==0xabcdef12&&b.before==0x12345678&&b.after==0xabcdef12);}
static void center(const Ship*s,int id,int*x,int*y){int minx=1000,miny=1000,maxx=-1000,maxy=-1000;const GameRoom*r=0;
 for(int i=0;i<s->room_count;i++){const GameRoom*q=&s->rooms[i];if(q->x<minx)minx=q->x;if(q->y<miny)miny=q->y;if(q->x+q->w>maxx)maxx=q->x+q->w;if(q->y+q->h>maxy)maxy=q->y+q->h;if(q->id==id)r=q;}
 int t=24;if((maxx-minx)*t>232)t=232/(maxx-minx);if((maxy-miny)*t>64)t=64/(maxy-miny);if(t<1)t=1;
 assert(r);*x=(256-(maxx-minx)*t)/2-minx*t+r->x*t+r->w*t/2;*y=44+(64-(maxy-miny)*t)/2-miny*t+r->y*t+r->h*t/2;
}
int main(void){Campaign c;Ui u={.focus_room=16,.weapon=1};audio_init();campaign_init_legacy(&c);draw(&c,&u);
 campaign_touch(&c,&u,95,183);assert(u.page==1&&c.mode==CAM_MAP);draw(&c,&u);
 campaign_touch(&c,&u,95,8);assert(u.page==2);draw(&c,&u);
 c.scrap=100;int reactor=c.game.player.reactor;campaign_touch(&c,&u,100,45);assert(c.game.player.reactor==reactor+1&&c.scrap==72);
 campaign_touch(&c,&u,160,8);assert(u.page==3);c.cargo[0]=2;campaign_touch(&c,&u,120,45);campaign_touch(&c,&u,155,156);assert(c.game.weapon_count==3&&c.cargo[0]==-1);draw(&c,&u);
 campaign_touch(&c,&u,225,8);assert(u.page==4);draw(&c,&u);
 campaign_touch(&c,&u,30,183);assert(!u.page);
 c.fuel=0;campaign_touch(&c,&u,185,183);assert(c.fuel==2&&c.mode==CAM_MAP);
 c.mode=CAM_STORE;c.current=5;u.shop_page=0;int fuel=c.fuel;campaign_touch(&c,&u,80,38);assert(c.fuel==fuel+3);draw(&c,&u);
 campaign_touch(&c,&u,190,158);assert(u.shop_page==1);draw(&c,&u);campaign_touch(&c,&u,90,183);assert(c.mode==CAM_MAP);
 c.mode=CAM_EVENT;c.event_id=EVENT_DISTRESS;c.event_resolved=0;c.fuel=0;draw(&c,&u);int money=c.scrap;campaign_touch(&c,&u,80,60);assert(!c.event_resolved&&c.scrap==money);campaign_touch(&c,&u,80,95);assert(c.event_resolved&&c.scrap==money+10);
 campaign_touch(&c,&u,100,145);assert(u.page==1);c.game.outcome=GAME_VICTORY;draw(&c,&u);campaign_touch(&c,&u,30,183);assert(u.page==0);
 for(int kind=0;kind<ENEMY_KIND_COUNT;kind++){game_begin_encounter(&c.game,kind,5,2);c.mode=CAM_COMBAT;u.aiming=1;u.weapon=1;
  for(int i=0;i<c.game.enemy.room_count;i++){int x,y,id=c.game.enemy.rooms[i].id;center(&c.game.enemy,id,&x,&y);assert(y<112);campaign_touch(&c,&u,x,y);assert(c.game.weapons[1].aim_room==id);draw(&c,&u);}
 }
 c.game.crew_count=6;for(int i=3;i<6;i++){c.game.crew[i]=c.game.crew[0];c.game.crew[i].health=100;}u.aiming=0;u.crew_page=0;
 campaign_touch(&c,&u,130,141);assert(u.crew_page==1);campaign_touch(&c,&u,200,121);assert(u.selected_crew==5);draw(&c,&u);
 c.mode=CAM_COMPLETE;u.page=0;draw(&c,&u);campaign_touch(&c,&u,120,180);assert(c.mode==CAM_COMPLETE&&u.menu_requested);
 c.mode=CAM_DEFEAT;draw(&c,&u);u.logs=1;draw(&c,&u);
 puts("UI: management/store/event/crew pages, all enemy room targets, framebuffer bounds and terminal menu request passed");return 0;}

