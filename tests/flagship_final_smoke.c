/* V44 flagship stage 3: Zoltan super shield, laser-barrage surge with shield
 * restores, teleporter blocking, and winnability of stages 2-3 for a strong build. */
#include "game.h"
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void stage(int phase){game_init(&g);g.flagship_authentic=1;assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,phase));g.player.systems[SYS_ENGINES].power=0;g.enemy.systems[SYS_ENGINES].power=0;g.enemy_pilot_health=0;}
static int land(int kind,const GameDamage*d,int piercing){assert(game_launch_effect(&g,0,g.enemy.rooms[1].id,kind,d,piercing));for(int t=0;t<12;t++){game_step(&g);}
 for(int i=0;i<GAME_MAX_EFFECTS;i++){if(g.effects[i].side==0&&g.effects[i].result!=EFFECT_PENDING){int r=g.effects[i].result;memset(&g.effects[i],0,sizeof g.effects[i]);return r;}}
 return -1;}
static int enemy_lasers(void){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].kind==EFFECT_LASER;return n;}
static int strong(unsigned seed,int phase,int authentic){
 game_init(&g);g.flagship_authentic=authentic;g.rng^=seed*2654435761u;assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,phase));
 for(int i=g.crew_count;i<6;i++){game_init_crew(&g,0,i,g.player.rooms[0].id);g.crew_count++;}
 game_equip(&g,0,0);game_equip(&g,1,1);game_equip(&g,2,1);game_equip(&g,3,3);
 g.player.reactor=18;g.player.systems[SYS_SHIELDS].capacity=g.player.systems[SYS_SHIELDS].health=6;
 g.player.systems[SYS_ENGINES].capacity=g.player.systems[SYS_ENGINES].health=4;g.player.systems[SYS_WEAPONS].capacity=g.player.systems[SYS_WEAPONS].health=8;g.missiles=14;
 for(int k=0;k<GAME_SYSTEM_COUNT;k++)while(game_adjust_power(&g,k,-1)){}
 game_adjust_power(&g,SYS_OXYGEN,1);while(game_adjust_power(&g,SYS_WEAPONS,1)){}while(game_adjust_power(&g,SYS_SHIELDS,1)){}while(game_adjust_power(&g,SYS_ENGINES,1)){}
 for(int t=0;t<300*GAME_HZ&&g.outcome==GAME_PLAYING;t++){
  int aim=-1,ready=1,hostile=-1;static const int order[]={1,0,2};
  for(int i=0;i<3&&aim<0;i++)for(int k=0;k<g.artillery_count;k++)if(g.artillery[k].weapon==order[i]&&g.artillery[k].health>0)aim=g.artillery[k].room;
  if(aim<0)aim=g.enemy.systems[SYS_SHIELDS].room;
  for(int w=0;w<g.weapon_count;w++)if(game_weapon_powered(&g,w)&&g.weapons[w].charge_ticks<g.weapons[w].cooldown_ticks)ready=0;
  if(ready)for(int w=0;w<g.weapon_count;w++)game_fire(&g,w,aim);
  for(int i=0;i<g.enemy_crew_count;i++)if(g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0){hostile=g.enemy_crew[i].room;break;}
  int repair=-1,healing=0,med=g.player.systems[SYS_MEDBAY].room,weaponroom=g.player.systems[SYS_WEAPONS].room,gunner=-1;
  /* V46: weapons fire only while manned, so one healthy crewmember stays a gunner. */
  for(int i=0;i<g.crew_count;i++)if(g.crew[i].health>=40&&g.crew[i].ship==0&&g.crew[i].destination==weaponroom){gunner=i;break;}
  if(gunner<0)for(int i=g.crew_count-1;i>=0;i--)if(g.crew[i].health>=40&&g.crew[i].ship==0){gunner=i;break;}
  for(int i=0;i<g.crew_count;i++){Crew*cr=&g.crew[i];if(cr->health<=0)continue;int target=-1;
   if(cr->health<40||(cr->destination==med&&cr->health<95)){target=med;healing=1;}else if(i==gunner)target=weaponroom;else if(hostile>=0)target=hostile;else if(repair<0)repair=i;
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
 /* Stage 3 starts with a full 12-point super shield; stages 1-2 have none. */
 stage(2);assert(g.super_shield==12&&g.super_shield_max==12&&game_enemy_super_shield(&g)==12);
 {Game h;game_init(&h);assert(game_begin_encounter(&h,ENEMY_FLAGSHIP,7,1)&&!h.super_shield_max);game_init(&h);assert(game_begin_encounter(&h,ENEMY_FLAGSHIP,7,2)&&h.super_shield_max==8);} /* DS tuning: 8 points. */
 /* Every damage type is absorbed point for point; nothing reaches the hull. */
 {GameDamage laser={1,1,5,0,0,0,0,0},missile={2,2,10,0,0,0,0,0};int hull=g.enemy.hull;
  assert(land(EFFECT_LASER,&laser,0)==EFFECT_SHIELD&&g.super_shield==11);
  assert(land(EFFECT_MISSILE,&missile,5)==EFFECT_SHIELD&&g.super_shield==9);
  assert(game_launch_ion(&g,0,g.enemy.rooms[1].id,1));for(int t=0;t<12;t++){game_step(&g);}assert(g.super_shield==8);
  assert(game_launch_bomb(&g,0,g.enemy.rooms[1].id,&(GameDamage){0,2,10,0,0,0,0,0}));for(int t=0;t<12;t++){game_step(&g);}assert(g.super_shield==7);
  {int x0=(2*g.enemy.rooms[1].x+1)*GAME_BEAM_UNITS/2,y0=(2*g.enemy.rooms[1].y+1)*GAME_BEAM_UNITS/2;assert(game_launch_beam(&g,0,x0,y0,x0+GAME_BEAM_UNITS,y0,&missile));for(int t=0;t<12;t++){game_step(&g);}assert(g.super_shield==5);}
  assert(g.enemy.hull==hull);
  /* Down to zero, normal shields take over. */
  g.super_shield=1;assert(land(EFFECT_MISSILE,&missile,5)==EFFECT_SHIELD&&!g.super_shield&&!game_enemy_super_shield(&g));
  g.enemy.shields=0;assert(land(EFFECT_MISSILE,&missile,5)==EFFECT_HIT&&g.enemy.hull<hull);}
 /* The teleporter cannot send crew in, or bring them back, through it. */
 stage(2);assert(boarding_install(&g,0));g.teleporter[0].power=1;g.teleporter[0].cooldown_ticks=0;
 for(int i=0;i<2;i++){g.crew[i].room=g.crew[i].destination=g.teleporter[0].room;}
 assert(!boarding_deploy(&g,3u,g.enemy.rooms[1].id)&&strstr(g.message,"Super shield"));
 g.super_shield=0;assert(boarding_deploy(&g,3u,g.enemy.rooms[1].id));g.super_shield=12;g.teleporter[0].cooldown_ticks=0;
 for(int i=0;i<2;i++){g.crew[i].room=g.crew[i].destination=g.enemy.rooms[1].id;}
 assert(!boarding_recall(&g,3u)&&strstr(g.message,"traps"));
 /* Surges: three seven-laser barrages, then the shield is restored. */
 stage(2);for(int i=0;i<g.artillery_count;i++)g.artillery[i].capacity=g.artillery[i].health=g.artillery[i].power=0;g.enemy.systems[SYS_SHIELDS].power=0;
 for(int k=0;k<3;k++){memset(g.effects,0,sizeof g.effects);g.surge_ticks=1;game_step(&g);assert(enemy_lasers()==7&&g.surge_count==k+1&&g.surge_ticks>=20*GAME_HZ-1&&g.surge_ticks<30*GAME_HZ);}
 g.super_shield=0;memset(g.effects,0,sizeof g.effects);g.surge_ticks=1;game_step(&g);assert(!enemy_lasers()&&g.super_shield==12&&g.surge_count==4);
 memset(g.effects,0,sizeof g.effects);g.surge_ticks=1;game_step(&g);assert(enemy_lasers()==7);
 /* Winnability with the strong attainable build: the default DS tuning must leave
  * every stage winnable; authentic normal numbers are reported for comparison. */
 {int w[2][3]={{0}};for(int mode=0;mode<2;mode++)for(int ph=0;ph<3;ph++)for(unsigned s=1;s<=10;s++)w[mode][ph]+=strong(s,ph,mode);
  fprintf(stderr,"Strong DS build, DS tuning: stages %d/%d/%d of 10; authentic normal: %d/%d/%d of 10.\n",w[0][0],w[0][1],w[0][2],w[1][0],w[1][1],w[1][2]);
  assert(w[0][0]>=8&&w[0][1]>=7&&w[0][2]>=6);}
 puts("PASS: V44 super shield absorption, teleport blocking, laser surges and shield restores, DS-tuned winnability of all stages.");return 0;
}
