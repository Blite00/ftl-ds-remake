/* V68 shot paths: player shots leave from the layout's original weapon mount on
 * the hull and travel right toward the enemy; enemy shots leave from the enemy
 * hull's mounts toward the player. A hit ends inside the targeted room, a miss
 * outside every room, a shield block on the shield ellipse, an interception in
 * space. Checked for all 18 player layouts, every imported enemy hull and the
 * three flagship stages, in the overview and both schematic views. */
#include "shot_path.h"
#include "player_ships.h"
#include "enemy_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static Game g;
static int checks,mounted,fallback,aft;
static int inside(int x,int y,int rx,int ry,int rw,int rh){return x>=rx&&x<rx+rw&&y>=ry&&y<ry+rh;}
static long long ef(int x,int y,int cx,int cy,int rx,int ry){long long dx=x-cx,dy=y-cy,a=rx,b=ry;return dx*dx*b*b+dy*dy*a*a-a*a*b*b;}
static int near_ellipse(int x,int y,int cx,int cy,int rx,int ry){int in=0,out=0;
 for(int j=-2;j<=2;j++)for(int i=-2;i<=2;i++){long long f=ef(x+i,y+j,cx,cy,rx,ry);if(f<=0)in=1;if(f>=0)out=1;}
 return in&&out;}
static const GameRoom*room_of(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
static void shot(int side,int room,int mount,int age,int result){memset(g.effects,0,sizeof g.effects);
 g.effects[1]=(GameEffect){.active=1,.side=side,.room=room,.kind=EFFECT_LASER,.age=age,.duration=12,.result=result,.mount=mount};}
static int outside_rooms(int side,int view,int x,int y){const Ship*s=side?&g.enemy:&g.player;
 for(int i=0;i<s->room_count;i++){int rx,ry,rw,rh;shot_room_rect(&g,side,view,&s->rooms[i],&rx,&ry,&rw,&rh);if(inside(x,y,rx,ry,rw,rh))return 0;}
 return 1;}
/* Weapons-room centre (x) of a side in a view. */
static int weapons_centre(int side,int view){const Ship*s=side?&g.enemy:&g.player;const GameRoom*r=room_of(s,s->systems[SYS_WEAPONS].room);int x,y,w,h;
 if(!r||s->systems[SYS_WEAPONS].capacity<=0){return -1000;}shot_room_rect(&g,side,view,r,&x,&y,&w,&h);return x+w/2;}
static void origin_ok(const char*name,int shooter,int view,const ShotPath*p){int hx,hy,hw,hh;shot_hull_rect(&g,shooter,view,&hx,&hy,&hw,&hh);
 if(!inside(p->x0,p->y0,hx-1,hy-1,hw+2,hh+2)){fprintf(stderr,"%s: origin %d,%d off hull %d,%d %dx%d (view %d)\n",name,p->x0,p->y0,hx,hy,hw,hh,view);assert(0);}
 if(p->mounted)mounted++;else fallback++;
 if(!shooter){ /* The player faces right: original mounts sit in the forward two thirds of the hull; the fallback is the forward edge of the weapons room. */
  if(p->mounted&&p->x0-hx<hw/3){fprintf(stderr,"%s: origin %d (mounted %d) in aft third of hull %d+%d view %d\n",name,p->x0,p->mounted,hx,hw,view);assert(0);}
  if(!p->mounted)assert(p->x0>=weapons_centre(0,view));else if(p->x0<weapons_centre(0,view))aft++;}
 else if(!p->mounted){int c=weapons_centre(1,view);assert(c==-1000||p->x0<=c);}
}
/* One shooter slot (mount = slot + 1, 0 = no mount) against every target room in all three views. */
static void check(const char*name,int side,int mount){
 int tside=side?0:1,shooter=!tside,dir=side?-1:1;const Ship*target=tside?&g.enemy:&g.player;ShotPath p;
 int views[3]={SHOT_VIEW_OVERVIEW,shooter?SHOT_VIEW_ENEMY:SHOT_VIEW_PLAYER,tside?SHOT_VIEW_ENEMY:SHOT_VIEW_PLAYER};
 /* Outgoing in the shooter's schematic: from the mount straight out toward the target. */
 shot(side,target->rooms[0].id,mount,6,EFFECT_PENDING);assert(shot_path(&g,1,views[1],&p)&&p.terminal==SHOT_EXIT);
 origin_ok(name,shooter,views[1],&p);assert((p.gx-p.x0)*dir>0&&p.gy==p.y0);
 {int x,y,dx,dy;shot_point(&p,6,12,&x,&y);shot_heading(&p,6,12,8,&dx,&dy);assert(dx*dir>0&&(x-p.x0)*dir>0);}
 for(int k=0;k<target->room_count;k++){const GameRoom*r=&target->rooms[k];
  for(int v=0;v<2;v++){int view=v?views[2]:views[0],rx,ry,rw,rh,ex,ey,erx,ery;
   shot_room_rect(&g,tside,view,r,&rx,&ry,&rw,&rh);shot_shield(&g,tside,view,&ex,&ey,&erx,&ery);
   shot(side,r->id,mount,6,EFFECT_PENDING);assert(shot_path(&g,1,view,&p)&&p.terminal==SHOT_FLIGHT);
   if(view==SHOT_VIEW_OVERVIEW)origin_ok(name,shooter,view,&p);else assert(p.x0==(dir>0?-8:264));
   assert((p.gx-p.x0)*dir>0&&(p.cx-p.x0)*dir>0); /* Toward the target. */
   {int dx,dy;shot_heading(&p,6,12,8,&dx,&dy);assert(dx*dir>0);}
   shot(side,r->id,mount,12,EFFECT_HIT);assert(shot_path(&g,1,view,&p)&&p.terminal==SHOT_ROOM);
   if(!inside(p.x1,p.y1,rx,ry,rw,rh)){fprintf(stderr,"%s: hit %d,%d outside room %d %d,%d %dx%d\n",name,p.x1,p.y1,r->id,rx,ry,rw,rh);assert(0);}
   {int x,y;shot_point(&p,12+GAME_EFFECT_IMPACT_TICKS,12,&x,&y);assert(x==p.x1&&y==p.y1);}
   shot(side,r->id,mount,12,EFFECT_MISS);assert(shot_path(&g,1,view,&p)&&p.terminal==SHOT_MISS&&outside_rooms(tside,view,p.x1,p.y1));
   {int x,y;shot_point(&p,12+GAME_EFFECT_IMPACT_TICKS,12,&x,&y);assert(outside_rooms(tside,view,x,y));}
   shot(side,r->id,mount,12,EFFECT_SHIELD);assert(shot_path(&g,1,view,&p)&&p.terminal==SHOT_SHIELD);
   if(!near_ellipse(p.x1,p.y1,ex,ey,erx,ery)){fprintf(stderr,"%s: shield stop %d,%d off ellipse %d,%d %dx%d\n",name,p.x1,p.y1,ex,ey,erx,ery);assert(0);}
   {int x,y;shot_point(&p,11,12,&x,&y);assert(abs(x-p.x1)<=(abs(p.x1-p.x0)+11)/12+1);} /* Arrives at the bubble as it resolves. */
   shot(side,r->id,mount,12,EFFECT_INTERCEPT);assert(shot_path(&g,1,view,&p)&&p.terminal==SHOT_SPACE&&outside_rooms(tside,view,p.x1,p.y1));
   checks++;
  }
 }
}
int main(void){
 /* Beams and bombs have no projectile path; a missing room has no end point. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));ShotPath p;
 shot(0,g.enemy.rooms[0].id,1,6,EFFECT_PENDING);g.effects[1].kind=EFFECT_BOMB;assert(!shot_path(&g,1,SHOT_VIEW_OVERVIEW,&p));
 g.effects[1].kind=EFFECT_BEAM;assert(!shot_path(&g,1,SHOT_VIEW_OVERVIEW,&p));g.effects[1].kind=EFFECT_LASER;g.effects[1].room=99;assert(!shot_path(&g,1,SHOT_VIEW_OVERVIEW,&p));
 g.effects[1].active=0;assert(!shot_path(&g,1,SHOT_VIEW_OVERVIEW,&p));
 /* Every player layout (Type A and B), each weapon slot, plus a mountless shot (drones). */
 for(int layout=0;layout<PLAYER_SHIP_COUNT;layout++){char name[48];
  game_init(&g);assert(game_init_ship(&g,layout)&&g.player_ship==layout);g.enemy_ship=4;assert(game_begin_encounter(&g,ENEMY_SHIP,1,0));
  assert(player_ship_hulls[layout].mount_count>=player_ships[layout].weapon_slots);
  for(int slot=0;slot<=player_ships[layout].weapon_slots;slot++){snprintf(name,sizeof name,"%s slot %d",player_ships[layout].blueprint,slot);check(name,0,slot<player_ships[layout].weapon_slots?slot+1:0);}
  /* Slot 0 starts at its imported mount in the player schematic. */
  {int x,y,ox,oy,t;const PlayerShipHull*h=&player_ship_hulls[layout];assert(shot_mount(&g,0,0,SHOT_VIEW_PLAYER,&x,&y));shot_geometry(&g.player,0,&ox,&oy,&t);
   assert(x==ox+(h->hull_x+h->mounts[0][0])*t/35&&y==oy+(h->hull_y+h->mounts[0][1])*t/35);}
  check(player_ships[layout].blueprint,1,1);
 }
 /* Every imported enemy hull (legacy kinds and ENEMY_SHIP), both directions. */
 static const int kinds[]={ENEMY_SCOUT,ENEMY_ASSAULT,ENEMY_REBEL,ENEMY_REBEL_FAT};
 for(int i=0;i<IMPORTED_ENEMY_COUNT;i++){char name[48];game_init(&g);
  if(i<4)assert(game_begin_encounter(&g,kinds[i],1,0));else{g.enemy_ship=i;assert(game_begin_encounter(&g,ENEMY_SHIP,1,0));}
  assert(game_enemy_layout_index(&g)==i);
  for(int slot=0;slot<=2;slot++){snprintf(name,sizeof name,"%s enemy slot %d",imported_enemies[i].blueprint,slot);check(name,1,slot?slot:0);}
  check(imported_enemies[i].blueprint,0,1);
  {int x,y;assert(shot_mount(&g,1,0,SHOT_VIEW_OVERVIEW,&x,&y)==1);}
 }
 /* Type B against a Rebel fighter, and the flagship stages with their artillery mounts (4+i). */
 for(int stage=0;stage<3;stage++){char name[32];game_init(&g);assert(game_init_ship(&g,1));assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,stage)&&game_is_flagship(&g)&&g.flagship_stage==stage);
  snprintf(name,sizeof name,"flagship %d",stage+1);check(name,1,1);for(int a=0;a<g.artillery_count;a++){check(name,1,5+a);int x,y;assert(shot_mount(&g,1,4+a,SHOT_VIEW_ENEMY,&x,&y)==1);}
  check(name,0,2);}
 /* The engine records the firing slot: a Kestrel Burst Laser II volley in slot 1 uses mount 2. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 {int slot=-1;for(int i=0;i<g.weapon_count;i++)if(!g.weapons[i].missiles&&slot<0)slot=i;assert(slot>=0);
  g.enemy.hull=g.enemy.max_hull=999;g.weapons[slot].charge_ticks=g.weapons[slot].cooldown_ticks;g.weapons[slot].enabled=1;
  assert(game_fire(&g,slot,g.enemy.rooms[0].id));int seen=0;for(int t=0;t<40&&!seen;t++){game_step(&g);for(int i=0;i<GAME_MAX_EFFECTS;i++)if(g.effects[i].active&&g.effects[i].side==0){assert(g.effects[i].mount==slot+1);seen=1;}}
  assert(seen);}
 printf("PASS: V68 shot paths (%d room checks, %d mounted / %d fallback origins, %d original mounts aft of the weapons room).\n",checks,mounted,fallback,aft);
 return 0;
}
