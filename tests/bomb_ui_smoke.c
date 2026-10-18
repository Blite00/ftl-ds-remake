#include "render.h"
#include "audio.h"
#include "subsystems.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct { unsigned before; uint16_t p[256*192]; unsigned after; } a,b;
static uint16_t oldtop[256*192],oldbottom[256*192];
static void draw(Campaign*c,Ui*u){a.before=b.before=17;a.after=b.after=31;render_campaign(c,u,a.p,b.p);assert(a.before==17&&b.before==17&&a.after==31&&b.after==31);}
int main(void){Campaign c;Ui u={.weapon=2,.aiming=1};audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,6));c.game.weapons[0].enabled=c.game.weapons[1].enabled=0;
 int room=c.game.enemy.systems[SYS_WEAPONS].room;c.game.target_room=c.game.weapons[2].aim_room=room;c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;
 c.game.missiles=0;ui_fire(&c.game,&u);assert(u.feedback_error&&!c.game.weapons[2].pending_shots);c.game.missiles=3;ui_fire(&c.game,&u);assert(!u.feedback_error);game_step(&c.game);assert(c.game.missiles==2);
 int effect=-1;for(int i=0;i<GAME_MAX_EFFECTS;i++)if(c.game.effects[i].active&&c.game.effects[i].kind==EFFECT_BOMB)effect=i;assert(effect>=0);GameEffect*f=&c.game.effects[effect];assert(f->room==room);
 /* Bomb animation lives at the chosen room, never in the travel corridor. */
 u.feedback_ticks=u.tap_ticks=0;f->age=f->duration/2;draw(&c,&u);memcpy(oldtop,a.p,sizeof oldtop);f->active=0;draw(&c,&u);for(int y=50;y<100;y++)for(int x=120;x<150;x++)assert(oldtop[y*256+x]==a.p[y*256+x]);f->active=1;
 /* Effects must not disclose hidden room oxygen, crew health or hazards. */
 draw(&c,&u);memcpy(oldtop,a.p,sizeof oldtop);memcpy(oldbottom,b.p,sizeof oldbottom);for(int i=0;i<c.game.enemy.room_count;i++){c.game.enemy.rooms[i].oxygen=123;c.game.enemy.rooms[i].fires=1;c.game.enemy.rooms[i].breaches=1;}for(int i=0;i<c.game.enemy_crew_count;i++)c.game.enemy_crew[i].health=42;draw(&c,&u);assert(!memcmp(oldtop,a.p,sizeof oldtop)&&!memcmp(oldbottom,b.p,sizeof oldbottom));
 f->age=f->duration;f->result=EFFECT_HIT;draw(&c,&u);memcpy(oldtop,a.p,sizeof oldtop);f->result=EFFECT_MISS;draw(&c,&u);assert(memcmp(oldtop,a.p,sizeof oldtop));
 for(int n=0;n<40;n++){game_step(&c.game);draw(&c,&u);}
 /* Ownship selection uses the same touchscreen room geometry and snapshots side. */
 memset(c.game.effects,0,sizeof c.game.effects);ui_bomb_toggle_side(&c.game,&u);assert(c.game.weapons[2].bomb_side==1);u.feedback_ticks=u.tap_ticks=0;c.game.subsystems[0][SUB_SENSORS].health=0;draw(&c,&u);memcpy(oldtop,a.p,sizeof oldtop);memcpy(oldbottom,b.p,sizeof oldbottom);for(int i=0;i<c.game.player.room_count;i++){GameRoom*hidden=&c.game.player.rooms[i];if(!game_room_visible(&c.game,0,hidden->id)){hidden->fires=1;hidden->breaches=1;hidden->oxygen=321;}}draw(&c,&u);assert(!memcmp(oldtop,a.p,sizeof oldtop)&&!memcmp(oldbottom,b.p,sizeof oldbottom));c.game.subsystems[0][SUB_SENSORS].health=1;int mx=1000,my=1000,xx=-1000,yy=-1000;for(int i=0;i<c.game.player.room_count;i++){GameRoom*r=&c.game.player.rooms[i];if(r->x<mx)mx=r->x;if(r->y<my)my=r->y;if(r->x+r->w>xx)xx=r->x+r->w;if(r->y+r->h>yy)yy=r->y+r->h;}int t=24;if((xx-mx)*t>232)t=232/(xx-mx);if((yy-my)*t>64)t=64/(yy-my);GameRoom*r=&c.game.player.rooms[1];int x=(256-(xx-mx)*t)/2+(r->x-mx)*t+r->w*t/2,y=44+(64-(yy-my)*t)/2+(r->y-my)*t+r->h*t/2;campaign_touch(&c,&u,x,y);assert(c.game.weapons[2].aim_room==r->id&&c.game.weapons[2].bomb_side==1);
 assert(game_set_bomb_target(&c.game,2,1,c.game.player.rooms[c.game.player.room_count-1].id));c.game.target_room=c.game.weapons[2].aim_room;ui_select_weapon(&c.game,&u,1);int found=0;for(int i=0;i<c.game.enemy.room_count;i++)if(c.game.enemy.rooms[i].id==c.game.target_room)found=1;assert(found);assert(game_equip(&c.game,3,5));ui_select_weapon(&c.game,&u,2);ui_select_weapon(&c.game,&u,3);found=0;for(int i=0;i<c.game.enemy.room_count;i++)if(c.game.enemy.rooms[i].id==c.game.target_room)found=1;assert(found&&!u.beam_pending);ui_select_weapon(&c.game,&u,2);assert(c.game.target_room==c.game.player.rooms[c.game.player.room_count-1].id&&c.game.weapons[2].bomb_side==1);
 c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;ui_fire(&c.game,&u);assert(!u.feedback_error);game_step(&c.game);effect=-1;for(int i=0;i<GAME_MAX_EFFECTS;i++)if(c.game.effects[i].active&&c.game.effects[i].kind==EFFECT_BOMB)effect=i;assert(effect>=0&&c.game.effects[effect].side==1);campaign_touch(&c,&u,50,139);assert(c.game.weapons[2].bomb_side==0&&c.game.effects[effect].side==1);draw(&c,&u);
 campaign_init_legacy(&c);c.mode=CAM_STORE;c.current=5;c.scrap=100;u=(Ui){.shop_page=2};draw(&c,&u);campaign_touch(&c,&u,90,100);assert(c.cargo[0]==6);Campaign before=c;u.shop_page=(CAM_OFFER_COUNT-1)/4;campaign_touch(&c,&u,90,130);assert(!memcmp(&before,&c,sizeof c));
 puts("Bomb UI: ammo feedback, room teleport without travel, hit/miss distinction, hidden-room isolation, store and framebuffer bounds passed");return 0;}
