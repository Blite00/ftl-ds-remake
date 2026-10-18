#include "render.h"
#include "audio.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static struct {unsigned head;uint16_t pixels[256*192];unsigned tail;} top,bottom;
int main(void){
 Campaign c;Ui u={.weapon=1};campaign_init_legacy(&c);audio_init();
 c.mode=CAM_COMBAT;c.game.paused=1;
 ui_fire(&c.game,&u);assert(u.feedback_error&&strstr(u.feedback,"charging"));
 int ticks=c.game.ticks;for(int i=0;i<75;i++)ui_tick(&u);
 assert(!u.feedback_ticks&&c.game.paused&&c.game.ticks==ticks);
 c.game.weapons[1].charge_ticks=c.game.weapons[1].cooldown_ticks;
 ui_fire(&c.game,&u);assert(!u.feedback_error&&strstr(u.feedback,"queued"));
 campaign_touch(&c,&u,20,180);assert(!c.game.paused&&!u.feedback_error&&u.tap_ticks);
 c.mode=CAM_MAP;u.page=2;c.scrap=0;
 campaign_touch(&c,&u,60,45);assert(u.feedback_error&&strstr(u.feedback,"scrap"));
 u.page=3;campaign_touch(&c,&u,20,155);assert(u.feedback_error&&strstr(u.feedback,"cargo"));
 u.page=0;c.mode=CAM_STORE;c.current=5;c.stock[5][0]=0;
 campaign_touch(&c,&u,70,38);assert(u.feedback_error&&strstr(u.feedback,"sold out"));
 /* Repeat rejected input must restart the feedback timer. */
 ui_tick(&u);campaign_touch(&c,&u,70,38);assert(u.feedback_ticks==75);
 top.head=bottom.head=0x1234;top.tail=bottom.tail=0x5678;
 for(int x=0;x<=255;x+=255)for(int y=0;y<=191;y+=191){u.tap_x=x;u.tap_y=y;u.tap_ticks=8;render_campaign(&c,&u,top.pixels,bottom.pixels);}
 assert(top.head==0x1234&&bottom.head==0x1234&&top.tail==0x5678&&bottom.tail==0x5678);
 puts("Feedback: paused timer, fire rejection/queue, repeated purchase rejection, upgrade/cargo reasons and edge tap rendering passed");
}
