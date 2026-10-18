#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct{unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static uint16_t top[256*192],bottom[256*192];
static void draw(Campaign*c,Ui*u){a.before=b.before=123;a.after=b.after=456;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==123&&b.before==123&&a.after==456&&b.after==456);}
static void snap(Campaign*c,Ui*u){draw(c,u);memcpy(top,a.pixels,sizeof top);memcpy(bottom,b.pixels,sizeof bottom);}
int main(void){Campaign c,d;Ui u={.weapon=2,.aiming=1};audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,4));
 c.game.weapons[0].enabled=c.game.weapons[1].enabled=0;c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;ui_fire(&c.game,&u);assert(!u.feedback_error);game_step(&c.game);int ion=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)if(c.game.effects[i].active&&c.game.effects[i].kind==EFFECT_ION)ion++;assert(ion==1);draw(&c,&u);
 memset(c.game.effects,0,sizeof c.game.effects);u.feedback_ticks=u.tap_ticks=0;
 /* Hidden ion timers must not leak through targeting or boarding. */
 for(int level=1;level<=3;level++){c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=level;d=c;d.game.enemy.systems[SYS_WEAPONS].ion_ticks=150;d.game.enemy.systems[SYS_WEAPONS].ion_bars=1;
  for(int board=0;board<2;board++){u.boarding=board;u.board_side=1;u.board_room=c.game.enemy.systems[SYS_WEAPONS].room;snap(&c,&u);draw(&d,&u);if(level<3){assert(!memcmp(top,a.pixels,sizeof top)&&!memcmp(bottom,b.pixels,sizeof bottom));}else assert(memcmp(top,a.pixels,sizeof top)||memcmp(bottom,b.pixels,sizeof bottom));}
 }
 u.boarding=u.aiming=0;snap(&c,&u);c.game.player.systems[SYS_WEAPONS].ion_ticks=150;c.game.player.systems[SYS_WEAPONS].ion_bars=1;draw(&c,&u);assert(memcmp(bottom,b.pixels,sizeof bottom));
 campaign_init_legacy(&c);c.mode=CAM_STORE;c.current=5;c.scrap=100;u=(Ui){0};campaign_touch(&c,&u,190,157);campaign_touch(&c,&u,190,157);assert(u.shop_page==2);draw(&c,&u);
 Campaign before=c;u.shop_page=(CAM_OFFER_COUNT-1)/4;campaign_touch(&c,&u,90,130);assert(!memcmp(&before,&c,sizeof c));u.shop_page=2;
 campaign_touch(&c,&u,90,38);assert(c.cargo[0]==4&&c.scrap==70);campaign_touch(&c,&u,190,157);assert(u.shop_page==3);campaign_touch(&c,&u,190,157);assert(u.shop_page==4);campaign_touch(&c,&u,190,157);assert(u.shop_page==0);
 puts("Ion UI: actual ion launch, visible countdown, sensor isolation, ninth-offer paging and empty-row safety passed");}
