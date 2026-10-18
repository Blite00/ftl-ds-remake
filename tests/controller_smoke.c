/* Portable input integration, independent of libnds key masks. */
#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,b;static Ui u;static uint16_t region[256*18];
static struct {unsigned a;uint16_t p[256*192];unsigned b;} top,bottom;
static void draw(void){top.a=bottom.a=17;top.b=bottom.b=31;b=c;render_campaign(&c,&u,top.p,bottom.p);assert(!memcmp(&b,&c,sizeof c));assert(top.a==17&&bottom.a==17&&top.b==31&&bottom.b==31);}
static void key(int k){ui_combat_key(&c,&u,k);}
static void setup(void){campaign_init_seed(&c,42);assert(campaign_jump(&c,1));u=(Ui){0};}
static void actions(int row){key(UI_COMBAT_START);assert(u.actions);for(int i=0;i<row;i++)key(UI_COMBAT_DOWN);assert(u.action_selected==row);draw();}
int main(void){audio_init();setup();
 int crew=u.selected_crew;key(UI_COMBAT_R);assert(u.selected_crew!=crew&&u.controller_focus);key(UI_COMBAT_L);assert(u.selected_crew==crew);c.game.crew[1].health=0;key(UI_COMBAT_R);assert(u.selected_crew==2);c.game.crew[1].health=100;
 int moved=0;for(int direction=UI_COMBAT_UP;direction<=UI_COMBAT_RIGHT;direction++){int before=u.focus_room;key(direction);moved|=before!=u.focus_room;}assert(moved);key(UI_COMBAT_A);assert(c.game.crew[u.selected_crew].destination==u.focus_room);draw();
 key(UI_COMBAT_X);assert(c.game.paused);key(UI_COMBAT_X);assert(!c.game.paused);
 actions(0);b=c;for(int i=0;i<30;i++)ui_campaign_step(&c,&u);assert(!memcmp(&b,&c,sizeof c));int power=c.game.player.systems[0].power;key(UI_COMBAT_LEFT);assert(c.game.player.systems[0].power==power-1);key(UI_COMBAT_RIGHT);assert(c.game.player.systems[0].power==power);
 campaign_touch(&c,&u,199,39);assert(c.game.player.systems[0].power==power-1);campaign_touch(&c,&u,231,39);assert(c.game.player.systems[0].power==power);assert(u.actions);key(UI_COMBAT_B);assert(!u.actions);
 key(UI_COMBAT_Y);assert(u.aiming);int weapon=u.weapon;key(UI_COMBAT_R);assert(u.weapon!=weapon);key(UI_COMBAT_L);assert(u.weapon==weapon);int target=c.game.target_room;for(int direction=0;direction<4;direction++)key(direction);assert(c.game.weapons[u.weapon].aim_room==c.game.target_room);(void)target;
 c.game.weapons[u.weapon].charge_ticks=c.game.weapons[u.weapon].cooldown_ticks;key(UI_COMBAT_A);assert(c.game.weapons[u.weapon].pending_shots);draw();key(UI_COMBAT_B);assert(u.aiming&&c.game.weapons[u.weapon].aim_room==GAME_AIM_HOLD&&game_weapon_aim(&c.game,u.weapon)<0);draw();key(UI_COMBAT_B);assert(!u.aiming&&!u.retreat);
 actions(6);int auto_before=c.game.autofire[u.weapon];key(UI_COMBAT_A);assert(c.game.autofire[u.weapon]!=auto_before);key(UI_COMBAT_B);
 actions(7);key(UI_COMBAT_A);assert(u.actions==2);key(UI_COMBAT_DOWN);int door=c.game.doors[0].open;key(UI_COMBAT_A);assert(c.game.doors[0].open!=door);draw();key(UI_COMBAT_B);assert(u.actions==1);key(UI_COMBAT_B);
 actions(9);key(UI_COMBAT_A);assert(u.drone_panel&&!u.actions);key(UI_COMBAT_DOWN);assert(u.drone_focus==1);key(UI_COMBAT_B);assert(!u.drone_panel);
 actions(11);int volume=audio_volume_level();key(UI_COMBAT_A);assert(audio_volume_level()==(volume+1)%4);key(UI_COMBAT_B);
 actions(8);key(UI_COMBAT_A);assert(u.boarding);key(UI_COMBAT_START);assert(u.actions==3);draw();key(UI_COMBAT_B);key(UI_COMBAT_Y);assert(u.board_side==1);key(UI_COMBAT_RIGHT);key(UI_COMBAT_B);assert(!u.boarding);
 key(UI_COMBAT_B);assert(u.crew_deselected&&!u.retreat);key(UI_COMBAT_B);assert(u.retreat);key(UI_COMBAT_B);assert(!u.retreat);
 /* A previous battle result must not disable peaceful home crew management. */
 campaign_init_seed(&c,4);u=(Ui){.page=1};c.game.outcome=GAME_ESCAPE;c.game.result_reason=RESULT_PLAYER_ESCAPE;key(UI_COMBAT_R);assert(u.selected_crew==1);key(UI_COMBAT_RIGHT);key(UI_COMBAT_A);assert(c.game.crew[1].destination==u.focus_room);actions(0);b=c;ui_campaign_step(&c,&u);assert(!memcmp(&b,&c,sizeof c));key(UI_COMBAT_B);key(UI_COMBAT_Y);assert(u.page==2&&!u.aiming);u.page=1;key(UI_COMBAT_B);assert(u.crew_deselected);key(UI_COMBAT_B);assert(!u.page);
 /* Modal reward acceptance never leaks through to underlying combat controls. */
 setup();assert(rewards_offer_weapon(&c.reward,2));int selected=u.selected_crew;key(UI_COMBAT_R);assert(u.selected_crew==selected&&c.reward.kind);key(UI_COMBAT_A);assert(!c.reward.kind&&c.cargo[0]==2);
 /* Selected crew hints never replace home warnings or confirmed hit feedback. */
 setup();key(UI_COMBAT_R);c.game.player.rooms[0].fires=1;u.controller_focus=0;draw();memcpy(region,top.p+122*256,11*256*sizeof(uint16_t));u.controller_focus=1;draw();assert(!memcmp(region,top.p+122*256,11*256*sizeof(uint16_t)));
 snprintf(c.game.message,sizeof c.game.message,"Weapons hit: system damaged.");u.controller_focus=0;draw();memcpy(region,top.p+174*256,sizeof region);u.controller_focus=1;draw();assert(!memcmp(region,top.p+174*256,sizeof region));
 /* Beam endpoints and bomb side are reachable without touch. */
 setup();assert(game_equip(&c.game,0,5));key(UI_COMBAT_Y);key(UI_COMBAT_A);assert(u.beam_pending);for(int direction=0;direction<4&&u.beam_pending;direction++){key(direction);key(UI_COMBAT_A);}assert(c.game.weapons[0].beam_target_valid&&!u.beam_pending);actions(12);key(UI_COMBAT_A);assert(!c.game.weapons[0].beam_target_valid);key(UI_COMBAT_B);
 assert(game_equip(&c.game,0,6));actions(12);key(UI_COMBAT_A);assert(c.game.weapons[0].bomb_side==1);draw();key(UI_COMBAT_B);
 /* Terminal combat cannot change power through a stale menu. */
 setup();actions(0);c.game.outcome=GAME_VICTORY;b=c;key(UI_COMBAT_LEFT);assert(!memcmp(&b,&c,sizeof c));key(UI_COMBAT_B);assert(!u.actions);
 /* Touch remains usable after controller focus and menu return. */
 setup();key(UI_COMBAT_R);actions(0);key(UI_COMBAT_B);campaign_touch(&c,&u,220,183);assert(u.aiming);draw();
 setup();campaign_touch(&c,&u,255,21);assert(u.crew_deselected&&!u.crew_group&&!u.controller_focus);b=c;
 key(UI_COMBAT_A);assert(!memcmp(&b,&c,sizeof c));
 campaign_touch(&c,&u,20,120);assert(!u.crew_deselected&&!u.crew_details);
 campaign_touch(&c,&u,20,120);assert(u.crew_details);key(UI_COMBAT_B);
 key(UI_COMBAT_B);assert(u.crew_deselected);key(UI_COMBAT_R);assert(!u.crew_deselected);draw();
 puts("Controller: spatial focus/orders, crew/weapon cycling, fire/pause, frozen actions, power touch +/- and keys, doors, drones, boarding entry, retreat, volume and peaceful/modal routing passed.");return 0;}
