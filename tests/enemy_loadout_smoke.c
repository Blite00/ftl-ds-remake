/* Controlled loadout fixtures; charge/system edits below isolate edge cases.
 * First timing test waits real catalog cooldowns without accelerated charges. */
#include "game.h"
#include "drones.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static void setup(void){game_init(&g);assert(game_begin_encounter(&g,ENEMY_ARMORED,0,0));g.player.systems[SYS_ENGINES].power=0;}
static void ticks(int n){while(n--)game_step(&g);}
static void load(int a,int b){int ids[]={a,b};assert(game_enemy_set_loadout(&g,ids,2,10));}
static int effects(int kind,int damage){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].kind==kind&&g.effects[i].damage==damage;return n;}
static void reject(const int*ids,int count,int missiles){before=g;assert(!game_enemy_set_loadout(&g,ids,count,missiles)&&!memcmp(&before,&g,sizeof g));}
int main(void){
 setup();assert(!g.enemy_weapon_count&&game_enemy_weapon_slots(&g)==1&&game_enemy_weapon_at(&g,0)==&g.enemy_weapon&&!game_enemy_weapon_at(&g,1));int ids[]={2,3};reject(ids,0,10);reject(ids,3,10);reject(ids,2,-1);reject(NULL,2,10);int bad[]={-1,GAME_CATALOG_COUNT,99}; /* V51: every catalog weapon, including beams and bombs, may arm an enemy. */for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++)reject(&bad[i],1,10);g.enemy.systems[SYS_WEAPONS].capacity=1;reject(ids,2,10);
 setup();load(2,3);assert(game_enemy_weapon_slots(&g)==2&&game_enemy_weapon_at(&g,1)==&g.enemy_weapon_secondary&&game_enemy_weapon_powered(&g,0)&&game_enemy_weapon_powered(&g,1));ticks(269);assert(g.enemy_weapon.charge_ticks==269&&g.enemy_weapon_secondary.charge_ticks==269&&!effects(EFFECT_LASER,2));game_step(&g);assert(g.enemy_weapon.charge_ticks==270&&!g.enemy_weapon_secondary.charge_ticks&&effects(EFFECT_LASER,2)==1);ticks(30);assert(!g.enemy_weapon.charge_ticks&&g.enemy_weapon_secondary.charge_ticks==30&&effects(EFFECT_LASER,1)==1&&g.enemy_fire_ticks==g.enemy_weapon.charge_ticks);
 /* Pause/service freeze both clocks, queues and visible projectiles. */
 before=g;g.paused=1;before.paused=1;ticks(50);assert(!memcmp(&g,&before,sizeof g));g.paused=0;int a=g.enemy_weapon.charge_ticks,b=g.enemy_weapon_secondary.charge_ticks;game_service_step(&g);assert(g.enemy_weapon.charge_ticks==a&&g.enemy_weapon_secondary.charge_ticks==b);
 setup();load(2,3);ticks(10);g.enemy.systems[SYS_WEAPONS].power=1;game_step(&g);assert(g.enemy_weapon.charge_ticks==11&&!g.enemy_weapon_secondary.charge_ticks&&game_enemy_weapon_powered(&g,0)&&!game_enemy_weapon_powered(&g,1));g.enemy.systems[SYS_WEAPONS].power=2;g.enemy.systems[SYS_WEAPONS].ion_bars=1;g.enemy.systems[SYS_WEAPONS].ion_ticks=100;game_step(&g);assert(g.enemy_weapon.charge_ticks==12&&!g.enemy_weapon_secondary.charge_ticks);g.enemy.systems[SYS_WEAPONS].ion_bars=0;game_step(&g);assert(g.enemy_weapon_secondary.charge_ticks==1);
 setup();load(1,2);g.enemy.systems[SYS_WEAPONS].power=1;assert(!game_enemy_weapon_powered(&g,0)&&game_enemy_weapon_powered(&g,1));game_step(&g);assert(!g.enemy_weapon.charge_ticks&&g.enemy_weapon_secondary.charge_ticks==1);
 /* Actual room damage stops both independent recharge clocks. */
 setup();load(2,3);ticks(20);g.enemy.systems[SYS_ENGINES].power=0;GameDamage hit={0,3,0,0,0,0,0,0};assert(game_launch_effect(&g,0,g.enemy.systems[SYS_WEAPONS].room,EFFECT_MISSILE,&hit,5));ticks(15);assert(!g.enemy.systems[SYS_WEAPONS].health&&!g.enemy_weapon.charge_ticks&&!g.enemy_weapon_secondary.charge_ticks);
 /* A ready pair launches together; power loss doesn't retarget/cancel already
  * queued burst rounds (planned distinct rooms), nor does a later payload edit rewrite their damage. */
 setup();load(1,3);g.enemy_weapon.charge_ticks=g.enemy_weapon.cooldown_ticks-1;g.enemy_weapon_secondary.charge_ticks=g.enemy_weapon_secondary.cooldown_ticks-1;game_step(&g);assert(effects(EFFECT_LASER,1)==1&&effects(EFFECT_LASER,2)==1&&g.enemy_weapon.pending_shots==2);int plan[GAME_MAX_VOLLEY];memcpy(plan,g.enemy_weapon.queued_targets,sizeof plan);assert(g.enemy_weapon.queued_target_count==3&&g.enemy_weapon.target_room==plan[0]&&plan[0]!=plan[1]&&plan[1]!=plan[2]&&plan[0]!=plan[2]);g.enemy.systems[SYS_WEAPONS].power=0;g.enemy_weapon.damage=9;ticks(6);assert(g.enemy_weapon.pending_shots==1&&g.enemy_weapon.target_room==plan[1]&&!memcmp(plan,g.enemy_weapon.queued_targets,sizeof plan)&&effects(EFFECT_LASER,1)>=1&&!effects(EFFECT_LASER,9));ticks(6);assert(!g.enemy_weapon.pending_shots&&!g.enemy_pending_shots);
 /* Ion payload uses the same real interception and lockout path. */
 setup();load(4,2);g.enemy_weapon.charge_ticks=g.enemy_weapon.cooldown_ticks-1;game_step(&g);assert(effects(EFFECT_ION,0)==1);ticks(12);assert(g.player.systems[SYS_SHIELDS].ion_bars>0&&!g.player.shields);
 /* Capacity saturation retains pending rounds for retry. */
 setup();load(1,3);for(int i=0;i<GAME_MAX_EFFECTS;i++){g.effects[i].active=1;g.effects[i].duration=1000;g.effects[i].side=0;}g.enemy_weapon.charge_ticks=g.enemy_weapon.cooldown_ticks-1;g.enemy_weapon_secondary.charge_ticks=g.enemy_weapon_secondary.cooldown_ticks-1;game_step(&g);assert(g.enemy_weapon.pending_shots==3&&g.enemy_weapon_secondary.pending_shots==1);g.effects[0].active=0;game_step(&g);assert(g.enemy_weapon.pending_shots==2&&g.enemy_weapon_secondary.pending_shots==1);reject(ids,2,10);
 /* Enemy Artemis is the source enemy variant, not player catalog0. Each
  * fired volley pays one finite missile; empty ammo cannot create free shots. */
 setup();load(2,GAME_ENEMY_ARTEMIS);assert(g.enemy_weapon_secondary.power==2&&g.enemy_weapon_secondary.cooldown_ticks==10*GAME_HZ&&g.enemy_weapon_secondary.damage==2&&g.enemy_weapon_secondary.missiles==1&&g.enemy_weapon_secondary.shield_piercing==5);g.enemy_missiles=1;ticks(300);assert(g.enemy_missiles==0&&effects(EFFECT_MISSILE,2)==1&&effects(EFFECT_LASER,1)==1);ticks(300);assert(!g.enemy_missiles&&!g.enemy_weapon_secondary.pending_shots&&g.enemy_weapon_secondary.charge_ticks==g.enemy_weapon_secondary.cooldown_ticks&&!effects(EFFECT_MISSILE,2));
 setup();g.enemy.systems[SYS_WEAPONS].capacity=g.enemy.systems[SYS_WEAPONS].health=g.enemy.systems[SYS_WEAPONS].power=4;load(GAME_ENEMY_ARTEMIS,GAME_ENEMY_ARTEMIS);g.enemy_missiles=1;g.enemy_weapon.charge_ticks=g.enemy_weapon.cooldown_ticks-1;g.enemy_weapon_secondary.charge_ticks=g.enemy_weapon_secondary.cooldown_ticks-1;game_step(&g);assert(!g.enemy_missiles&&effects(EFFECT_MISSILE,2)==1&&!g.enemy_weapon.charge_ticks&&g.enemy_weapon_secondary.charge_ticks==g.enemy_weapon_secondary.cooldown_ticks);
 /* Existing Defense I intercepts actual secondary-slot missiles, not lasers. */
 setup();load(2,GAME_ENEMY_ARTEMIS);assert(drones_install(&g));g.drones.capacity=g.drones.health=g.drones.power=2;g.player.reactor=30;g.drone_parts=1;assert(drones_equip(&g,0,1)&&drones_toggle(&g,0));ticks(GAME_HZ);g.enemy_weapon_secondary.charge_ticks=g.enemy_weapon_secondary.cooldown_ticks-1;game_step(&g);assert(effects(EFFECT_MISSILE,2)==1);int hull=g.player.hull;game_step(&g);int intercepted=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)intercepted+=g.effects[i].active&&g.effects[i].result==EFFECT_INTERCEPT;assert(intercepted==1&&g.player.hull==hull&&g.enemy_missiles==9);
 /* All departure/terminal paths reset secondary offense; new contacts remain
  * legacy until explicitly configured again. */
 setup();load(2,3);ticks(10);assert(game_accept_surrender(&g));assert(!g.enemy_weapon.charge_ticks&&!g.enemy_weapon_secondary.charge_ticks);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0)&&!g.enemy_weapon_count&&!g.enemy_weapon_secondary.enabled);
 setup();load(2,3);ticks(10);g.player.systems[SYS_ENGINES].power=2;g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;assert(game_depart(&g,0)&&!g.enemy_weapon_count&&!g.enemy_weapon_secondary.enabled);
 setup();load(2,3);ticks(10);assert(game_enemy_escape_start(&g,1,g.enemy.hull,g.enemy.hull));game_step(&g);assert(g.enemy_departed&&!g.enemy_weapon_secondary.charge_ticks&&!g.enemy_weapon_secondary.pending_shots);
 setup();load(2,3);g.enemy_weapon_secondary.pending_shots=1;g.player.hull=0;game_step(&g);assert(g.outcome==GAME_DEFEAT&&!g.enemy_weapon_secondary.pending_shots&&!g.enemy_weapon.pending_shots);for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g.effects[i].active||g.effects[i].side==0);
 puts("Enemy loadouts: atomic bounded catalog, real independent cooldowns, simultaneous volleys, power priority/damage/ions, pause/service, queued payload retention, saturation, finite source-Artemis ammo/Defense I and terminal/departure cleanup passed.");
}
