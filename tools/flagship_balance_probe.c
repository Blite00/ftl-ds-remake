/* Flagship balance probe: seeded stage-by-stage win rates for a strong and a
 * typical DS build under authentic normal numbers and the default DS tuning.
 * Build: gcc -O2 -Isource -Igenerated tools/flagship_balance_probe.c source/{game,fire,boarding,breach,subsystems,species,beam,drones}.c */
#include "game.h"
#include "species.h"
#include <stdio.h>
#include <string.h>
static Game g;
typedef struct { const char*name;int layers; } Scale; /* layers: flagship_authentic flag */
static int fight(unsigned seed,int phase,int strong,const Scale*sc){
 game_init(&g);g.flagship_authentic=sc->layers;g.rng^=seed*2654435761u;game_begin_encounter(&g,ENEMY_FLAGSHIP,7,phase);
 if(strong){for(int i=g.crew_count;i<6;i++){game_init_crew(&g,0,i,g.player.rooms[0].id);g.crew_count++;}
  game_equip(&g,0,0);game_equip(&g,1,1);game_equip(&g,2,1);game_equip(&g,3,3);
  g.player.reactor=18;g.player.systems[SYS_SHIELDS].capacity=g.player.systems[SYS_SHIELDS].health=6;
  g.player.systems[SYS_ENGINES].capacity=g.player.systems[SYS_ENGINES].health=4;g.player.systems[SYS_WEAPONS].capacity=g.player.systems[SYS_WEAPONS].health=8;g.missiles=14;}
 else{game_equip(&g,0,0);game_equip(&g,1,1);game_equip(&g,2,1);g.player.reactor=12;g.player.systems[SYS_SHIELDS].capacity=g.player.systems[SYS_SHIELDS].health=4;
  g.player.systems[SYS_WEAPONS].capacity=g.player.systems[SYS_WEAPONS].health=5;g.missiles=10;}
 for(int k=0;k<GAME_SYSTEM_COUNT;k++)while(game_adjust_power(&g,k,-1)){}
 game_adjust_power(&g,SYS_OXYGEN,1);while(game_adjust_power(&g,SYS_WEAPONS,1)){}while(game_adjust_power(&g,SYS_SHIELDS,1)){}while(game_adjust_power(&g,SYS_ENGINES,1)){}
 for(int t=0;t<300*GAME_HZ&&g.outcome==GAME_PLAYING;t++){
  int aim=-1,ready=1,hostile=-1;static const int order[]={1,0,2};
  for(int i=0;i<3&&aim<0;i++)for(int k=0;k<g.artillery_count;k++)if(g.artillery[k].weapon==order[i]&&g.artillery[k].health>0)aim=g.artillery[k].room;
  if(aim<0)aim=g.enemy.systems[SYS_SHIELDS].room;
  for(int w=0;w<g.weapon_count;w++)if(game_weapon_powered(&g,w)&&g.weapons[w].charge_ticks<g.weapons[w].cooldown_ticks)ready=0;
  if(ready)for(int w=0;w<g.weapon_count;w++)game_fire(&g,w,aim);
  for(int i=0;i<g.enemy_crew_count;i++)if(g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0){hostile=g.enemy_crew[i].room;break;}
  int repair=-1,healing=0,med=g.player.systems[SYS_MEDBAY].room;
  for(int i=0;i<g.crew_count;i++){Crew*cr=&g.crew[i];if(cr->health<=0)continue;int target=-1;
   if(cr->health<40||(cr->destination==med&&cr->health<95)){target=med;healing=1;}else if(hostile>=0)target=hostile;else if(repair<0)repair=i;
   if(target>=0&&cr->destination!=target)game_order_crew(&g,i,target);}
  if(healing&&!g.player.systems[SYS_MEDBAY].power){if(game_used_power(&g)>=g.player.reactor)game_adjust_power(&g,SYS_ENGINES,-1);if(game_used_power(&g)>=g.player.reactor)game_adjust_power(&g,SYS_WEAPONS,-1);game_adjust_power(&g,SYS_MEDBAY,1);}
  if(!healing&&g.player.systems[SYS_MEDBAY].power){game_adjust_power(&g,SYS_MEDBAY,-1);while(game_adjust_power(&g,SYS_WEAPONS,1)){}while(game_adjust_power(&g,SYS_ENGINES,1)){}}
  if(repair>=0){int target=-1;const GameSystemState*sy=g.player.systems;
   if(sy[SYS_OXYGEN].health<sy[SYS_OXYGEN].capacity)target=sy[SYS_OXYGEN].room;else if(sy[SYS_SHIELDS].health<sy[SYS_SHIELDS].capacity)target=sy[SYS_SHIELDS].room;else if(sy[SYS_WEAPONS].health<sy[SYS_WEAPONS].capacity)target=sy[SYS_WEAPONS].room;
   if(target>=0&&g.crew[repair].destination!=target)game_order_crew(&g,repair,target);}
  if(g.player.systems[SYS_OXYGEN].health>0&&!g.player.systems[SYS_OXYGEN].power)game_adjust_power(&g,SYS_OXYGEN,1);
  while(game_adjust_power(&g,SYS_SHIELDS,1)){}
  game_step(&g);
 }
 return g.outcome==GAME_VICTORY;
}
int main(void){
 Scale sc[]={{"authentic normal",1},{"DS tuning (default)",0}};
 for(unsigned v=0;v<sizeof sc/sizeof sc[0];v++){
  for(int strong=1;strong>=0;strong--){int w[3]={0,0,0};for(int ph=0;ph<3;ph++)for(unsigned s=1;s<=10;s++)w[ph]+=fight(s,ph,strong,&sc[v]);
   printf("%-22s %-7s stage1 %2d/10 stage2 %2d/10 stage3 %2d/10\n",sc[v].name,strong?"strong":"typical",w[0],w[1],w[2]);fflush(stdout);}
 }
 return 0;}
