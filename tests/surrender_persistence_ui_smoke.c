/* Core integration fixtures; playable source offers have separate coverage. */
#include "render.h"
#include "save.h"
#include "outcome.h"
#include "audio.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static struct {unsigned first;uint16_t pixels[256*192];unsigned last;} top,bottom;
static void draw(Ui*u,const char*path){
 before=c;top.first=bottom.first=17;top.last=bottom.last=31;render_campaign(&c,u,top.pixels,bottom.pixels);assert(!memcmp(&c,&before,sizeof c));
 assert(top.first==17&&bottom.first==17&&top.last==31&&bottom.last==31);
 if(path){FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");for(int y=0;y<384;y++)for(int x=0;x<256;x++){unsigned v=y<192?top.pixels[y*256+x]:bottom.pixels[(y-192)*256+x];unsigned char rgb[3]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31};fwrite(rgb,1,3,f);}assert(!fclose(f));}
}
int main(int argc,char**argv){audio_init();campaign_init_seed(&c,2901);assert(game_begin_encounter(&c.game,ENEMY_REBEL_BOARDER,2,0));c.mode=CAM_COMBAT;
 Game*g=&c.game;g->crew[1].ship=1;g->crew[1].room=g->crew[1].destination=g->enemy.rooms[0].id;g->crew[1].health=71;g->crew[1].skill[2]=42;unsigned ally=g->crew[1].id;
 g->enemy_crew[1].ship=0;g->enemy_crew[1].room=g->enemy_crew[1].destination=g->pilot_room;
 g->enemy_crew[2].ship=0;g->enemy_crew[2].room=g->enemy_crew[2].destination=g->pilot_room;g->enemy_crew[2].health=0;g->enemy_crew[2].species=SPEC_ZOLTAN;g->enemy_crew[2].death_processed=0;unsigned pending=g->enemy_crew[2].id;
 int hull=g->enemy.hull,scrap=c.scrap;assert(game_accept_surrender(g));assert(g->enemy.hull==hull&&outcome_duration(g)==0&&g->crew[1].id==ally&&!g->crew[1].ship);
 Ui u={.aiming=1,.weapon=1};draw(&u,argc>1?argv[1]:0);u=(Ui){.boarding=1,.board_side=1};draw(&u,0);
 /* The event callback owns this transition in play. Here its resolved shell
    verifies that existing map persistence can preserve negotiated crew state. */
 c.mode=CAM_EVENT;c.event_id=EVENT_AFTERMATH;c.event_resolved=1;snprintf(c.text,sizeof c.text,"Ceasefire fixture. Both ships remain intact.");campaign_continue(&c);assert(c.mode==CAM_MAP&&c.scrap==scrap);
 const char*path="tests/surrender-state.sav";assert(campaign_save(&c,path));campaign_init_seed(&d,99);assert(campaign_load(&d,path));
 assert(d.game.crew[1].id==ally&&!d.game.crew[1].ship&&d.game.crew[1].health==71&&d.game.crew[1].skill[2]==42);
 assert(d.game.enemy_crew[GAME_MAX_ENEMY_CREW].id==pending&&!d.game.enemy_crew[GAME_MAX_ENEMY_CREW].health&&!d.game.enemy_crew[GAME_MAX_ENEMY_CREW].death_processed&&d.scrap==scrap);
 c=d;u=(Ui){.page=1};draw(&u,argc>2?argv[2]:0);int hp=c.game.crew[0].health;ui_campaign_step(&c,&u);assert(c.game.crew[0].health==hp-15&&c.game.enemy_crew[GAME_MAX_ENEMY_CREW].death_processed);
 ui_campaign_step(&c,&u);assert(c.game.crew[0].health==hp-15);assert(campaign_save(&c,path));assert(campaign_load(&d,path));assert(d.game.crew[0].health==hp-15);
 const char*paths[]={"tests/surrender-state.sav","tests/surrender-state.sav.bak","tests/surrender-state.sav.tmp","tests/surrender-state.sav.bad"};for(int i=0;i<4;i++){FILE*f=fopen(paths[i],"rb");if(f){fclose(f);assert(!remove(paths[i]));}}
 puts("Surrender integration fixtures: intact renderer, exchanged crew save22, pending home death once across cold loads and framebuffer guards passed; source offers covered separately.");return 0;
}
