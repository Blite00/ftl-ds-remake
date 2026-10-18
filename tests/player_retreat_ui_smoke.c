#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;
static struct {unsigned first;uint16_t p[256*192];unsigned last;} top,bottom;
static void draw(Ui*u,const char*path){
 before=c;top.first=bottom.first=17;top.last=bottom.last=31;render_campaign(&c,u,top.p,bottom.p);
 assert(!memcmp(&c,&before,sizeof c));assert(top.first==17&&bottom.first==17&&top.last==31&&bottom.last==31);
 if(path){FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");for(int y=0;y<384;y++)for(int x=0;x<256;x++){unsigned v=y<192?top.p[y*256+x]:bottom.p[(y-192)*256+x];unsigned char rgb[3]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);}
}
static void combat(void){campaign_init_seed(&c,2828);assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT);}
int main(int argc,char**argv){Ui u={0};audio_init();combat();before=c;
 campaign_touch(&c,&u,180,184);assert(u.retreat&&!memcmp(&c,&before,sizeof c));draw(&u,argc>1?argv[1]:0);
 for(int tick=0;tick<60;tick++)ui_campaign_step(&c,&u);
 assert(!memcmp(&c,&before,sizeof c));
 ui_retreat_key(&c,&u,2);assert(u.retreat&&u.feedback_error&&!memcmp(&c,&before,sizeof c));ui_retreat_key(&c,&u,3);assert(!u.retreat&&!memcmp(&c,&before,sizeof c));
 c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;c.game.crew[1].ship=1;c.game.crew[1].room=c.game.crew[1].destination=c.game.enemy.rooms[0].id;
 before=c;ui_retreat_open(&c,&u);int selected=u.retreat_selected;ui_retreat_key(&c,&u,1);assert(campaign_linked(&c,c.current,u.retreat_selected));ui_retreat_key(&c,&u,0);assert(u.retreat_selected==selected);
 ui_retreat_key(&c,&u,2);assert(u.retreat_confirm&&!memcmp(&c,&before,sizeof c));draw(&u,argc>2?argv[2]:0);
 ui_retreat_key(&c,&u,3);assert(u.retreat&&!u.retreat_confirm&&!memcmp(&c,&before,sizeof c));
 ui_retreat_key(&c,&u,2);campaign_touch(&c,&u,190,184);assert(!u.retreat&&c.current==selected&&!c.game.crew[1].health&&c.fuel==before.fuel-1);
 /* Unresolved arrival can defend without executing a choice or buying upgrades. */
 combat();c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;c.links[1]|=1<<2;c.links[2]|=1<<1;c.node_kind[2]=NODE_SUPPLY;c.event_policy=0;
 game_init_crew(&c.game,1,0,c.game.player.systems[SYS_WEAPONS].room);c.game.enemy_crew[0].ship=0;c.game.enemy_crew_count=1;
 assert(campaign_retreat(&c,2,0)&&!c.event_resolved);u=(Ui){0};int event=c.event_id,fuel=c.fuel,scrap=c.scrap;ui_home_open(&c,&u);assert(u.home_defense);
 draw(&u,argc>3?argv[3]:0);campaign_touch(&c,&u,190,184);assert(c.game.paused);campaign_touch(&c,&u,190,184);assert(!c.game.paused);
 for(int i=0;i<10;i++)ui_campaign_step(&c,&u);
 assert(c.event_id==event&&!c.event_resolved&&c.fuel==fuel&&c.scrap==scrap);
 campaign_touch(&c,&u,20,184);assert(!u.home_defense&&!c.event_resolved);draw(&u,argc>4?argv[4]:0);
 /* Remote side of boarding controls is safe after departure removed its rooms. */
 u.home_defense=1;u.boarding=1;u.board_side=1;draw(&u,0);
 c.game.player.hull=0;ui_campaign_step(&c,&u);assert(c.mode==CAM_DEFEAT);draw(&u,0);
 uint16_t terminal[256*192];memcpy(terminal,bottom.p,sizeof terminal);Ui clean={0};draw(&clean,0);assert(!memcmp(terminal,bottom.p,sizeof terminal));
 Campaign ended=c;campaign_touch(&c,&clean,100,180);assert(c.mode==CAM_DEFEAT&&clean.menu_requested&&!memcmp(&c,&ended,sizeof c));draw(&clean,0);

 puts("Retreat UI: pure open/cancel, blocked charge feedback, linked selection, explicit abandonment, home defense, event preservation and framebuffer guards passed");return 0;
}
