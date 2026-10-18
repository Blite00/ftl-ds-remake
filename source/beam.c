#include "beam.h"
#include <stdint.h>
static int compare(int64_t a,int64_t b,int64_t c,int64_t d){int64_t x=a*d-c*b;return x<0?-1:x>0;}
int beam_intersects_room(const GameRoom*r,int x0,int y0,int x1,int y1){
 int axis;int64_t low=0,lowden=1,high=1,highden=1;
 if(!r||r->w<=0||r->h<=0||(x0==x1&&y0==y1))return 0;
 for(axis=0;axis<2;axis++){int64_t p=axis?y0:x0,delta=(int64_t)(axis?y1:x1)-p;
  int64_t min=(int64_t)(axis?r->y:r->x)*GAME_BEAM_UNITS,max=min+(int64_t)(axis?r->h:r->w)*GAME_BEAM_UNITS,a,b,den;
  if(!delta){if(p<=min||p>=max)return 0;continue;}
  if(delta>0){a=min-p;b=max-p;den=delta;}else{a=p-max;b=p-min;den=-delta;}
  if(compare(a,den,low,lowden)>0){low=a;lowden=den;}
  if(compare(b,den,high,highden)<0){high=b;highden=den;}
  if(compare(low,lowden,high,highden)>=0)return 0;
 }
 return compare(low,lowden,high,highden)<0;
}
int beam_valid_line(const Ship*s,int x0,int y0,int x1,int y1){int i,minx=32767,miny=32767,maxx=0,maxy=0,hit=0;
 if(!s||s->room_count<1||x0<0||y0<0||x1<0||y1<0||x0>32767||y0>32767||x1>32767||y1>32767||(x0==x1&&y0==y1))return 0;
 for(i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];int a=r->x*GAME_BEAM_UNITS,b=r->y*GAME_BEAM_UNITS,c=(r->x+r->w)*GAME_BEAM_UNITS,d=(r->y+r->h)*GAME_BEAM_UNITS;
  if(a<minx)minx=a;
  if(b<miny)miny=b;
  if(c>maxx)maxx=c;
  if(d>maxy)maxy=d;
  hit|=beam_intersects_room(r,x0,y0,x1,y1);
 }
 return hit&&x0>=minx&&x0<=maxx&&x1>=minx&&x1<=maxx&&y0>=miny&&y0<=maxy&&y1>=miny&&y1<=maxy;
}
int beam_clamp_line(int x0,int y0,int*x1,int*y1,int length){int64_t dx,dy,squared,lo=0,hi=65536;
 if(!x1||!y1||length<1||length>32767||x0<0||y0<0||*x1<0||*y1<0||x0>32767||y0>32767||*x1>32767||*y1>32767)return 0;
 dx=(int64_t)*x1-x0;dy=(int64_t)*y1-y0;squared=dx*dx+dy*dy;if(!squared)return 0;if(squared<=(int64_t)length*length)return 1;
 /* Ceil integer square root ensures the truncated endpoint cannot exceed range. */
 while(lo<hi){int64_t mid=(lo+hi)/2;if(mid*mid<squared)lo=mid+1;else hi=mid;}
 *x1=x0+(int)(dx*length/lo);*y1=y0+(int)(dy*length/lo);return *x1!=x0||*y1!=y0;
}
