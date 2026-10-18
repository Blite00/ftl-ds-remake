/* Host renderer fixture: enemy system health is public on the target card (V47, as in the original). */
#include "render.h"
#include "subsystems.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned pre;uint16_t p[256*192];unsigned post;} top,bottom;
static uint16_t card[256*72];static Game g,prior;static Ui u;
static void draw(void){top.pre=bottom.pre=17;top.post=bottom.post=31;prior=g;render_game(&g,&u,top.p,bottom.p);assert(!memcmp(&g,&prior,sizeof g));assert(top.pre==17&&bottom.pre==17&&top.post==31&&bottom.post==31);}
static void snapshot(void){memcpy(card,top.p+44*256,sizeof card);}
int main(void){
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,1,0));u=(Ui){.aiming=1};ui_select_weapon(&g,&u,0);assert(game_set_weapon_target(&g,0,g.enemy.systems[SYS_WEAPONS].room));g.target_room=g.enemy.systems[SYS_WEAPONS].room;
 g.enemy.systems[SYS_WEAPONS].capacity=3;g.enemy.systems[SYS_WEAPONS].health=3;draw();snapshot();g.enemy.systems[SYS_WEAPONS].health=0;draw();assert(memcmp(card,top.p+44*256,sizeof card)); /* V47: enemy system damage is public. */
 g.subsystems[0][SUB_SENSORS].health=g.subsystems[0][SUB_SENSORS].capacity=3;draw();snapshot();g.enemy.systems[SYS_WEAPONS].health=1;draw();assert(memcmp(card,top.p+44*256,sizeof card));
 for(int system=0;system<GAME_SYSTEM_COUNT;system++){assert(game_set_weapon_target(&g,0,g.enemy.systems[system].room));g.target_room=g.enemy.systems[system].room;draw();}
 for(int weapon=0;weapon<GAME_CATALOG_COUNT;weapon++){if(!game_weapon_player(weapon)){assert(!game_equip(&g,0,weapon));continue;}assert(game_equip(&g,0,weapon));ui_select_weapon(&g,&u,0);draw();if(g.weapons[0].bomb){ui_bomb_toggle_side(&g,&u);draw();}}
 puts("Target card: visible health changes, public enemy system damage, every main system/weapon and home bomb framebuffer bounds passed.");return 0;
}
