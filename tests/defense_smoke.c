#include "drones.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void ticks(int n){while(n--)game_step(&g);}
static void setup(int count){int i;game_init(&g);g.enemy_weapon.cooldown_ticks=999999;g.player.systems[SYS_ENGINES].power=0;g.player.systems[SYS_SHIELDS].power=0;g.player.shields=0;
 assert(drones_install(&g));g.drones.capacity=g.drones.health=g.drones.power=2*count;g.player.reactor=30;g.drone_parts=4;
 for(i=0;i<count;i++){assert(drones_equip(&g,i,1));assert(drones_toggle(&g,i));}
}
static int launch(int side,int kind){int i;GameDamage damage={2,2,10,0,0,0,0,0};int room=side?g.player.systems[SYS_ENGINES].room:g.enemy.systems[SYS_ENGINES].room;
 for(i=0;i<GAME_MAX_EFFECTS&&g.effects[i].active;i++){}
 assert(i<GAME_MAX_EFFECTS);assert(game_launch_effect(&g,side,room,kind,&damage,99));return i;
}
static int intercepted(void){int i,n=0;for(i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].result==EFFECT_INTERCEPT;return n;}
int main(void){int a,b,c,i,hull,health,parts,charge;Crew crew[GAME_MAX_CREW];
 assert(drones_is_defense(1)&&!drones_is_defense(0)&&!drones_is_defense(-1));assert(drones_cooldown(1)==GAME_HZ&&drones_cooldown(0)==10*GAME_HZ&&drones_cooldown(2)==6*GAME_HZ&&!drones_cooldown(5)&&!drones_cooldown(GAME_DRONE_CATALOG_COUNT));assert(drones_power(1)==2&&drones_price(1)==50&&!strcmp(drones_name(1),"Defense Drone Mark I"));
 setup(1);assert(!g.drones.slots[0].charge_ticks);ticks(GAME_HZ-1);assert(g.drones.slots[0].charge_ticks==GAME_HZ-1);a=launch(1,EFFECT_MISSILE);hull=g.player.hull;health=g.player.systems[SYS_ENGINES].health;memcpy(crew,g.crew,sizeof crew);parts=g.drone_parts;game_step(&g);assert(g.effects[a].result==EFFECT_INTERCEPT&&g.effects[a].age>=g.effects[a].duration&&!g.drones.slots[0].charge_ticks);assert(g.player.hull==hull&&g.player.systems[SYS_ENGINES].health==health&&!g.player.shields);for(i=0;i<g.crew_count;i++)assert(g.crew[i].health==crew[i].health);ticks(GAME_EFFECT_IMPACT_TICKS);assert(!g.effects[a].active&&g.player.hull==hull&&g.drone_parts==parts);
 /* Saturation: one ready unit chooses shortest flight, then lowest slot. */
 setup(1);ticks(GAME_HZ);a=launch(1,EFFECT_MISSILE);b=launch(1,EFFECT_MISSILE);c=launch(1,EFFECT_MISSILE);g.effects[b].age=g.effects[c].age=5;hull=g.player.hull;game_step(&g);assert(g.effects[b].result==EFFECT_INTERCEPT&&g.effects[a].result==EFFECT_PENDING&&g.effects[c].result==EFFECT_PENDING);ticks(12);assert(g.player.hull==hull-4);ticks(GAME_HZ);a=launch(1,EFFECT_MISSILE);game_step(&g);assert(g.effects[a].result==EFFECT_INTERCEPT);
 /* Two drones catch distinct missiles; an already-resolved shot is excluded. */
 setup(2);ticks(GAME_HZ);a=launch(1,EFFECT_MISSILE);b=launch(1,EFFECT_MISSILE);c=launch(1,EFFECT_MISSILE);game_step(&g);assert(intercepted()==2&&g.effects[a].result==EFFECT_INTERCEPT&&g.effects[b].result==EFFECT_INTERCEPT&&g.effects[c].result==EFFECT_PENDING);hull=g.player.hull;ticks(12);assert(g.player.hull==hull-2);
 /* Friendly missile, hostile laser/ion/bomb/beam are never intercepted. */
 setup(1);ticks(GAME_HZ);a=launch(0,EFFECT_MISSILE);b=launch(1,EFFECT_LASER);assert(game_launch_ion(&g,1,g.player.systems[SYS_ENGINES].room,1));{GameDamage damage={0,1,0,0,0,0,0,0};assert(game_launch_bomb(&g,1,g.player.systems[SYS_MEDBAY].room,&damage));}
 /* Beam path geometry has separate tests; exercise defense filtering in pool. */
 g.effects[4].active=1;g.effects[4].duration=12;g.effects[4].side=1;g.effects[4].kind=EFFECT_BEAM;
 game_step(&g);assert(!intercepted()&&g.effects[a].result==EFFECT_PENDING&&g.effects[b].result==EFFECT_PENDING&&g.drones.slots[0].charge_ticks==GAME_HZ);
 /* Recharge and missile age freeze while paused. Suspension/ion/power freeze
    readiness; they do not grant free redeployment or intercept on disabled units. */
 setup(1);ticks(5);charge=g.drones.slots[0].charge_ticks;a=launch(1,EFFECT_MISSILE);g.paused=1;ticks(50);assert(g.drones.slots[0].charge_ticks==charge&&!g.effects[a].age);g.paused=0;assert(drones_toggle(&g,0));parts=g.drone_parts;ticks(2);assert(g.drones.slots[0].charge_ticks==charge);assert(drones_toggle(&g,0)&&g.drone_parts==parts);g.drones.ion_bars=2;g.drones.ion_ticks=100;ticks(2);assert(g.drones.slots[0].charge_ticks==charge);g.drones.ion_bars=g.drones.ion_ticks=0;g.drones.power=0;ticks(2);assert(g.drones.slots[0].charge_ticks==charge);g.drones.power=2;g.drones.health=1;ticks(2);assert(g.drones.slots[0].charge_ticks==charge);g.drones.health=2;g.drones.power=2;game_step(&g);assert(g.drones.slots[0].charge_ticks==charge+1);
 /* Interception needs no free effect slot, and disabled ready drones cannot
    protect the hull. A resolved impact is never retroactively intercepted. */
 setup(1);ticks(GAME_HZ);for(i=0;i<GAME_MAX_EFFECTS;i++)launch(1,EFFECT_MISSILE);game_step(&g);assert(intercepted()==1);for(i=0;i<GAME_MAX_EFFECTS;i++)assert(g.effects[i].active);
 setup(1);ticks(GAME_HZ);assert(drones_toggle(&g,0));a=launch(1,EFFECT_MISSILE);hull=g.player.hull;ticks(12);assert(g.player.hull==hull-2&&g.effects[a].result==EFFECT_HIT);assert(drones_toggle(&g,0));game_step(&g);assert(g.effects[a].result==EFFECT_HIT&&!intercepted());
 /* Terminal guard prevents late interception even if called directly. */
 setup(1);ticks(GAME_HZ);a=launch(1,EFFECT_MISSILE);g.outcome=GAME_VICTORY;drones_intercept(&g);assert(g.effects[a].result==EFFECT_PENDING);g.outcome=GAME_PLAYING;g.player.hull=0;drones_intercept(&g);assert(g.effects[a].result==EFFECT_PENDING);g.player.hull=30;g.enemy.hull=0;drones_intercept(&g);assert(g.effects[a].result==EFFECT_PENDING);
 puts("PASS: Defense I interception before damage, initial charge, cooldown saturation, deterministic priority, double defense, exclusions and pause/power/ion/terminal guards.");return 0;
}
