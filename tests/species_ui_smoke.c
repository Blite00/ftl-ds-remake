#include "render.h"
#include "species.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=123;a.after=b.after=456;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==123&&b.before==123&&a.after==456&&b.after==456);}
static void room_center(const Ship*s,int id,int*x,int*y){int minx=1000,miny=1000,maxx=0,maxy=0;const GameRoom*r=0;
 for(int i=0;i<s->room_count;i++){const GameRoom*q=&s->rooms[i];if(q->x<minx)minx=q->x;if(q->y<miny)miny=q->y;if(q->x+q->w>maxx)maxx=q->x+q->w;if(q->y+q->h>maxy)maxy=q->y+q->h;if(q->id==id)r=q;}
 int t=14;if((maxx-minx)*t>232)t=232/(maxx-minx);if((maxy-miny)*t>84)t=84/(maxy-miny);assert(r);*x=(256-(maxx-minx)*t)/2+(r->x-minx)*t+r->w*t/2;*y=25+(84-(maxy-miny)*t)/2+(r->y-miny)*t+r->h*t/2;
}
int main(void){Campaign c;Ui u={0};audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;
 for(int i=3;i<8;i++){game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);}c.game.crew_count=8;
 for(int i=0;i<8;i++)assert(game_set_crew_species(&c.game,0,i,i%SPEC_COUNT));
 campaign_touch(&c,&u,100,140);campaign_touch(&c,&u,100,140);assert(u.crew_page==2);draw(&c,&u);campaign_touch(&c,&u,100,122);assert(u.selected_crew==7);campaign_touch(&c,&u,100,122);assert(u.crew_details);draw(&c,&u);campaign_touch(&c,&u,30,183);assert(!u.crew_details);
 u.crew_page=0;u.selected_crew=0;campaign_touch(&c,&u,20,140);assert(u.group_mode&&u.crew_group==1);campaign_touch(&c,&u,100,122);assert(u.crew_group==3&&!u.crew_details);
 int x,y,room=c.game.player.systems[SYS_MEDBAY].room;room_center(&c.game.player,room,&x,&y);campaign_touch(&c,&u,x,y);assert(c.game.crew[0].destination==room&&c.game.crew[1].destination==room);draw(&c,&u);
 campaign_touch(&c,&u,65,140);assert(u.crew_group==255);campaign_touch(&c,&u,20,140);assert(!u.group_mode);
 campaign_touch(&c,&u,150,140);assert(u.boarding);u.board_party=0;campaign_touch(&c,&u,150,10);assert(u.board_page==1);campaign_touch(&c,&u,20,117);campaign_touch(&c,&u,100,117);assert(u.board_party==192);draw(&c,&u);campaign_touch(&c,&u,150,10);campaign_touch(&c,&u,20,117);assert(u.board_party==192&&u.feedback_error);campaign_touch(&c,&u,220,181);
 /* Edge pixels outside cards must not select an invisible next-page actor. */
 u.boarding=1;u.board_page=0;u.board_party=0;int selected=u.selected_crew;campaign_touch(&c,&u,255,117);assert(!u.board_party&&u.selected_crew==selected);u.boarding=0;u.crew_page=0;campaign_touch(&c,&u,255,122);assert(u.selected_crew==selected);
 /* Home room taps cannot route an away actor to a same-numbered enemy room. */
 u.selected_crew=0;u.group_mode=0;c.game.crew[0].ship=1;c.game.crew[0].destination=c.game.crew[0].room;int previous=c.game.crew[0].destination;
 campaign_touch(&c,&u,x,y);assert(u.feedback_error&&c.game.crew[0].destination==previous&&strstr(u.feedback,"BOARD"));c.game.crew[0].ship=0;
 /* Supported species details show bounded health and original sprite shapes. */
 for(int i=0;i<7;i++){u.selected_crew=i;u.crew_details=1;draw(&c,&u);assert(c.game.crew[i].health==species_max_health(i));}
 u.selected_crew=6;campaign_touch(&c,&u,180,183);assert(!u.crew_details&&c.game.crew[6].ability_ticks>0);
 /* Combat cannot initiate dismissal, and management requires confirm. */
 u.crew_details=1;u.selected_crew=0;campaign_touch(&c,&u,190,129);assert(!u.dismiss_confirm&&u.feedback_error);
 c.mode=CAM_MAP;unsigned survivor=c.game.crew[1].id;int count=c.game.crew_count;campaign_touch(&c,&u,190,129);assert(u.dismiss_confirm&&c.game.crew_count==count);draw(&c,&u);campaign_touch(&c,&u,20,125);assert(!u.dismiss_confirm&&c.game.crew_count==count);
 campaign_touch(&c,&u,190,129);campaign_touch(&c,&u,190,125);assert(c.game.crew_count==count-1&&c.game.crew[0].id==survivor&&!u.crew_details&&!u.group_mode&&!u.board_party);
 /* Last living and away crew are rejected before opening dismissal confirmation. */
 campaign_init_legacy(&c);c.mode=CAM_MAP;u=(Ui){.crew_details=1};c.game.crew[1].health=c.game.crew[2].health=0;
 campaign_touch(&c,&u,190,129);assert(!u.dismiss_confirm&&u.feedback_error);
 c.game.crew[1].health=100;c.game.crew[0].ship=1;campaign_touch(&c,&u,190,129);assert(!u.dismiss_confirm&&u.feedback_error);
 puts("Species UI: eightcrew paging, grouporders, boarding bounded across pages, species/lockdown and confirmed dismissal passed");}
