/* V38 enemy weapon AI: FTL 1.6.14 CombatAI normal-difficulty targeting.
 * Distribution bounds are statistical over a fixed seed sequence. */
#include "game.h"
#include "beam.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static int system_of(int room){for(int s=0;s<GAME_SYSTEM_COUNT;s++)if(g.player.systems[s].capacity>0&&g.player.systems[s].room==room)return s;return -1;}
static int room_known(int room){for(int i=0;i<g.player.room_count;i++)if(g.player.rooms[i].id==room)return 1;return 0;}
int main(void){
 int hits[GAME_MAX_ROOMS]={0},n=0,total=20000;
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0));
 int rooms=g.player.room_count;assert(rooms>=5);
 /* Single shots: 67% random room, plus 33%/5 per listed system. The absent
  * drones entry (starting Kestrel) falls back to a random room too. */
 for(int i=0;i<total;i++){int r;assert(game_enemy_plan_volley(&g,&r,1)==1&&room_known(r));for(int k=0;k<g.player.room_count;k++)if(g.player.rooms[k].id==r)hits[k]++;}
 for(int k=0;k<g.player.room_count;k++){int s=system_of(g.player.rooms[k].id);double expect=total*((0.67+0.33/5)/rooms+((s==SYS_SHIELDS||s==SYS_ENGINES||s==SYS_WEAPONS||s==SYS_OXYGEN)?0.33/5:0.0));
  assert(hits[k]>expect*0.85&&hits[k]<expect*1.15);n+=hits[k];}
 assert(n==total);
 /* Medbay is never a priority target: it only receives the random-room share. */
 {int k;for(k=0;k<g.player.room_count&&g.player.rooms[k].id!=g.player.systems[SYS_MEDBAY].room;k++);assert(k<g.player.room_count);assert(hits[k]<total*((0.67+0.33/5)/rooms)*1.15);}
 /* Drones join the list once installed. */
 if(!g.drones.installed){int droom=-1;for(int k=0;k<g.player.room_count;k++)if(system_of(g.player.rooms[k].id)<0){droom=g.player.rooms[k].id;break;}
  if(droom>=0){int c=0;g.drones.installed=1;g.drones.room=droom;for(int i=0;i<total;i++){int r;game_enemy_plan_volley(&g,&r,1);c+=r==droom;}assert(c>total*(0.67/rooms+0.066)*0.85);g.drones.installed=0;}}
 /* Volleys target distinct rooms; requests above room count are capped. */
 for(int i=0;i<2000;i++){int r[GAME_MAX_VOLLEY];int c=game_enemy_plan_volley(&g,r,3);assert(c==3);assert(r[0]!=r[1]&&r[1]!=r[2]&&r[0]!=r[2]);}
 {Game small=g;small.player.room_count=2;int r[GAME_MAX_VOLLEY];assert(game_enemy_plan_volley(&small,r,3)==2&&r[0]!=r[1]);assert(!game_enemy_plan_volley(&small,r,0));}
 /* Beam: two distinct room centres, full weapon length unless the hull ends. */
 for(int i=0;i<500;i++){int x0,y0,x1,y1,len=80*GAME_BEAM_UNITS/35;assert(game_enemy_plan_beam(&g,len,&x0,&y0,&x1,&y1));assert(beam_valid_line(&g.player,x0,y0,x1,y1));long long d=(long long)(x1-x0)*(x1-x0)+(long long)(y1-y0)*(y1-y0);assert(d>0&&d<=(long long)(len+1)*(len+1));}
 /* Integrated: legacy single-weapon fire plans a full distinct volley. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0));g.player.systems[SYS_ENGINES].power=0;
 {WeaponState*w=&g.enemy_weapon;w->shots=3;w->beam_length=0;w->ion_damage=0;w->bomb=0;g.enemy_fire_ticks=w->cooldown_ticks-1;
  assert(!g.enemy_weapon_count);
  /* The first round launches in the same step the volley is planned. */
  for(int t=0;t<3&&!w->queued_target_count;t++)game_step(&g);
  assert(w->queued_target_count==3&&g.enemy_pending_shots==2);int plan[3]={w->queued_targets[0],w->queued_targets[1],w->queued_targets[2]};
  assert(room_known(plan[0])&&room_known(plan[1])&&room_known(plan[2])&&plan[0]!=plan[1]&&plan[1]!=plan[2]&&plan[0]!=plan[2]&&w->target_room==plan[0]);
  int fired=1;for(int t=0;t<20&&fired<3;t++){int before=g.enemy_pending_shots;game_step(&g);if(g.enemy_pending_shots<before){assert(w->target_room==plan[fired]);fired++;}}assert(fired==3&&!g.enemy_pending_shots);}
 puts("enemy weapon AI smoke passed");return 0;
}
