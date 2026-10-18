/* V69: every weapon keeps its own target room (as in the original); B clears one weapon's
   target so it holds fire, autofire included; the aim screen numbers each weapon's target. */
#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;static Ui u;static uint16_t top[256*192],bottom[256*192];
static void key(int k){ui_combat_key(&c,&u,k);}
static int launched_at(int room){for(int i=0;i<GAME_MAX_EFFECTS;i++){if(c.game.effects[i].active&&c.game.effects[i].side==0&&c.game.effects[i].room==room)return 1;}return 0;}
int main(void){audio_init();campaign_init_seed(&c,42);assert(campaign_jump(&c,1));u=(Ui){0};
 Game*g=&c.game;assert(g->weapon_count>=2&&g->enemy.room_count>=3);
 for(int i=0;i<2;i++){assert(!g->weapons[i].beam_length&&!g->weapons[i].bomb);}
 int ra=g->enemy.rooms[0].id,rb=g->enemy.rooms[g->enemy.room_count-1].id;assert(ra>=0&&rb>=0&&ra!=rb);
 /* Buttons: Y aims, L/R picks the weapon, the room cursor sets only that weapon's target. */
 key(UI_COMBAT_Y);assert(u.aiming);ui_select_weapon(g,&u,0);assert(game_set_weapon_target(g,0,ra));
 ui_select_weapon(g,&u,1);assert(game_set_weapon_target(g,1,rb));g->target_room=rb;
 assert(game_weapon_aim(g,0)==ra&&game_weapon_aim(g,1)==rb);
 render_campaign(&c,&u,top,bottom);
 /* Each charged weapon fires at its own room on autofire. */
 for(int i=0;i<2;i++){g->autofire[i]=1;g->weapons[i].enabled=1;}g->enemy.shields=0;
 int seen_a=0,seen_b=0;for(int t=0;t<40*GAME_HZ&&!(seen_a&&seen_b)&&g->outcome==GAME_PLAYING;t++){g->enemy.hull=20;game_step(g);seen_a|=launched_at(ra);seen_b|=launched_at(rb);}
 assert(seen_a&&seen_b);
 /* B clears the selected weapon's target: it holds fire while the other keeps firing. */
 ui_select_weapon(g,&u,0);key(UI_COMBAT_B);assert(u.aiming&&g->weapons[0].aim_room==GAME_AIM_HOLD&&game_weapon_aim(g,0)<0&&game_weapon_aim(g,1)==rb);
 render_campaign(&c,&u,top,bottom);
 for(int i=0;i<GAME_MAX_EFFECTS;i++){g->effects[i].active=0;}g->weapons[0].charge_ticks=g->weapons[0].cooldown_ticks;g->weapons[0].pending_shots=0;
 for(int t=0;t<20*GAME_HZ&&g->outcome==GAME_PLAYING;t++){g->enemy.hull=20;game_step(g);assert(!g->weapons[0].pending_shots);}assert(g->outcome==GAME_PLAYING);
 /* Picking a room again re-arms it; a second B leaves the aim screen. */
 ui_select_weapon(g,&u,0);key(UI_COMBAT_DOWN);assert(game_weapon_aim(g,0)>=0);key(UI_COMBAT_B);assert(u.aiming&&game_weapon_aim(g,0)<0);key(UI_COMBAT_B);assert(!u.aiming);
 /* Touch: tapping a room on the aim screen sets the selected weapon's target. */
 key(UI_COMBAT_Y);ui_select_weapon(g,&u,0);assert(game_weapon_aim(g,0)<0);
 {int ok=0;for(int y=48;y<160&&!ok;y+=4){for(int x=8;x<248&&!ok;x+=4){campaign_touch(&c,&u,x,y);ok=game_weapon_aim(g,0)>=0;}}assert(ok);}
 printf("PASS: per-weapon targets: own rooms on autofire, B clears one weapon (holds fire), buttons and touch re-aim, numbered markers drawn.\n");
 return 0;
}
