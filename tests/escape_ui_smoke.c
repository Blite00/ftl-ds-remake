#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned head;uint16_t pixels[256*192];unsigned tail;} top,bottom;
static uint16_t prior[256*192];
static void draw(Campaign*c,Ui*u){top.head=bottom.head=17;top.tail=bottom.tail=31;Campaign before=*c;render_campaign(c,u,top.pixels,bottom.pixels);assert(!memcmp(c,&before,sizeof *c));assert(top.head==17&&bottom.head==17&&top.tail==31&&bottom.tail==31);}
int main(void){Campaign c,expected;Ui u={0};audio_init();campaign_init_seed(&c,42);assert(campaign_imported_begin(&c,8));draw(&c,&u);
 expected=c;assert(campaign_choice(&expected,0));campaign_touch(&c,&u,80,60);assert(!memcmp(&c,&expected,sizeof c)&&c.imported_battle==4);draw(&c,&u);
 Game held=c.game;for(int i=0;i<100;i++)campaign_step(&c);assert(!memcmp(&held,&c.game,sizeof held));
 assert(campaign_choice(&expected,0));ui_event_key(&c,&u,2);assert(!memcmp(&c,&expected,sizeof c)&&c.mode==CAM_COMBAT);u.feedback_ticks=0;
 draw(&c,&u);memcpy(prior,top.pixels,sizeof prior);c.game.enemy.systems[SYS_ENGINES].power=0;draw(&c,&u);assert(memcmp(prior+102*256,top.pixels+102*256,17*256*sizeof(uint16_t)));
 c.game.enemy.systems[SYS_ENGINES].power=1;c.game.enemy_escape_ticks=1;campaign_step(&c);assert(c.game.outcome==GAME_ESCAPE&&c.mode==CAM_EVENT);draw(&c,&u);
 /* Display departed hull with stale aim/boarding pages; no remote art or rooms. */
 c.mode=CAM_COMBAT;u.aiming=1;draw(&c,&u);memcpy(prior,top.pixels,sizeof prior);
 c.game.target_room=c.game.enemy.rooms[0].id;draw(&c,&u);assert(!memcmp(prior,top.pixels,sizeof prior));
 c.game.outcome=GAME_PLAYING;u.aiming=0;u.boarding=1;u.board_side=1;draw(&c,&u);

 /* After departure, touch can still switch a bomb to the home ship. */
 assert(game_equip(&c.game,0,6));u=(Ui){.weapon=0,.aiming=1};c.game.weapons[0].bomb_side=0;
 campaign_touch(&c,&u,30,139);assert(c.game.weapons[0].bomb_side==1);draw(&c,&u);
 puts("Escape UI: touch/button warning flow, frozen notice, charging/stopped distinction, departure/stale-page rendering and framebuffer guards passed");return 0;}
