#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static WeaponState weapon,empty;
int main(void){int i,ammo,room;
 game_init(&g);assert(g.weapons[0].catalog_id==0&&g.weapons[1].catalog_id==1);g.player.systems[SYS_WEAPONS].power=2;
 assert(game_weapon_powered(&g,0)&&!game_weapon_powered(&g,1));assert(game_reorder_weapon(&g,1,0));assert(game_weapon_powered(&g,0)&&!game_weapon_powered(&g,1));assert(g.weapons[0].catalog_id==1);
 before=g;assert(!game_remove_weapon(&g,-1)&&!game_remove_weapon(&g,2)&&!game_reorder_weapon(&g,0,2)&&!game_reorder_weapon(&g,-1,0));assert(!memcmp(&before,&g,sizeof g));assert(game_reorder_weapon(&g,0,0));assert(!memcmp(&before,&g,sizeof g));
 /* Move all payload, targeting, charging and autofire identity together. */
 game_init(&g);assert(game_equip(&g,0,6));assert(game_equip(&g,2,5));assert(game_equip(&g,3,4));room=g.player.systems[SYS_WEAPONS].room;assert(game_set_bomb_target(&g,0,1,room));g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;assert(game_fire(&g,0,room));g.autofire[0]=1;weapon=g.weapons[0];ammo=g.missiles;
 assert(game_reorder_weapon(&g,0,3));assert(!memcmp(&weapon,&g.weapons[3],sizeof weapon)&&g.autofire[3]&&!g.autofire[0]&&g.missiles==ammo);assert(g.weapons[0].catalog_id==1&&g.weapons[1].catalog_id==5&&g.weapons[2].catalog_id==4);
 assert(game_reorder_weapon(&g,3,1));assert(!memcmp(&weapon,&g.weapons[1],sizeof weapon)&&g.autofire[1]&&g.weapons[2].catalog_id==5);
 /* Removing a different weapon retains pending payload on its owner only. */
 assert(game_remove_weapon(&g,0));assert(!memcmp(&weapon,&g.weapons[0],sizeof weapon)&&g.autofire[0]);assert(g.weapons[1].catalog_id==5&&!g.weapons[1].pending_shots&&!g.autofire[1]);assert(!memcmp(&empty,&g.weapons[3],sizeof empty)&&!g.autofire[3]);
 /* Removing a queued owner does not refund ammunition or copy its attack. */
 assert(game_remove_weapon(&g,0));assert(g.missiles==ammo&&g.weapons[0].catalog_id==5&&!g.weapons[0].pending_shots&&!g.autofire[0]);
 while(g.weapon_count)assert(game_remove_weapon(&g,0));
 for(i=0;i<GAME_MAX_WEAPONS;i++)assert(!memcmp(&empty,&g.weapons[i],sizeof empty)&&!g.autofire[i]);
 assert(!game_fire(&g,0,0)&&!game_toggle_weapon(&g,0)&&!game_toggle_autofire(&g,0)&&!game_set_weapon_target(&g,0,0)&&!game_weapon_powered(&g,0));g.enemy_weapon.cooldown_ticks=999999;for(i=0;i<30;i++){game_step(&g);game_service_step(&g);}assert(!g.weapon_count);
 assert(game_equip(&g,0,2)&&g.weapon_count==1&&g.weapons[0].catalog_id==2&&!g.weapons[0].pending_shots&&!g.autofire[0]);
 puts("PASS: displayed power priority, identity-preserving reorder/remove, pure bounds, zero weapons simulation and fresh re-equip.");return 0;
}

