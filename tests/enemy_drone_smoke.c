#include "game.h"
#include "enemy_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void attack(Game*g,int room,int damage){
 memset(g->effects,0,sizeof g->effects);
 g->effects[0]=(GameEffect){.active=1,.age=0,.duration=1,.side=0,.room=room,.kind=EFFECT_LASER,.result=EFFECT_PENDING,.damage=damage,.system_damage=damage,.crew_damage=damage*5,.piercing=99};
 g->enemy.systems[SYS_ENGINES].power=0;game_step(g);
}
int main(void){
 Game g;game_init(&g);game_begin_encounter(&g,ENEMY_ASSAULT,0,0);
 const ImportedEnemy*data=&imported_enemies[1];
 assert(g.enemy.room_count==data->room_count&&g.enemy.max_hull==data->hull);
 for(int i=0;i<data->room_count;i++){
  const GameRoom*r=&g.enemy.rooms[i];const ImportedEnemyRoom*o=&data->rooms[i];
  assert(r->id==o->id&&r->x==o->x&&r->y==o->y&&r->w==o->w&&r->h==o->h);
 }
 assert(g.enemy.systems[SYS_SHIELDS].room==3&&g.enemy.systems[SYS_ENGINES].room==4&&g.enemy.systems[SYS_WEAPONS].room==1);
 assert(g.enemy_drone.room==0&&g.enemy_drone.power==2&&g.enemy_drone.health==2);
 assert(g.enemy_drone.damage==1&&g.enemy_drone.fire_chance==10&&g.enemy_drone.cooldown_ticks>0);
 g.enemy_drone.charge_ticks=g.enemy_drone.cooldown_ticks-1;g.paused=1;game_step(&g);assert(g.enemy_drone.charge_ticks==g.enemy_drone.cooldown_ticks-1);
 g.paused=0;game_step(&g);assert(g.enemy_drone.charge_ticks==0);
 int shots=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)if(g.effects[i].active){assert(g.effects[i].side==1&&g.effects[i].damage==1&&g.effects[i].fire_chance==10);shots++;}
 assert(shots==1);
 /* Dropping either power or health below the installed drone cost resets charge. */
 g.enemy_drone.charge_ticks=7;g.enemy_drone.power=1;game_step(&g);assert(!g.enemy_drone.charge_ticks);
 g.enemy_drone.power=2;g.enemy_drone.health=1;g.enemy_drone.charge_ticks=7;game_step(&g);assert(!g.enemy_drone.charge_ticks);
 g.enemy_drone.health=2;g.enemy_drone.power=2;attack(&g,g.enemy_drone.room,2);
 assert(!g.enemy_drone.health&&!g.enemy_drone.power&&!g.enemy_drone.charge_ticks);
 game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_drone.room<0&&!g.enemy_drone.capacity);
 for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g.effects[i].active);
 puts("Auto-Assault: imported geometry/systems, bounded drone launch, pause, power/damage disable and encounter reset passed");
}
