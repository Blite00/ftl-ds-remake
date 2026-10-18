/* V43 flagship stage 2: drone system (defense, combat, beam, boss boarder),
 * parts, power-limited activation, interception both ways and the drone surge. */
#include "game.h"
#include "drones.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void stage2(void){game_init(&g);g.flagship_authentic=1;assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,1));g.player.systems[SYS_ENGINES].power=0;}
static int count(int side,int kind){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==side&&g.effects[i].kind==kind;return n;}
static int intruders(void){int n=0;for(int i=0;i<GAME_MAX_ENEMY_CREW&&i<g.enemy_crew_count;i++)n+=g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0;return n;}
static void steps(int n){while(n--&&g.outcome==GAME_PLAYING)game_step(&g);}
int main(void){
 stage2();
 assert(g.enemy_drone.room==12&&g.enemy_drone.capacity==8&&g.flagship_drone_count==4&&g.flagship_drone_parts==10);
 assert(g.flagship_drones[0].kind==FDRONE_DEFENSE&&g.flagship_drones[1].kind==FDRONE_COMBAT&&g.flagship_drones[2].kind==FDRONE_BEAM&&g.flagship_drones[3].kind==FDRONE_BOARDER);
 assert(g.surge_ticks>=20*GAME_HZ&&g.surge_ticks<30*GAME_HZ&&g.surge_beams>=0&&g.surge_beams<=GAME_SURGE_DRONES);
 assert(game_flagship_drones_active(&g)==4);
 /* Two bars per drone, powered in blueprint order. */
 g.enemy_drone.health=4;g.enemy_drone.power=4;assert(game_flagship_drones_active(&g)==2);
 g.enemy_drone.ion_bars=1;assert(game_flagship_drones_active(&g)==1);g.enemy_drone.ion_bars=0;g.enemy_drone.health=g.enemy_drone.power=8;
 /* Deployment spends one part per drone. */
 game_step(&g);for(int i=0;i<4;i++)assert(g.flagship_drones[i].deployed);assert(g.flagship_drone_parts==6);
 /* Defense drone shoots down a player missile within about a second. */
 steps(GAME_HZ+1); /* The defense drone arms one second after deployment. */
 {GameDamage hit={2,2,10,0,0,0,0,0};assert(game_launch_effect(&g,0,g.enemy.rooms[1].id,EFFECT_MISSILE,&hit,5));int got=0;
  for(int t=0;t<12&&!got;t++){game_step(&g);for(int k=0;k<GAME_MAX_EFFECTS;k++)got|=g.effects[k].side==0&&g.effects[k].kind==EFFECT_MISSILE&&g.effects[k].result==EFFECT_INTERCEPT;}assert(got);}
 /* Boss boarding drone: 3 s flight, then a breach and a 150 HP intruder. */
 stage2();{int before=intruders();steps(3*GAME_HZ+2);assert(intruders()==before+1);
  int i;for(i=0;i<g.enemy_crew_count;i++)if(g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0)break;
  assert(g.enemy_crew[i].health==150&&g.enemy_crew[i].species==SPEC_ROCK&&!strcmp(g.enemy_crew[i].name,"Drone"));
  int breached=0;for(int r=0;r<g.player.room_count;r++)breached+=g.player.rooms[r].breaches>0;assert(breached>=1);
  /* The landed boarder keeps its slot until it dies; then a part buys a new one. */
  assert(g.flagship_drones[3].deployed&&g.flagship_drones[3].crew==i&&g.flagship_drone_parts==6);steps(GAME_HZ);assert(intruders()==before+1&&g.flagship_drone_parts==6);
  g.enemy_crew[i].health=0;g.enemy_crew[i].death_processed=1;steps(2);assert(!g.flagship_drones[3].deployed);steps(10*GAME_HZ);assert(g.flagship_drones[3].deployed&&g.flagship_drones[3].flight>0&&g.flagship_drone_parts==5);
  /* Losing drone power shuts a landed boarder down. */
  steps(3*GAME_HZ+2);{int j=g.flagship_drones[3].crew;assert(j>=0&&g.enemy_crew[j].health>0);g.enemy_drone.health=g.enemy_drone.power=6;steps(1);assert(g.enemy_crew[j].health==0);}}
 /* Destroying the stage powers down landed boarders, so they cannot block victory. */
 stage2();steps(3*GAME_HZ+2);{int j=g.flagship_drones[3].crew;assert(j>=0&&g.enemy_crew[j].health>0);g.enemy.hull=0;steps(GAME_HZ);assert(g.enemy_crew[j].health==0&&g.outcome==GAME_VICTORY);}
 /* The player's Defense I destroys a boarding drone in flight. */
 stage2();g.drones.installed=1;g.drones.capacity=g.drones.health=g.drones.power=2;g.drones.slot_count=1;g.drones.slots[0].catalog_id=1;g.drones.slots[0].enabled=1;g.drones.slots[0].deployed=1;g.drones.slots[0].charge_ticks=GAME_HZ;
 game_step(&g);assert(g.flagship_drones[3].deployed&&g.flagship_drones[3].flight>0);
 {int before=intruders();steps(3*GAME_HZ+2);assert(intruders()==before);}
 /* Combat and beam drones fire on their 10 s interval. */
 stage2();g.surge_ticks=0;{int lasers=0,beams=0;for(int t=0;t<11*GAME_HZ;t++){game_step(&g);lasers+=count(1,EFFECT_LASER)>0;beams+=count(1,EFFECT_BEAM)>0;}assert(lasers&&beams);}
 /* Depowered drones stop firing. */
 stage2();g.surge_ticks=0;g.enemy_drone.health=0;g.enemy_drone.power=0;for(int i=0;i<GAME_SURGE_DRONES;i++)g.surge[i].shots=0;
 {int shots=0;for(int t=0;t<12*GAME_HZ;t++){game_step(&g);shots+=count(1,EFFECT_BEAM);}assert(!shots);}
 /* Drone surge: six drones, two shots each, a 5 s warning beforehand, then a new 20-30 s timer. */
 stage2();g.enemy_drone.health=g.enemy_drone.power=0;g.artillery_count=0;g.surge_ticks=5*GAME_HZ;assert(game_surge_warning(&g)==5);
 {int launched=0;while(g.surge_ticks>1){game_step(&g);}game_step(&g);
  int armed=0;for(int i=0;i<GAME_SURGE_DRONES;i++){armed+=g.surge[i].shots==2;}assert(armed==GAME_SURGE_DRONES&&g.surge_ticks>=20*GAME_HZ-1&&!game_surge_warning(&g));
  for(int t=0;t<10*GAME_HZ;t++){int before=0;for(int i=0;i<GAME_SURGE_DRONES;i++)before+=g.surge[i].shots;game_step(&g);int after=0;for(int i=0;i<GAME_SURGE_DRONES;i++)after+=g.surge[i].shots;launched+=before-after;}
  assert(launched==2*GAME_SURGE_DRONES);}
 /* No drones or surge in stage 1. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,0));assert(!g.flagship_drone_count&&!g.surge_ticks&&!g.enemy_drone.capacity);
 puts("PASS: V43 flagship drones (order, parts, power), defense interception both ways, boss boarder breach/intruder, combat/beam fire and drone surge.");return 0;
}
