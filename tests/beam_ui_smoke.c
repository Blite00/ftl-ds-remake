#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct{unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static void draw(Campaign*c,Ui*u){a.before=b.before=123;a.after=b.after=456;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==123&&b.before==123&&a.after==456&&b.after==456);}
static void screen(const Ship*s,int bx,int by,int*x,int*y){int mx=1000,my=1000,xx=-1000,yy=-1000;for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];if(r->x<mx)mx=r->x;if(r->y<my)my=r->y;if(r->x+r->w>xx)xx=r->x+r->w;if(r->y+r->h>yy)yy=r->y+r->h;}int t=24;if((xx-mx)*t>232)t=232/(xx-mx);if((yy-my)*t>64)t=64/(yy-my);if(t<1)t=1;*x=(256-(xx-mx)*t)/2-mx*t+bx*t/GAME_BEAM_UNITS;*y=44+(64-(yy-my)*t)/2-my*t+by*t/GAME_BEAM_UNITS;}
int main(void){Campaign c;Ui u={.weapon=2,.aiming=1};audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,5));c.game.weapons[0].enabled=c.game.weapons[1].enabled=0;
 GameRoom*r=&c.game.enemy.rooms[0];int x0,y0,x1,y1,bx=(r->x*2+r->w)*GAME_BEAM_UNITS/2,by=r->y*GAME_BEAM_UNITS+GAME_BEAM_UNITS/2;screen(&c.game.enemy,bx,by,&x0,&y0);screen(&c.game.enemy,bx,by+GAME_BEAM_UNITS,&x1,&y1);
 campaign_touch(&c,&u,x0,y0);assert(u.beam_pending&&!c.game.weapons[2].beam_target_valid);draw(&c,&u);
 c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;ui_fire(&c.game,&u);assert(u.feedback_error&&!c.game.weapons[2].pending_shots);
 campaign_touch(&c,&u,x0,y0);assert(u.beam_pending&&u.feedback_error);campaign_touch(&c,&u,x1,y1);assert(!u.beam_pending&&c.game.weapons[2].beam_target_valid&&!c.game.weapons[2].pending_shots);draw(&c,&u);
 int oldx=c.game.weapons[2].beam_x0,oldy=c.game.weapons[2].beam_y0,oldend=c.game.weapons[2].beam_y1;
 ui_fire(&c.game,&u);assert(!u.feedback_error);game_step(&c.game);int effect=-1;for(int i=0;i<GAME_MAX_EFFECTS;i++)if(c.game.effects[i].active&&c.game.effects[i].kind==EFFECT_BEAM)effect=i;assert(effect>=0);
 /* Editing the next line never rewrites the already launched sweep. */
 campaign_touch(&c,&u,x1,y1);campaign_touch(&c,&u,x0,y0);assert(c.game.effects[effect].beam_x0==oldx&&c.game.effects[effect].beam_y0==oldy&&c.game.effects[effect].beam_y1==oldend);
 /* Physical Down selection uses the same two-point setter; repeated point rejects. */
 u.beam_pending=0;c.game.target_room=c.game.enemy.rooms[0].id;ui_beam_pick_room(&c.game,&u);assert(u.beam_pending);ui_beam_pick_room(&c.game,&u);assert(u.beam_pending&&u.feedback_error);c.game.target_room=c.game.enemy.rooms[1].id;ui_beam_pick_room(&c.game,&u);assert(!u.beam_pending);
 for(int tick=0;tick<40;tick++){game_step(&c.game);draw(&c,&u);}
 campaign_init_legacy(&c);c.mode=CAM_STORE;c.current=5;c.scrap=100;u=(Ui){.shop_page=2};draw(&c,&u);campaign_touch(&c,&u,90,70);assert(c.cargo[0]==5);Campaign before=c;u.shop_page=(CAM_OFFER_COUNT-1)/4;campaign_touch(&c,&u,90,130);assert(!memcmp(&before,&c,sizeof c));
 puts("Beam UI: two points/controller picking, zero-line and prematurefire rejection, independent sweep snapshot, store offer and bounds passed");}
