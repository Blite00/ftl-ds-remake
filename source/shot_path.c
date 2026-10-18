#include "shot_path.h"
#include "player_ships.h"
#include "enemy_catalog.h"
#include "enemy_catalog_art.h"
#include "flagship_data.h"
#include "flagship_art.h"
/* Enemy hull art: hull box in room-grid pixels, art source size, 90x65 art rect, mounts. */
typedef struct { int hx,hy,hw,hh,sw,sh,ax,ay,aw,ah; const ImportedEnemyMounts *m; } EnemyHull;
static int enemy_hull(const Game*g,EnemyHull*e){const ImportedEnemyArtRect*a;
 if(game_is_flagship(g)){const FlagshipStage*s=&flagship_stages[g->flagship_stage];a=&imported_flagship_art_rect[g->flagship_stage];
  *e=(EnemyHull){s->hull_x,s->hull_y,s->hull_w,s->hull_h,s->art_source_width,s->art_source_height,a->x,a->y,a->w,a->h,&flagship_mounts[g->flagship_stage]};return 1;}
 int i=game_enemy_layout_index(g);if(i<0||i>=IMPORTED_ENEMY_COUNT)return 0;
 const ImportedEnemy*d=&imported_enemies[i];a=&imported_enemy_art_rect[i];
 *e=(EnemyHull){d->hull_x,d->hull_y,d->hull_w,d->hull_h,d->art_source_width,d->art_source_height,a->x,a->y,a->w,a->h,&imported_enemy_mounts[i]};return 1;}
static const PlayerShipHull*player_hull(const Game*g){return g->player_ship>=0&&g->player_ship<PLAYER_SHIP_COUNT?&player_ship_hulls[g->player_ship]:0;}
static const Ship*ship_of(const Game*g,int side){return side?&g->enemy:&g->player;}
static const GameRoom*room_of(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
static void bounds(const Ship*s,int*mx,int*my,int*xx,int*yy){*mx=*my=1000;*xx=*yy=-1000;
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];if(r->x<*mx)*mx=r->x;if(r->y<*my)*my=r->y;if(r->x+r->w>*xx)*xx=r->x+r->w;if(r->y+r->h>*yy)*yy=r->y+r->h;}
 if(*xx<=*mx){*mx=0;*xx=1;}if(*yy<=*my){*my=0;*yy=1;}}
void shot_geometry(const Ship*s,int enemy,int*ox,int*oy,int*scale){int mx,my,xx,yy;bounds(s,&mx,&my,&xx,&yy);
 int height=enemy?64:84,t=enemy?24:14;if((xx-mx)*t>232)t=232/(xx-mx);if((yy-my)*t>height)t=height/(yy-my);if(t<1)t=1;
 *scale=t;*ox=(256-(xx-mx)*t)/2-mx*t;*oy=(enemy?44:25)+(height-(yy-my)*t)/2-my*t;}
/* Procedural enemy thumbnail (no imported art): tile size and origin inside the 84x52 box at 160,55. */
static void enemy_mini_grid(const Game*g,int*t,int*x,int*y,int*mx,int*my){int xx,yy;bounds(&g->enemy,mx,my,&xx,&yy);
 *t=84/(xx-*mx);if((yy-*my)**t>52)*t=52/(yy-*my);if(*t<1)*t=1;*x=160+(84-(xx-*mx)**t)/2;*y=55+(52-(yy-*my)**t)/2;}
void shot_ship_point(const Game*g,int side,int view,int px,int py,int*x,int*y){
 if(view!=SHOT_VIEW_OVERVIEW){int ox,oy,t;shot_geometry(ship_of(g,side),side,&ox,&oy,&t);*x=ox+px*t/35;*y=oy+py*t/35;return;}
 if(!side){const PlayerShipHull*h=player_hull(g);
  if(h){*x=4+h->mini_x+(px-h->hull_x)*h->mini_w/h->hull_w;*y=47+h->mini_y+(py-h->hull_y)*h->mini_h/h->hull_h;}
  else{*x=4+(px+59)*110/652;*y=47+(py+103)*70/418;}return;}
 EnemyHull e;if(enemy_hull(g,&e)){*x=156+e.ax+(px-e.hx)*e.aw/e.hw;*y=50+e.ay+(py-e.hy)*e.ah/e.hh;return;}
 int t,bx,by,mx,my;enemy_mini_grid(g,&t,&bx,&by,&mx,&my);*x=bx+(px-mx*35)*t/35;*y=by+(py-my*35)*t/35;
}
void shot_room_rect(const Game*g,int side,int view,const GameRoom*r,int*x,int*y,int*w,int*h){
 if(view!=SHOT_VIEW_OVERVIEW){int ox,oy,t;shot_geometry(ship_of(g,side),side,&ox,&oy,&t);*x=ox+r->x*t;*y=oy+r->y*t;*w=r->w*t;*h=r->h*t;return;}
 if(!side){const PlayerShipHull*p=player_hull(g);
  if(p){*x=4+p->mini_x+(r->x*35-p->hull_x)*p->mini_w/p->hull_w;*y=47+p->mini_y+(r->y*35-p->hull_y)*p->mini_h/p->hull_h;*w=r->w*35*p->mini_w/p->hull_w;*h=r->h*35*p->mini_h/p->hull_h;}
  else{*x=4+(r->x*35+59)*110/652;*y=47+(r->y*35+103)*70/418;*w=r->w*35*110/652;*h=r->h*35*70/418;}return;}
 EnemyHull e;if(enemy_hull(g,&e)){*x=156+e.ax+(r->x*35-e.hx)*e.aw/e.hw;*y=50+e.ay+(r->y*35-e.hy)*e.ah/e.hh;*w=r->w*35*e.aw/e.hw;*h=r->h*35*e.ah/e.hh;return;}
 int t,bx,by,mx,my;enemy_mini_grid(g,&t,&bx,&by,&mx,&my);*x=bx+(r->x-mx)*t;*y=by+(r->y-my)*t;*w=r->w*t;*h=r->h*t;
}
void shot_hull_rect(const Game*g,int side,int view,int*x,int*y,int*w,int*h){const Ship*s=ship_of(g,side);int mx,my,xx,yy;bounds(s,&mx,&my,&xx,&yy);
 if(view==SHOT_VIEW_OVERVIEW){const PlayerShipHull*p;EnemyHull e;
  if(!side&&(p=player_hull(g))){*x=4+p->mini_x;*y=47+p->mini_y;*w=p->mini_w;*h=p->mini_h;}
  else if(!side){*x=4;*y=47;*w=110;*h=70;}
  else if(enemy_hull(g,&e)){*x=156+e.ax;*y=50+e.ay;*w=e.aw;*h=e.ah;}
  else{*x=157;*y=51;*w=90;*h=60;}
  return;}
 int ox,oy,t;shot_geometry(s,side,&ox,&oy,&t);
 const PlayerShipHull*p;EnemyHull e;
 if(!side&&(p=player_hull(g))){*x=ox+p->hull_x*t/35;*y=oy+p->hull_y*t/35;*w=p->hull_w*t/35;*h=p->hull_h*t/35;}
 else if(side&&enemy_hull(g,&e)){*x=ox+e.hx*t/35;*y=oy+e.hy*t/35;*w=e.hw*t/35;*h=e.hh*t/35;}
 else{*x=ox+mx*t-3;*y=oy+my*t-3;*w=(xx-mx)*t+6;*h=(yy-my)*t+6;}
}
void shot_shield(const Game*g,int side,int view,int*cx,int*cy,int*rx,int*ry){
 if(view==SHOT_VIEW_OVERVIEW){if(side){*cx=200;*cy=83;*rx=48;*ry=37;}else{*cx=58;*cy=82;*rx=55;*ry=38;}return;}
 const Ship*s=ship_of(g,side);int mx,my,xx,yy,ox,oy,t;bounds(s,&mx,&my,&xx,&yy);shot_geometry(s,side,&ox,&oy,&t);
 *cx=ox+(mx+xx)*t/2;*cy=oy+(my+yy)*t/2;*rx=(xx-mx)*t/2+8;*ry=(yy-my)*t/2+8;if(*cy-*ry<20)*ry=*cy-20;if(*ry<4)*ry=4;
}
/* The room weapons fire from: the weapons system, else the first artillery room. */
static const GameRoom*weapons_room(const Game*g,int side){const Ship*s=ship_of(g,side);const GameRoom*r=0;
 if(s->systems[SYS_WEAPONS].capacity>0)r=room_of(s,s->systems[SYS_WEAPONS].room);
 if(!r&&side&&g->artillery_count)r=room_of(s,g->artillery[0].room);
 if(!r&&!side&&g->player_artillery.capacity>0)r=room_of(s,g->player_artillery.room);
 return r;}
int shot_mount(const Game*g,int side,int slot,int view,int*x,int*y){int px,py;
 if(!side){const PlayerShipHull*h=player_hull(g);
  if(h&&slot>=0&&slot<h->mount_count){shot_ship_point(g,0,view,h->hull_x+h->mounts[slot][0],h->hull_y+h->mounts[slot][1],x,y);return 1;}}
 else{EnemyHull e;
  if(enemy_hull(g,&e)&&slot>=0&&slot<e.m->count){
   if(view==SHOT_VIEW_OVERVIEW){*x=156+e.ax+e.m->xy[slot][0]*e.aw/e.sw;*y=50+e.ay+e.m->xy[slot][1]*e.ah/e.sh;}
   else{int ox,oy,t;shot_geometry(&g->enemy,1,&ox,&oy,&t);*x=ox+e.hx*t/35+e.m->xy[slot][0]*(e.hw*t/35)/e.sw;*y=oy+e.hy*t/35+e.m->xy[slot][1]*(e.hh*t/35)/e.sh;}
   return 1;}}
 /* Fallback: the forward edge of the weapons room (player right, enemy left). */
 const GameRoom*r=weapons_room(g,side);
 if(r){px=(side?r->x:r->x+r->w)*35;py=(2*r->y+r->h)*35/2;}
 else{int mx,my,xx,yy;bounds(ship_of(g,side),&mx,&my,&xx,&yy);px=(side?mx:xx)*35;py=(my+yy)*35/2;}
 shot_ship_point(g,side,view,px,py,x,y);return 0;
}
/* Ellipse test: <0 inside, >0 outside. */
static long long ellipse_f(int x,int y,int cx,int cy,int rx,int ry){long long dx=x-cx,dy=y-cy,a=rx,b=ry;return dx*dx*b*b+dy*dy*a*a-a*a*b*b;}
/* Where a shot from (x0,y0) toward (x1,y1) meets the ellipse; when the segment
 * does not cross it, the radial projection of the aim point onto it. */
static void ellipse_entry(int x0,int y0,int x1,int y1,int cx,int cy,int rx,int ry,int*x,int*y){
 if(ellipse_f(x0,y0,cx,cy,rx,ry)>0&&ellipse_f(x1,y1,cx,cy,rx,ry)<=0){int lo=0,hi=4096;
  while(hi-lo>1){int m=(lo+hi)/2;if(ellipse_f(x0+(x1-x0)*m/4096,y0+(y1-y0)*m/4096,cx,cy,rx,ry)>0)lo=m;else hi=m;}
  *x=x0+(x1-x0)*lo/4096;*y=y0+(y1-y0)*lo/4096;return;}
 int dx=x1-cx,dy=y1-cy;if(!dx&&!dy){dx=x0-cx;dy=y0-cy;}if(!dx&&!dy)dx=1;
 int lo=0,hi=1024;while(hi<(1<<20)&&ellipse_f(cx+dx*hi/1024,cy+dy*hi/1024,cx,cy,rx,ry)<=0)hi*=2;
 while(hi-lo>1){int m=(lo+hi)/2;if(ellipse_f(cx+(int)((long long)dx*m/1024),cy+(int)((long long)dy*m/1024),cx,cy,rx,ry)>0)hi=m;else lo=m;}
 *x=cx+(int)((long long)dx*hi/1024);*y=cy+(int)((long long)dy*hi/1024);
}
static int in_room(const Game*g,int side,int view,int x,int y){const Ship*s=ship_of(g,side);
 for(int i=0;i<s->room_count;i++){int rx,ry,rw,rh;shot_room_rect(g,side,view,&s->rooms[i],&rx,&ry,&rw,&rh);if(x>=rx&&x<rx+rw&&y>=ry&&y<ry+rh)return 1;}
 return 0;}
int shot_path(const Game*g,int index,int view,ShotPath*p){
 if(!g||!p||index<0||index>=GAME_MAX_EFFECTS)return 0;
 const GameEffect*f=&g->effects[index];
 if(!f->active||f->kind==EFFECT_BEAM||f->kind==EFFECT_BOMB)return 0;
 int target=f->side?0:1,shooter=!target,dir=target?1:-1,shown=view==SHOT_VIEW_ENEMY?1:0;
 const GameRoom*r=room_of(ship_of(g,target),f->room);
 if(view!=SHOT_VIEW_OVERVIEW&&shown==shooter){ /* Outgoing: from the mount out of the view toward the target. */
  p->mounted=shot_mount(g,shooter,f->mount-1,view,&p->x0,&p->y0);
  p->gx=p->x1=dir>0?264:-8;p->gy=p->y1=p->y0;p->cx=p->gx;p->cy=p->gy;p->terminal=SHOT_EXIT;return 1;}
 if(!r)return 0;
 {int x,y,w,h;shot_room_rect(g,target,view,r,&x,&y,&w,&h);p->cx=x+w/2;p->cy=y+h/2;}
 if(view==SHOT_VIEW_OVERVIEW)p->mounted=shot_mount(g,shooter,f->mount-1,view,&p->x0,&p->y0);
 else{int y=p->cy-24+(index%3)*12;p->mounted=0;p->x0=dir>0?-8:264;p->y0=y<20?20:y>110?110:y;} /* Incoming from the attacker's side. */
 int ex,ey,erx,ery;shot_shield(g,target,view,&ex,&ey,&erx,&ery);
 ellipse_entry(p->x0,p->y0,p->cx,p->cy,ex,ey,erx,ery,&p->gx,&p->gy);
 p->x1=p->gx;p->y1=p->gy;p->terminal=SHOT_FLIGHT;
 if(f->result==EFFECT_HIT){p->x1=p->cx;p->y1=p->cy;p->terminal=SHOT_ROOM;}
 else if(f->result==EFFECT_SHIELD)p->terminal=SHOT_SHIELD;
 else if(f->result==EFFECT_INTERCEPT){int k=6; /* Shot down short of the target, clear of its rooms. */
  do{p->x1=p->x0+(p->gx-p->x0)*k/8;p->y1=p->y0+(p->gy-p->y0)*k/8;}while(k-->0&&in_room(g,target,view,p->x1,p->y1));
  p->terminal=SHOT_SPACE;}
 else if(f->result==EFFECT_MISS){ /* Past the room, beside the hull: above or below every room. */
  const Ship*s=ship_of(g,target);int top=1000,bottom=-1000;
  for(int i=0;i<s->room_count;i++){int x,y,w,h;shot_room_rect(g,target,view,&s->rooms[i],&x,&y,&w,&h);if(y<top)top=y;if(y+h>bottom)bottom=y+h;}
  p->x1=p->cx+dir*16;p->y1=2*p->cy<top+bottom?top-6:bottom+6;p->terminal=SHOT_MISS;}
 return 1;
}
static int impact_ticks(int terminal){return terminal==SHOT_MISS?GAME_EFFECT_IMPACT_TICKS:terminal==SHOT_ROOM?2:0;}
void shot_point(const ShotPath*p,int age,int duration,int*x,int*y){int span=impact_ticks(p->terminal),k=age-duration;
 if(duration<1)duration=1;
 if(age<duration){if(age<0)age=0;*x=p->x0+(p->gx-p->x0)*age/duration;*y=p->y0+(p->gy-p->y0)*age/duration;return;}
 if(k>=span){*x=p->x1;*y=p->y1;return;}
 *x=p->gx+(p->x1-p->gx)*k/span;*y=p->gy+(p->y1-p->gy)*k/span;
}
void shot_heading(const ShotPath*p,int age,int duration,int len,int*dx,int*dy){
 int vx=p->gx-p->x0,vy=p->gy-p->y0;
 if(age>=duration&&(p->x1!=p->gx||p->y1!=p->gy)){vx=p->x1-p->gx;vy=p->y1-p->gy;}
 int ax=vx<0?-vx:vx,ay=vy<0?-vy:vy,n=(ax>ay?ax+ay/2:ay+ax/2);
 if(!n){*dx=len;*dy=0;return;}
 *dx=vx*len/n;*dy=vy*len/n;
}
