#include "render.h"
#include "audio.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} top,bottom;
static uint16_t normal[256*192],fire[256*192];
static void draw(Campaign*c,Ui*u){top.before=bottom.before=0x1122;top.after=bottom.after=0x3344;render_campaign(c,u,top.pixels,bottom.pixels);assert(top.before==0x1122&&bottom.before==0x1122&&top.after==0x3344&&bottom.after==0x3344);}
int main(void){
 Campaign c;Ui u={.weapon=1};campaign_init_legacy(&c);audio_init();c.mode=CAM_COMBAT;draw(&c,&u);memcpy(normal,bottom.pixels,sizeof normal);
 c.game.player.rooms[0].fires=1;draw(&c,&u);memcpy(fire,bottom.pixels,sizeof fire);
 c.game.player.rooms[0].fires=0;c.game.player.rooms[0].breaches=1;c.game.player.rooms[0].breach_repair_ticks=70;draw(&c,&u);
 assert(memcmp(normal,bottom.pixels,sizeof normal)&&memcmp(fire,bottom.pixels,sizeof fire));
 assert(!strcmp(game_crew_action(&c.game,0),"SEAL"));u.crew_details=1;draw(&c,&u);u.crew_details=0;
 /* Bounded extremes must render on all imported/procedural geometry and panels. */
 for(int kind=0;kind<ENEMY_KIND_COUNT;kind++){
  game_begin_encounter(&c.game,kind,5,0);c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
  for(int side=0;side<2;side++){Ship*s=side?&c.game.enemy:&c.game.player;for(int r=0;r<s->room_count;r++){int area=s->rooms[r].w*s->rooms[r].h;s->rooms[r].breaches=area<4?area:4;s->rooms[r].fires=area;s->rooms[r].breach_repair_ticks=149;}}
  for(int tick=0;tick<12;tick++){c.game.ticks=tick;u.aiming=0;u.boarding=0;draw(&c,&u);u.aiming=1;draw(&c,&u);u.boarding=1;u.board_side=0;u.board_room=c.game.player.rooms[0].id;draw(&c,&u);u.board_side=1;u.board_room=c.game.enemy.rooms[0].id;draw(&c,&u);}
 }
 puts("Breach UI: distinct flame/breach pixels, SEAL action, all enemy layouts and maximum hazard framebuffer bounds passed");
}
