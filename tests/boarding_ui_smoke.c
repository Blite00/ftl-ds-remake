#include "render.h"
#include "boarding.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=123;a.after=b.after=456;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==123&&b.before==123&&a.after==456&&b.after==456);}
int main(void){
 Campaign c;Ui u={.weapon=1};campaign_init_legacy(&c);audio_init();c.scrap=500;c.mode=CAM_STORE;u.page=2;
 campaign_touch(&c,&u,80,155);assert(c.game.teleporter[0].installed&&c.scrap==380);draw(&c,&u);
 u.page=0;c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,1,0);
 campaign_touch(&c,&u,150,141);assert(u.boarding&&u.board_party==1);draw(&c,&u);
 campaign_touch(&c,&u,100,117);assert(u.board_party==3);
 campaign_touch(&c,&u,190,117);assert(u.board_party==3&&u.feedback_error);
 campaign_touch(&c,&u,130,152);assert(c.game.crew[0].destination==c.game.teleporter[0].room&&c.game.crew[1].destination==c.game.teleporter[0].room);
 /* Place the arrived party; walking is covered by the simulation suite. */
 for(int i=0;i<2;i++){c.game.crew[i].room=c.game.crew[i].destination=c.game.teleporter[0].room;c.game.crew[i].move_ticks=0;}
 if(!c.game.teleporter[0].power){c.game.player.systems[SYS_ENGINES].power=0;campaign_touch(&c,&u,70,152);}
 assert(c.game.teleporter[0].power);
 campaign_touch(&c,&u,95,181);assert(u.board_side==1);u.board_room=c.game.enemy.rooms[0].id;
 campaign_touch(&c,&u,180,152);assert(c.game.crew[0].ship==1&&c.game.crew[1].ship==1);draw(&c,&u);
 campaign_touch(&c,&u,230,152);assert(u.feedback_error); /* cooldown cannot be bypassed */
 c.game.teleporter[0].cooldown_ticks=0;campaign_touch(&c,&u,230,152);assert(c.game.crew[0].ship==0&&c.game.crew[1].ship==0);
 /* Crew-only aftermath permits recall; abandonment requires two distinct taps. */
 c.game.crew[0].ship=1;c.game.crew[0].room=c.game.crew[0].destination=c.game.enemy.rooms[0].id;
 c.mode=CAM_EVENT;c.event_resolved=1;c.game.outcome=GAME_VICTORY;u.board_party=1;
 campaign_touch(&c,&u,220,8);assert(u.abandon_confirm&&c.game.crew[0].health>0);draw(&c,&u);
 campaign_touch(&c,&u,40,126);assert(!u.abandon_confirm&&c.game.crew[0].health>0);
 campaign_touch(&c,&u,220,8);campaign_touch(&c,&u,200,126);assert(!u.abandon_confirm&&c.game.crew[0].health==0);
 campaign_touch(&c,&u,220,180);assert(!u.boarding);
 /* Outcome presentation freezes touch actions, even if a modal was open. */
 campaign_init_legacy(&c);c.mode=CAM_COMBAT;c.game.enemy.hull=0;
 c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_HULL;
 {Campaign before=c;int x,y,age;
  u.boarding=1;u.abandon_confirm=1;
  for(y=0;y<192;y+=8)for(x=0;x<256;x+=8){campaign_touch(&c,&u,x,y);assert(!memcmp(&before,&c,sizeof c));}
  for(age=0;age<=60;age++){
   static uint16_t clean_top[256*192],clean_bottom[256*192];
   c.result_ticks=age;Ui clean=u;clean.boarding=clean.crew_details=clean.renaming=0;clean.feedback_ticks=clean.tap_ticks=0;
   draw(&c,&clean);memcpy(clean_top,a.pixels,sizeof clean_top);memcpy(clean_bottom,b.pixels,sizeof clean_bottom);
   u.boarding=u.crew_details=u.renaming=1;u.feedback_ticks=u.tap_ticks=5;draw(&c,&u);
   assert(!memcmp(clean_top,a.pixels,sizeof clean_top)&&!memcmp(clean_bottom,b.pixels,sizeof clean_bottom));
  }
  c.game.outcome=GAME_DEFEAT;c.game.result_reason=RESULT_PLAYER_HULL;c.game.player.hull=0;
  for(age=0;age<=60;age++){
   static uint16_t clean_top[256*192],clean_bottom[256*192];
   c.result_ticks=age;Ui clean=u;clean.boarding=clean.crew_details=clean.renaming=0;clean.feedback_ticks=clean.tap_ticks=0;
   draw(&c,&clean);memcpy(clean_top,a.pixels,sizeof clean_top);memcpy(clean_bottom,b.pixels,sizeof clean_bottom);
   u.boarding=u.crew_details=u.renaming=1;u.feedback_ticks=u.tap_ticks=5;draw(&c,&u);
   assert(!memcmp(clean_top,a.pixels,sizeof clean_top)&&!memcmp(clean_bottom,b.pixels,sizeof clean_bottom));
  }
 }
 puts("Boarding UI: store install, party bound, gather, send/recall cooldown, two-step abandon and framebuffer bounds passed");
}

