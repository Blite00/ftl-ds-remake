#include "game.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void setup(int kind){game_init(&g);game_begin_encounter(&g,kind,0,0);g.enemy_weapon.cooldown_ticks=100000;g.enemy_drone.cooldown_ticks=100000;}
static void finish(void){for(int i=0;i<1000&&g.outcome==GAME_PLAYING;i++)game_step(&g);}
int main(void){
 assert(GAME_PLAYING==0&&GAME_VICTORY==1&&GAME_DEFEAT==2&&GAME_ESCAPE==3);
 assert(RESULT_ENEMY_CREW==4&&RESULT_ENEMY_ESCAPE==5);
 setup(ENEMY_SCOUT);for(int i=0;i<100;i++)game_step(&g);assert(!g.enemy_escape_active&&!g.enemy_departed&&g.outcome==GAME_PLAYING);
 assert(!game_enemy_escape_start(&g,-1,0,0));assert(!game_enemy_escape_start(&g,1,5,4));
 /* Configuration consumes RNG only when selecting a nonconstant threshold. */
 unsigned rng=g.rng;assert(game_enemy_escape_start(&g,10,999,999));assert(g.rng==rng);assert(!game_enemy_escape_start(&g,20,999,999));assert(g.enemy_escape_total==10);
 game_step(&g);int before=g.enemy_escape_ticks;assert(before>0);g.paused=1;game_step(&g);assert(g.enemy_escape_ticks==before);g.paused=0;
 g.enemy.systems[SYS_ENGINES].power=0;game_step(&g);assert(g.enemy_escape_ticks==before&&!game_enemy_escape_ready(&g));
 g.enemy.systems[SYS_ENGINES].power=1;g.enemy.systems[SYS_ENGINES].ion_bars=1;g.enemy.systems[SYS_ENGINES].ion_ticks=10;game_step(&g);assert(g.enemy_escape_ticks==before);
 g.enemy.systems[SYS_ENGINES].ion_ticks=g.enemy.systems[SYS_ENGINES].ion_bars=0;
 g.enemy_pilot_room=-1;assert(game_enemy_escape_ready(&g));game_step(&g);assert(g.enemy_escape_ticks<before);
 g.enemy_pilot_room=g.enemy.rooms[0].id;g.enemy_pilot_health=1;g.pilot_ion_ticks[1]=10;before=g.enemy_escape_ticks;game_step(&g);assert(g.enemy_escape_ticks==before);
 g.pilot_ion_ticks[1]=0;g.enemy_pilot_health=0;game_step(&g);assert(g.enemy_escape_ticks==before);
 g.enemy_pilot_health=1;finish();assert(g.outcome==GAME_ESCAPE&&g.result_reason==RESULT_ENEMY_ESCAPE&&g.enemy_departed);
 unsigned id=g.crew[0].id;game_step(&g);assert(g.crew[0].id==id);game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(!g.enemy_departed&&!g.enemy_escape_active);
 /* Damage triggers one latched attempt; healing above threshold does not undo it. */
 setup(ENEMY_SCOUT);int hull=g.enemy.hull;assert(game_enemy_escape_start(&g,20,hull-1,hull-1));game_step(&g);assert(!g.enemy_escape_active&&g.enemy_escape_ticks==20);
 g.enemy.hull--;game_step(&g);assert(g.enemy_escape_active&&g.enemy_escape_ticks==19);g.enemy.hull++;game_step(&g);assert(g.enemy_escape_ticks==18);
 /* A manned living cockpit is mandatory for nonautomated ships. */
 setup(ENEMY_REBEL);for(int i=0;i<g.enemy_crew_count;i++){g.enemy_crew[i].room=g.enemy_crew[i].destination=g.enemy.systems[SYS_OXYGEN].room;}assert(!game_enemy_escape_ready(&g));
 g.enemy_crew[0].room=g.enemy_crew[0].destination=g.enemy_pilot_room;assert(game_enemy_escape_ready(&g));g.enemy_crew[0].destination=g.enemy.systems[SYS_OXYGEN].room;assert(!game_enemy_escape_ready(&g));
 /* A real system impact interrupts the last charge tick before escape checks. */
 setup(ENEMY_SCOUT);assert(game_enemy_escape_start(&g,10,999,999));g.enemy_escape_ticks=1;g.effects[0]=(GameEffect){.active=1,.duration=1,.side=0,.room=g.enemy.systems[SYS_ENGINES].room,.kind=EFFECT_LASER,.system_damage=8,.piercing=99};g.enemy.systems[SYS_ENGINES].power=0;game_step(&g);assert(g.enemy_escape_ticks==1&&!g.enemy.systems[SYS_ENGINES].health&&!g.enemy_departed);
 /* Hull damage and crew elimination on the last charging tick win over escape. */
 setup(ENEMY_SCOUT);assert(game_enemy_escape_start(&g,10,999,999));game_step(&g);g.enemy_escape_ticks=1;g.enemy.hull=0;game_step(&g);assert(g.outcome==GAME_VICTORY&&g.result_reason==RESULT_ENEMY_HULL&&!g.enemy_departed);
 setup(ENEMY_REBEL);assert(game_enemy_escape_start(&g,10,999,999));game_step(&g);g.enemy_escape_ticks=1;for(int i=0;i<g.enemy_crew_count;i++)g.enemy_crew[i].health=0;game_step(&g);assert(g.outcome==GAME_VICTORY&&g.result_reason==RESULT_ENEMY_CREW&&!g.enemy_departed);
 setup(ENEMY_SCOUT);assert(game_enemy_escape_start(&g,10,999,999));game_step(&g);g.enemy_escape_ticks=1;g.player.hull=g.enemy.hull=0;game_step(&g);assert(g.outcome==GAME_DEFEAT&&g.result_reason==RESULT_PLAYER_HULL&&!g.enemy_departed);
 /* Crew lost on the escaped ship retain identity; home crew and boarders remain. */
 setup(ENEMY_REBEL);g.enemy_crew[0].room=g.enemy_crew[0].destination=g.enemy_pilot_room;g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=g.enemy.rooms[0].id;g.crew[0].species=SPEC_ZOLTAN;unsigned lost=g.crew[0].id,home=g.crew[1].id;
 g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=g.player.rooms[0].id;
 assert(game_enemy_escape_start(&g,10,999,999));game_step(&g);g.enemy_escape_ticks=1;game_step(&g);
 assert(g.enemy_departed&&g.outcome==GAME_PLAYING&&g.result_reason==RESULT_ENEMY_ESCAPE);assert(!g.crew[0].health&&g.crew[0].death_processed&&g.crew[0].id==lost&&g.crew[1].id==home&&g.crew[1].health>0);assert(g.enemy_crew[1].health>0);
 /* Remote shots reject before spending ammo or losing charge; home bombs still
  * launch, travel and damage the surviving hostile party through normal effects. */
 g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;int ammo=g.missiles,charge=g.weapons[0].charge_ticks;
 assert(!game_fire(&g,0,g.enemy.rooms[0].id));assert(g.missiles==ammo&&g.weapons[0].charge_ticks==charge&&!g.weapons[0].pending_shots);
 assert(strstr(g.message,"No remote target"));
 assert(game_equip(&g,0,6));g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
 assert(!game_fire(&g,0,g.enemy.rooms[0].id)&&g.missiles==ammo);
 int target=g.enemy_crew[1].room;g.enemy_crew[1].health=90;
 assert(game_set_bomb_target(&g,0,1,target)&&game_fire(&g,0,target));assert(g.missiles==ammo-1&&g.weapons[0].pending_shots==1);
 game_step(&g);assert(!g.weapons[0].pending_shots&&g.effects[0].active&&g.effects[0].kind==EFFECT_BOMB&&g.effects[0].bomb_friendly);
 for(int i=0;i<12;i++)game_step(&g);
 assert(g.effects[0].result==EFFECT_HIT&&g.enemy_crew[1].health==60&&g.outcome==GAME_PLAYING&&!g.enemy.hull&&g.result_reason==RESULT_ENEMY_ESCAPE);
 for(int i=0;i<g.enemy_crew_count;i++)if(g.enemy_crew[i].ship==1)assert(!g.enemy_crew[i].health);
 /* Accelerated cooldown fixture verifies repeated HOME autofire after departure. */
 Game before_auto=g;g.autofire[0]=1;g.weapons[0].cooldown_ticks=2;g.weapons[0].charge_ticks=2;int auto_ammo=g.missiles;
 for(int i=0;i<3;i++)game_step(&g);
 assert(g.missiles==auto_ammo-2&&g.effects[1].active&&g.effects[2].active);g=before_auto;
 Game after_bomb=g;g.player.hull=0;game_step(&g);assert(g.outcome==GAME_DEFEAT&&g.result_reason==RESULT_PLAYER_HULL);g=after_bomb;
 g.enemy_crew[1].health=30;g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;assert(game_fire(&g,0,target));
 for(int i=0;i<13;i++)game_step(&g);
 assert(!g.enemy_crew[1].health&&g.outcome==GAME_ESCAPE&&g.result_reason==RESULT_ENEMY_ESCAPE);g=after_bomb;
 /* Resolve remaining boarders through ordinary melee, not a salvage shortcut. */
 g.enemy_crew[1].health=1;g.crew[1].room=g.crew[1].destination=g.enemy_crew[1].room;g.crew[1].combat_ticks=GAME_HZ-1;game_step(&g);assert(!g.enemy_crew[1].health);assert(g.outcome==GAME_ESCAPE&&g.result_reason==RESULT_ENEMY_ESCAPE);Game copy=g;game_step(&g);assert(!memcmp(&g,&copy,sizeof g));
 setup(ENEMY_SCOUT);for(int i=0;i<g.crew_count;i++){g.crew[i].ship=1;g.crew[i].room=g.crew[i].destination=g.enemy.rooms[0].id;}assert(game_enemy_escape_start(&g,10,999,999));game_step(&g);g.enemy_escape_ticks=1;game_step(&g);assert(g.outcome==GAME_DEFEAT&&g.result_reason==RESULT_PLAYER_CREW);
 puts("Enemy escape: bounded opt-in charging, pause/power/ion/pilot stops, death priority, lost crew identity, boarder cleanup and one-shot departure passed");
}
