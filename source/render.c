#include "render.h"
#include "audio.h"
#include "setup.h"
#include "outcome.h"
#include "drones.h"
#include "boarding.h"
#include "subsystems.h"
#include "species.h"
#include "species_art.h"
#include "ftl_data.h"
#include "ship_art.h"
#include "combat_art.h"
#include "player_art.h"
#include "hangar.h"
#include "achievements.h"
#include "ui_art.h"
#include "enemy_data.h"
#include "enemy_catalog.h"
#include "enemy_catalog_art.h"
#include "flagship_data.h"
#include "flagship_art.h"
#include "diagnostics.h"
#include "shot_path.h"
#include "system_icons.h"
#include "map_art.h"
#include "ftl_ui_art.h"
#include <stdio.h>
#include <string.h>
#define RGB(r,g,b) (0x8000|((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))
#define BG RGB(10,16,22)
#define PANEL RGB(31,42,46)
#define EDGE RGB(78,96,101)
#define WHITE RGB(231,231,209)
#define MUTED RGB(127,158,174)
#define CYAN RGB(143,197,191)
#define GREEN RGB(133,218,92)
#define RED RGB(245,108,97)
#define GOLD RGB(247,201,105)
/* Original compact font, five bits per scanline. */
static const unsigned char glyph[][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
{14,17,17,15,1,1,14},{0,4,4,31,4,4,0},{0,0,0,31,0,0,0},
{1,2,2,4,8,8,16},{0,4,4,0,4,4,0},{0,0,0,0,0,4,4},
{25,25,2,4,8,19,19},{4,4,4,4,4,0,4},{8,4,2,1,2,4,8},
{0,0,0,0,0,4,8},{14,17,1,2,4,0,4},{4,4,0,0,0,0,0},
/* Original text punctuation: quotes, parentheses, semicolon, equals,
   brackets, ampersand, underscore, asterisk. */
{10,10,0,0,0,0,0},{2,4,8,8,8,4,2},{8,4,2,2,2,4,8},
{0,4,4,0,4,4,8},{0,0,31,0,31,0,0},{14,8,8,8,8,8,14},
{14,2,2,2,2,2,14},{12,18,20,8,21,18,13},{0,0,0,0,0,0,31},
{0,4,21,14,21,4,0},{2,4,8,16,8,4,2}
};
static void rect(uint16_t*p,int x,int y,int w,int h,int c){
 int x1=x+w,y1=y+h;if(x<0)x=0;if(y<0)y=0;if(x1>256)x1=256;if(y1>192)y1=192;
 for(int j=y;j<y1;j++)for(int i=x;i<x1;i++)p[j*256+i]=(uint16_t)c;
}
static void box(uint16_t*p,int x,int y,int w,int h,int c){rect(p,x,y,w,1,c);rect(p,x,y+h-1,w,1,c);rect(p,x,y,1,h,c);rect(p,x+w-1,y,1,h,c);}
static void text_scale(uint16_t*p,int x,int y,const char*s,int c,int scale){
 for(;*s;s++,x+=6*scale){unsigned char ch=(unsigned char)*s;int k=-1;if(ch>='a'&&ch<='z')ch-=32;
 if(ch>='A'&&ch<='Z')k=ch-'A';else if(ch>='0'&&ch<='9')k=26+ch-'0';
 else{const char*q=strchr("+-/:.%!>,?'\"();=[]&_*<",ch);if(q)k=36+(int)(q-"+-/:.%!>,?'\"();=[]&_*<");}
 if(k>=0)for(int j=0;j<7;j++)for(int i=0;i<5;i++)if(glyph[k][j]&(16>>i))rect(p,x+i*scale,y+j*scale,scale,scale,c);
 }
}
static void text(uint16_t*p,int x,int y,const char*s,int c){text_scale(p,x,y,s,c,1);}
static void bar(uint16_t*p,int x,int y,int w,int v,int max,int c){rect(p,x,y,w,4,EDGE);if(max>0)rect(p,x,y,w*(v<0?0:(v>max?max:v))/max,4,c);}
/* V54 original-style frames: the octagonal FTL window (img/window_base.png) at
 * DS scale: hex-textured dark interior, mint border, chamfered corners. The
 * accent colour keeps its meaning: yellow selected, green ready, red danger,
 * grey disabled, mint normal. Geometry (and so every touch target) is unchanged. */
static int mix(int dst,int src,int a,int max);
#define FTL_MINT RGB(235,245,229)
#define FTL_YELLOW RGB(255,224,96)
#define FTL_INK RGB(22,28,30)
static int ftl_tone(int accent){
 if(accent==GOLD)return FTL_YELLOW;
 if(accent==RED)return RGB(245,96,84);
 if(accent==GREEN)return RGB(126,228,96);
 if(accent==MUTED||accent==EDGE)return RGB(112,122,122);
 if(accent==RGB(112,181,255))return accent;
 return FTL_MINT;
}
static int ftl_inset(int j,int h,int c){return j<c?c-j:j>=h-c?j-(h-1-c):0;}
static void panel(uint16_t*p,int x,int y,int w,int h,int accent){
 int c=(w>=16&&h>=16)?3:2,border=ftl_tone(accent),thick=h>=20&&w>=40?2:1;
 for(int j=0;j<h;j++){int yy=y+j;if(yy<0||yy>=192)continue;int in=ftl_inset(j,h,c);
  for(int i=in;i<w-in;i++){int xx=x+i;if(xx<0||xx>=256)continue;
   int edge=j<thick||j>=h-thick||i<in+thick||i>=w-in-thick;
   p[yy*256+xx]=(uint16_t)(edge?border:ui_hex_tile[(yy&7)*8+(xx&7)]);}}
}
/* Original buttons are solid mint (yellow when selected) with dark lettering;
 * disabled ones stay dark with grey lettering. */
static void button(uint16_t*p,int x,int y,int w,int h,const char*s,int c){
 int tone=ftl_tone(c),off=c==MUTED||c==EDGE;
 if(off)panel(p,x,y,w,h,c);
 else{int ch=h>=16?3:2;for(int j=0;j<h;j++){int in=ftl_inset(j,h,ch);rect(p,x+in,y+j,w-2*in,1,tone);}
  rect(p,x+ch,y+h-1,w-2*ch,1,mix(tone,FTL_INK,6,16));}
 text(p,x+(w-(int)strlen(s)*6+1)/2,y+(h-7)/2,s,off?RGB(150,160,160):FTL_INK);
}
static void segments(uint16_t*p,int x,int y,int w,int count,int value,int c){
 if(count<1)return;
 for(int i=0;i<count;i++){int a=x+i*w/count,b=x+(i+1)*w/count;rect(p,a,y,b-a-1,4,i<value?c:EDGE);}
}
static const char*weapon_status(const Game*g,int w){
 if(w<0||w>=g->weapon_count)return "NO WEAPONS";
 if(!game_weapon_powered(g,w))return "NO POWER";
 if(g->weapons[w].pending_shots)return "FIRING";
 if(!game_weapons_manned(g,0))return "UNMANNED";
 if(g->missiles<g->weapons[w].missiles)return "EMPTY";
 if(g->weapons[w].charge_ticks<g->weapons[w].cooldown_ticks)return "CHARGING";
 return "READY";
}
static int weapon_ready(const Game*g,int w){return strcmp(weapon_status(g,w),"READY")==0;}
static const char*short_name(int s){static const char*n[]={"SHD","ENG","WPN","O2","MED"};return s>=0&&s<GAME_SYSTEM_COUNT?n[s]:"";}
static void geometry(const Ship*s,int*ox,int*oy,int*scale,int enemy){shot_geometry(s,enemy,ox,oy,scale);} /* V68: shared with shot_path.c. */
static void bitmap(uint16_t*p,int x,int y,const uint16_t*a,int w,int h){
 int left=x<0?-x:0,top=y<0?-y:0,right=w,bottom=h;
 if(x+right>256)right=256-x;
 if(y+bottom>192)bottom=192-y;
 for(int j=top;j<bottom;j++){
  const uint16_t*src=a+j*w;uint16_t*dst=p+(y+j)*256;
  for(int i=left;i<right;i++)if(src[i])dst[x+i]=src[i];
 }
}
static void space_background(uint16_t*p){bitmap(p,0,0,ui_space,UI_SPACE_WIDTH,UI_SPACE_HEIGHT);}
/* Room cells are engine-drawn: the original floor PNG is only a gray base.
 * These light panels approximate the gameplay floor at DS resolution. */
static void floor_tile(uint16_t*p,int x,int y,int size,int oxygen){
 for(int j=0;j<size;j++)for(int i=0;i<size;i++){
  unsigned c=(i==0||j==0)?RGB(119,121,112):
             ((i==1||j==1)?RGB(232,230,206):RGB(208,208,186));
  if(oxygen<1000){int amount=(1000-oxygen)*23/1000;
   int r=c&31,g=(c>>5)&31,b=(c>>10)&31;
   r+=(25-r)*amount/32;g+=(5-g)*amount/32;b+=(4-b)*amount/32;
   c=0x8000|r|(g<<5)|(b<<10);
  }
  rect(p,x+i,y+j,1,1,(int)c);
 }
}
static const char*target_name(const Game*g,const GameRoom*r){
 if(!r)return "UNKNOWN";
 if(r->system>=0)return game_system_name(r->system);
 if(g->enemy_drone.capacity>0&&r->id==g->enemy_drone.room)return "DRONES";
 for(int i=0;i<g->artillery_count;i++)if(r->id==g->artillery[i].room)return "ARTILLERY";
 if(g->enemy_cloak.capacity>0&&r->id==g->enemy_cloak.room)return "CLOAKING";
 if(g->teleporter[1].installed&&r->id==g->teleporter[1].room)return "TELEPORT";
 /* Imported pilot placement follows the selected enemy layout. */
 if(r->id==g->enemy_pilot_room)return "PILOT";
 return "EMPTY ROOM";
}
static void fog_room_label(uint16_t*p,const Game*g,const GameRoom*r,int side,int x,int y,int width){
 char label[12];const char*name=side?target_name(g,r):r->system>=0?game_system_name(r->system):"";
 if(side&&strcmp(name,"EMPTY ROOM")&&strcmp(name,"UNKNOWN"))snprintf(label,sizeof label,"%.*s",width>=22?3:1,name);
 else if(!side&&*name)snprintf(label,sizeof label,"%.*s",width>=22?3:1,name);
 else snprintf(label,sizeof label,"%d",r->id+1);
 text(p,x+2,y+2,label,MUTED);
}
static void line(uint16_t*p,int x0,int y0,int x1,int y1,int c){
 int dx=x1>x0?x1-x0:x0-x1,sx=x0<x1?1:-1,dy=y1>y0?y0-y1:y1-y0,sy=y0<y1?1:-1,err=dx+dy;
 for(;;){rect(p,x0,y0,1,1,c);if(x0==x1&&y0==y1)break;int e=err*2;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}
}
static void ellipse(uint16_t*p,int x,int y,int rx,int ry,int c){
 static const int points[17][2]={{100,0},{92,38},{71,71},{38,92},{0,100},{-38,92},{-71,71},{-92,38},{-100,0},{-92,-38},{-71,-71},{-38,-92},{0,-100},{38,-92},{71,-71},{92,-38},{100,0}};
 for(int i=0;i<16;i++)line(p,x+rx*points[i][0]/100,y+ry*points[i][1]/100,x+rx*points[i+1][0]/100,y+ry*points[i+1][1]/100,c);
}
static void burst(uint16_t*p,int x,int y,int age,int color){
 int r=3+age;line(p,x-r,y,x+r,y,color);line(p,x,y-r,x,y+r,color);line(p,x-r/2,y-r/2,x+r/2,y+r/2,color);line(p,x-r/2,y+r/2,x+r/2,y-r/2,color);box(p,x-2,y-2,5,5,WHITE);
}
static const GameRoom*room_by_id(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
/* V57: the current layout's furnishing, shown only while its system is installed. */
static int furnished(const Game*g,int room,int code){
 if(code>=0&&code<GAME_SYSTEM_COUNT)return g->player.systems[code].capacity>0&&g->player.systems[code].room==room;
 switch(code){case 5:return g->pilot_room==room;case 6:return g->subsystems[0][SUB_SENSORS].capacity>0;case 7:return g->subsystems[0][SUB_DOORS].capacity>0;
 case 8:return g->teleporter[0].installed;case 9:return g->drones.installed;case 10:return g->player_cloak.capacity>0;case 11:return g->player_artillery.capacity>0;default:return 0;}
}
static const uint16_t*furnishing(const Game*g,int id,int*w,int*h){int code=-1;const uint16_t*p;
 player_art_use(g->player_ship);p=player_art_interior(id,w,h,&code);
 return p&&furnished(g,id,code)?p:0;
}
/* Upper-case ship name without "The " (the Kestrel keeps reading KESTREL). */
static const char*ship_label(const Game*g){static char s[24];const char*n=game_layout_name(g->player_ship);int i=0;
 if(!strncmp(n,"The ",4))n+=4;
 for(;n[i]&&i<23;i++)s[i]=(char)(n[i]>='a'&&n[i]<='z'?n[i]-32:n[i]);
 s[i]=0;return s;}
static const char*room_label(const Game*g,const GameRoom*r){
 if(r->system>=0)return short_name(r->system);
 if(r->id==g->pilot_room)return "P";
 if(g->subsystems[0][SUB_DOORS].capacity>0&&r->id==g->subsystems[0][SUB_DOORS].room)return "DOR";
 if(g->subsystems[0][SUB_SENSORS].capacity>0&&r->id==g->subsystems[0][SUB_SENSORS].room)return "SNS";
 return "";
}
static void draw_doors(uint16_t*p,const Game*g,const Ui*u,int ox,int oy,int t){
 for(int i=0;i<g->door_count;i++){
  const GameDoor*d=&g->doors[i];int air=d->room_a<0||d->room_b<0;
  int x=ox+d->x*t+(d->vertical?0:t/2),y=oy+d->y*t+(d->vertical?t/2:0);
  int visible=game_room_visible(g,0,d->room_a)||game_room_visible(g,0,d->room_b);
  if(!visible)continue;
  int color=d->jam_ticks?RED:d->open?GREEN:(air?GOLD:subsystem_level(g,0,SUB_DOORS)>=2?CYAN:MUTED);
  if(u->doors)box(p,x-4,y-4,9,9,air?GOLD:EDGE);
  if(d->force_ticks>0)rect(p,x-3,y-6,1+d->force_ticks*6/120,2,RED);
  if(!d->open&&subsystem_level(g,0,SUB_DOORS)>=2)box(p,x-2,y-2,5,5,CYAN);
  if(d->vertical){rect(p,x-1,y-3,3,7,d->open?BG:color);if(d->open){rect(p,x-1,y-4,3,2,color);rect(p,x-1,y+3,3,2,color);}}
  else{rect(p,x-3,y-1,7,3,d->open?BG:color);if(d->open){rect(p,x-4,y-1,2,3,color);rect(p,x+3,y-1,2,3,color);}}
  if(air&&d->open){
   const GameRoom*r=room_by_id(&g->player,d->room_a<0?d->room_b:d->room_a);
   if(r){int direction=d->vertical?(d->x<=r->x?-1:1):(d->y<=r->y?-1:1);
    int distance=4+(g->ticks/3)%7;
    for(int q=-1;q<=1;q++){if(d->vertical)line(p,x+direction*3,y+q,x+direction*distance,y+q*3,CYAN);else {int yy=y+direction*distance;if(yy<19)yy=19;if(yy>113)yy=113;line(p,x+q,y+direction*3,x+q*3,yy,CYAN);}}
   }
  }
 }
}
/* Imported hull art and its source-pixel box: catalog enemies or a V42 flagship stage. */
static int enemy_art(const Game*g,const uint16_t**art,const ImportedEnemyArtRect**a,int*hx,int*hy,int*hw,int*hh){
 if(game_is_flagship(g)){const FlagshipStage*st=&flagship_stages[g->flagship_stage];*art=imported_flagship_art[g->flagship_stage];*a=&imported_flagship_art_rect[g->flagship_stage];*hx=st->hull_x;*hy=st->hull_y;*hw=st->hull_w;*hh=st->hull_h;return 1;}
 int i=game_enemy_layout_index(g);if(i<0)return 0;
 const ImportedEnemy*d=&imported_enemies[i];*art=imported_enemy_art[i];*a=&imported_enemy_art_rect[i];*hx=d->hull_x;*hy=d->hull_y;*hw=d->hull_w;*hh=d->hull_h;return 1;
}
/* Art, room overlays and impact coordinates share the source layout (V68: shot_room_rect in shot_path.c). */
static void procedural_hull(uint16_t*p,int x,int y,int w,int h){
 panel(p,x,y,w,h,RGB(78,95,108));
 line(p,x+3,y+h/2,x-4,y+h/2,CYAN);line(p,x+w,y+h/2,x+w+4,y+h/2,RED);
 box(p,x+2,y+2,w-4,h-4,RGB(112,120,127));
}
static void imported_hull(uint16_t*p,const Game*g,int ox,int oy,int t){
 const uint16_t*art;const ImportedEnemyArtRect*a;int hx,hy,hw,hh;if(!enemy_art(g,&art,&a,&hx,&hy,&hw,&hh))return;
 int x=ox+hx*t/35,y=oy+hy*t/35,w=hw*t/35,h=hh*t/35,cloaked=game_enemy_cloaked(g);
 for(int j=0;j<h;j++)for(int i=0;i<w;i++){
  unsigned color=art[(a->y+j*a->h/h)*IMPORTED_ENEMY_ART_WIDTH+a->x+i*a->w/w];
  if(cloaked&&((i+j)&1))continue;
  if(color)rect(p,x+i,y+j,1,1,(int)color);
 }
}
static void flame(uint16_t*p,int x,int y,int tick){
 rect(p,x-2,y-3,5,7,RED);rect(p,x-1,y-5-(tick/4)%2,3,8,GOLD);rect(p,x,y-1,1,4,WHITE);
}
static void breach_marker(uint16_t*p,int x,int y,int tick){
 ellipse(p,x,y,3,3,CYAN);rect(p,x-1,y-1,3,3,BG);
 line(p,x-4,y+3,x-2,y+1,WHITE);line(p,x+2,y-1,x+4,y-3,(tick/6)%2?CYAN:WHITE);
}
static int room_ion_ticks(const Game*g,int side,int room){
 if(side&&!game_enemy_details_visible(g))return 0;
 if(!game_room_visible(g,side,room))return 0;
 const Ship*s=side?&g->enemy:&g->player;int ticks=0;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)if(s->systems[i].room==room&&s->systems[i].ion_ticks>ticks)ticks=s->systems[i].ion_ticks;
 for(int i=0;i<SUB_COUNT;i++)if(g->subsystems[side][i].room==room&&g->subsystems[side][i].ion_ticks>ticks)ticks=g->subsystems[side][i].ion_ticks;
 if(g->teleporter[side].installed&&g->teleporter[side].room==room&&g->teleporter[side].ion_ticks>ticks)ticks=g->teleporter[side].ion_ticks;
 if(side&&g->enemy_drone.capacity&&g->enemy_drone.room==room&&g->enemy_drone.ion_ticks>ticks)ticks=g->enemy_drone.ion_ticks;
 if(room==(side?g->enemy_pilot_room:g->pilot_room)&&g->pilot_ion_ticks[side]>ticks)ticks=g->pilot_ion_ticks[side];
 return ticks;
}
/* V46 room system icons: original s_*_overlay silhouettes tinted like FTL
 * (green working, orange damaged, red destroyed, grey unpowered, blue ion). */
enum SystemIcon{ICON_SHIELDS,ICON_ENGINES,ICON_WEAPONS,ICON_OXYGEN,ICON_MEDBAY,ICON_PILOT,ICON_DOORS,ICON_SENSORS,ICON_DRONES,ICON_TELEPORTER,ICON_ARTILLERY,ICON_CLOAKING};
#define ICON_GREEN RGB(110,240,100)
#define ICON_ORANGE RGB(255,150,40)
#define ICON_RED RGB(250,60,50)
#define ICON_GREY RGB(160,160,160)
#define ICON_ION RGB(120,190,255)
#define ICON_HIDDEN RGB(205,205,190)
static int mix(int dst,int src,int a,int max){
 int dr=dst&31,dg=(dst>>5)&31,db=(dst>>10)&31,sr=src&31,sg=(src>>5)&31,sb=(src>>10)&31;
 return 0x8000|(dr+(sr-dr)*a/max)|((dg+(sg-dg)*a/max)<<5)|((db+(sb-db)*a/max)<<10);
}
static void tint(uint16_t*p,int x,int y,int w,int h,int color,int a){
 for(int j=y<0?0:y;j<y+h&&j<192;j++)for(int i=x<0?0:x;i<x+w&&i<256;i++)p[j*256+i]=(uint16_t)mix(p[j*256+i],color,a,16);
}
static void system_icon(uint16_t*p,int x,int y,int index,int size,int color){
 const uint8_t*m=size==SYSTEM_ICON_12?system_icons_12[index]:system_icons_7[index];
 for(int j=-1;j<=size;j++)for(int i=-1;i<=size;i++){
  int px=x+i,py=y+j;if(px<0||py<0||px>=256||py>=192)continue;
  int v=i>=0&&j>=0&&i<size&&j<size?m[j*size+i]:0;
  if(v>=6){p[py*256+px]=(uint16_t)mix(p[py*256+px],color,v,15);continue;}
  int edge=0;for(int d=0;d<4;d++){int a=i+(d==0)-(d==1),b=j+(d==2)-(d==3);if(a>=0&&b>=0&&a<size&&b<size&&m[b*size+a]>=6)edge=1;}
  if(edge)p[py*256+px]=RGB(24,30,32);else if(v)p[py*256+px]=(uint16_t)mix(p[py*256+px],color,v,15);
 }
}
static int state_color(int health,int capacity,int power,int ion){
 if(health<=0)return ICON_RED;
 if(ion>0)return ICON_ION;
 if(health<capacity)return ICON_ORANGE;
 if(power<=0)return ICON_GREY;
 return ICON_GREEN;
}
/* Returns the icon for a room's system (or -1) and its state colour. */
static int room_icon_state(const Game*g,int side,int room,int*color);
/* V47: enemy damage colours are public at any sensor level, as in the original;
 * power and ion state stay hidden below sensors level 3. */
static int room_icon(const Game*g,int side,int room,int*color){
 int icon=room_icon_state(g,side,room,color);
 if(icon>=0&&side&&!game_enemy_details_visible(g)&&*color!=ICON_RED&&*color!=ICON_ORANGE)*color=ICON_HIDDEN;
 return icon;
}
static int room_icon_state(const Game*g,int side,int room,int*color){
 const Ship*s=side?&g->enemy:&g->player;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++){const GameSystemState*y=&s->systems[i];if(y->capacity>0&&y->room==room){*color=state_color(y->health,y->capacity,game_system_power(s,i),y->ion_ticks);return i;}}
 if(room==(side?g->enemy_pilot_room:g->pilot_room)){*color=(side?g->enemy_pilot_health:g->pilot_health)<=0?ICON_RED:g->pilot_ion_ticks[side]?ICON_ION:ICON_GREEN;return ICON_PILOT;}
 for(int i=0;i<SUB_COUNT;i++){const Subsystem*y=&g->subsystems[side][i];if(y->capacity>0&&y->room==room){*color=state_color(y->health,y->capacity,1,y->ion_ticks);return i==SUB_DOORS?ICON_DOORS:ICON_SENSORS;}}
 if(!side&&g->drones.installed&&g->drones.room==room){*color=state_color(g->drones.health,g->drones.capacity,g->drones.power,g->drones.ion_ticks);return ICON_DRONES;}
 if(side&&g->enemy_drone.capacity>0&&g->enemy_drone.room==room){*color=state_color(g->enemy_drone.health,g->enemy_drone.capacity,g->enemy_drone.power,g->enemy_drone.ion_ticks);return ICON_DRONES;}
 {const Teleporter*y=&g->teleporter[side];if(y->installed&&y->capacity>0&&y->room==room){*color=state_color(y->health,y->capacity,y->power,y->ion_ticks);return ICON_TELEPORTER;}}
 if(side)for(int i=0;i<g->artillery_count;i++){const Artillery*a=&g->artillery[i];if(a->room==room){*color=state_color(a->health,a->capacity,game_artillery_effective(g,i),a->ion_ticks);return ICON_ARTILLERY;}}
 if(side&&g->enemy_cloak.capacity>0&&g->enemy_cloak.room==room){const EnemyCloak*k=&g->enemy_cloak;*color=state_color(k->health,k->capacity,k->power,k->ion_ticks);return ICON_CLOAKING;}
 if(!side&&g->player_artillery.capacity>0&&g->player_artillery.room==room){const Artillery*a=&g->player_artillery;*color=state_color(a->health,a->capacity,a->power,a->ion_ticks);return ICON_ARTILLERY;}
 if(!side&&g->player_cloak.capacity>0&&g->player_cloak.room==room){const EnemyCloak*k=&g->player_cloak;*color=state_color(k->health,k->capacity,k->power,k->ion_ticks);return ICON_CLOAKING;}
 return -1;
}
static void room_icon_draw(uint16_t*p,const Game*g,int side,const GameRoom*r,int x,int y,int w,int h){
 int color,icon=room_icon(g,side,r->id,&color);if(icon<0)return;
 int size=w>=13&&h>=13?SYSTEM_ICON_12:w>=8&&h>=8?SYSTEM_ICON_7:0;
 if(!size){rect(p,x+w/2-1,y+h/2-1,3,3,color);return;}
 system_icon(p,x+(w-size)/2,y+(h-size)/2,icon,size,color);
}
/* V46 crew walk room slot -> door -> next room, never through walls. */
static void door_tiles(const GameDoor*d,const GameRoom*a,int*ax,int*ay,int*bx,int*by){
 int x0=d->vertical?d->x-1:d->x,y0=d->vertical?d->y:d->y-1;
 if(x0>=a->x&&x0<a->x+a->w&&y0>=a->y&&y0<a->y+a->h){*ax=x0;*ay=y0;*bx=d->x;*by=d->y;}else{*ax=d->x;*ay=d->y;*bx=x0;*by=y0;}
}
static void crew_position(const Game*g,const Ship*s,int side,int faction,int k,int ox,int oy,int t,int*cx,int*cy){
 const Crew*c=faction?&g->enemy_crew[k]:&g->crew[k];const GameRoom*r=room_by_id(s,c->room);
 int tx=r->x+k%r->w,ty=r->y+r->h-1;*cx=ox+tx*t;*cy=oy+ty*t;
 if(c->destination==c->room)return;
 const GameRoom*n=room_by_id(s,game_crew_next_room_side(g,faction,k));if(!n)return;
 const GameDoor*doors=side?g->enemy_doors:g->doors,*d=0;int count=side?g->enemy_door_count:g->door_count,best=1<<20;
 for(int i=0;i<count;i++){const GameDoor*q=&doors[i];if((q->room_a==r->id&&q->room_b==n->id)||(q->room_a==n->id&&q->room_b==r->id)){int e=(q->x>tx?q->x-tx:tx-q->x)+(q->y>ty?q->y-ty:ty-q->y);if(e<best){best=e;d=q;}}}
 if(!d)return;
 int ax,ay,bx,by;door_tiles(d,r,&ax,&ay,&bx,&by);
 int px[4]={tx,ax,bx,n->x+k%n->w},py[4]={ty,ay,by,n->y+n->h-1},len[3],total=0;
 for(int i=0;i<3;i++){len[i]=((px[i+1]>px[i]?px[i+1]-px[i]:px[i]-px[i+1])+(py[i+1]>py[i]?py[i+1]-py[i]:py[i]-py[i+1]))*t;total+=len[i];}
 if(!total)return;
 int travel=species_move_ticks(c->species),dist=total*c->move_ticks/travel;
 if(game_crew_blocked(g,faction,k)&&dist>len[0])dist=len[0];
 for(int i=0;i<3;i++){if(dist<=len[i]||i==2){int f=dist<len[i]?dist:len[i],l=len[i]?len[i]:1;
   *cx=ox+px[i]*t+(px[i+1]-px[i])*t*f/l;*cy=oy+py[i]*t+(py[i+1]-py[i])*t*f/l;return;}
  dist-=len[i];}
}
void render_crew_path_point(const Game*g,int faction,int index,int t,int*x,int*y){
 const Crew*c=faction?&g->enemy_crew[index]:&g->crew[index];
 crew_position(g,c->ship?&g->enemy:&g->player,c->ship,faction,index,0,0,t,x,y);
}
static void enemy_doors_view(uint16_t*p,const Game*g,int ox,int oy,int t){
 for(int i=0;i<g->enemy_door_count;i++){const GameDoor*d=&g->enemy_doors[i];
  if(!(game_room_visible(g,1,d->room_a)||game_room_visible(g,1,d->room_b)))continue;
  int x=ox+d->x*t+(d->vertical?0:t/2),y=oy+d->y*t+(d->vertical?t/2:0),color=d->open?GREEN:subsystem_level(g,1,SUB_DOORS)>=2?CYAN:MUTED;
  if(d->vertical){rect(p,x-1,y-3,3,7,d->open?BG:color);if(d->open){rect(p,x-1,y-4,3,2,color);rect(p,x-1,y+3,3,2,color);}}
  else{rect(p,x-3,y-1,7,3,d->open?BG:color);if(d->open){rect(p,x-4,y-1,2,3,color);rect(p,x+3,y-1,2,3,color);}}
 }
}
/* Result tags avoid each other: try above, below, then further out. */
static int callout_boxes[GAME_MAX_EFFECTS][4],callout_count,callout_bottom=182;
static void callout_at(uint16_t*p,int x,int y,const char*s,int color){
 int w=(int)strlen(s)*6+3;if(x-w/2<1)x=1+w/2;if(x+w/2>255)x=255-w/2;if(y<20)y=20;if(y>callout_bottom)y=callout_bottom;
 rect(p,x-w/2,y-1,w,10,PANEL);box(p,x-w/2,y-1,w,10,color);text(p,x-w/2+2,y+1,s,color);
 if(callout_count<GAME_MAX_EFFECTS){int*b=callout_boxes[callout_count++];b[0]=x-w/2;b[1]=y-1;b[2]=w;b[3]=10;}
}
static void callout_room(uint16_t*p,int x,int top,int bottom,const char*s,int color){
 int w=(int)strlen(s)*6+3,tries[4]={top-11,bottom+2,top-22,bottom+13};
 for(int k=0;k<4;k++){int y=tries[k]<20?20:tries[k],cx=x<1+w/2?1+w/2:x>255-w/2?255-w/2:x,clear=1;
  if(y>callout_bottom)continue;
  for(int i=0;i<callout_count;i++){int*b=callout_boxes[i];if(cx-w/2<b[0]+b[2]&&b[0]<cx+w/2&&y-1<b[1]+b[3]&&b[1]<y+9)clear=0;}
  if(clear){callout_at(p,x,y,s,color);return;}}
 callout_at(p,x,top-11,s,color);
}
static void reticle(uint16_t*p,int x,int y,int w,int h,int color,int tick){
 int k=(tick/4)%2;x-=k;y-=k;w+=2*k;h+=2*k;
 rect(p,x,y,4,1,color);rect(p,x,y,1,4,color);rect(p,x+w-4,y,4,1,color);rect(p,x+w-1,y,1,4,color);
 rect(p,x,y+h-1,4,1,color);rect(p,x,y+h-4,1,4,color);rect(p,x+w-4,y+h-1,4,1,color);rect(p,x+w-1,y+h-4,1,4,color);
}
/* V68 projectile sprite along its shot path (style 0 schematic incoming, 1 schematic outgoing, 2 overview). */
static void shot_sprite(uint16_t*p,const ShotPath*sp,const GameEffect*f,int style,int color){
 int x,y,dx,dy,len=style==1?6:style==2?7:8;shot_point(sp,f->age,f->duration,&x,&y);shot_heading(sp,f->age,f->duration,len,&dx,&dy);
 if(f->kind==EFFECT_ION){if(style==1)ellipse(p,x,y,2,2,CYAN);else{ellipse(p,x,y,3,3,CYAN);if(style==2)line(p,x-dx*5/len,y-dy*5/len,x+dx*5/len,y+dy*5/len,WHITE);else rect(p,x-1,y-1,3,3,WHITE);}}
 else if(f->kind==EFFECT_MISSILE){line(p,x-dx,y-dy,x-dx*3/len,y-dy*3/len,GOLD);rect(p,x-2,y-1,5,3,WHITE);}
 else{line(p,x-dx,y-dy,x,y,color);if(!style)line(p,x-dx,y-dy+1,x,y+1,WHITE);}
}
/* V46 bottom-screen combat overlay: shots in flight, their target rooms and how they land. */
static void combat_overlay(uint16_t*p,const Game*g,int shown,int ox,int oy,int t){
 const Ship*s=shown?&g->enemy:&g->player;callout_count=0;callout_bottom=shown?101:104; /* Panels start at y 112 / 115. */
 int view=shown?SHOT_VIEW_ENEMY:SHOT_VIEW_PLAYER;
 for(int i=0;i<GAME_MAX_EFFECTS;i++){const GameEffect*f=&g->effects[i];if(!f->active)continue;
  int target=f->side?0:1,flight=f->age<f->duration,denom=f->duration?f->duration:1;ShotPath sp;
  if(target!=shown){
   /* V68: outgoing volleys leave from the firing weapon's hull mount toward the target's side. */
   if(flight&&shot_path(g,i,view,&sp))shot_sprite(p,&sp,f,1,shown?RED:GREEN);
   continue;
  }
  if(f->kind==EFFECT_BEAM){int x0=ox+f->beam_x0*t/GAME_BEAM_UNITS,y0=oy+f->beam_y0*t/GAME_BEAM_UNITS,x1=ox+f->beam_x1*t/GAME_BEAM_UNITS,y1=oy+f->beam_y1*t/GAME_BEAM_UNITS;
   int pr=flight?f->age:f->duration;int ex=x0+(x1-x0)*pr/denom,ey=y0+(y1-y0)*pr/denom;
   line(p,x0,y0,ex,ey,f->age%2?WHITE:RED);line(p,x0,y0+1,ex,ey+1,RED);
   if(!flight&&f->result==EFFECT_SHIELD)callout_at(p,x0,y0-14,"SHIELD",CYAN);
   else if(!flight&&f->result==EFFECT_MISS)callout_at(p,x0,y0-14,"MISS",MUTED);
   continue;}
  const GameRoom*r=room_by_id(s,f->room);if(!r)continue;
  int rx=ox+r->x*t,ry=oy+r->y*t,rw=r->w*t,rh=r->h*t,cx=rx+rw/2,cy=ry+rh/2,path=shot_path(g,i,view,&sp);
  int color=f->kind==EFFECT_ION?CYAN:f->kind==EFFECT_MISSILE?GOLD:f->kind==EFFECT_BOMB?GOLD:RED;
  if(flight){
   reticle(p,rx-1,ry-1,rw+2,rh+2,color,g->ticks);
   if(f->kind==EFFECT_BOMB){int radius=2+f->age%5;ellipse(p,cx,cy,radius,radius,CYAN);continue;}
   if(path)shot_sprite(p,&sp,f,0,RED);
   continue;
  }
  int age=f->age-f->duration;
  if(f->result==EFFECT_HIT){if(shown)burst(p,cx,cy,age,f->kind==EFFECT_ION?CYAN:RED);tint(p,rx,ry,rw,rh,f->kind==EFFECT_ION?CYAN:RED,age<3?8:4);callout_room(p,cx,ry,ry+rh,f->kind==EFFECT_ION?"ION":"HIT",f->kind==EFFECT_ION?CYAN:RED);}
  else if(f->result==EFFECT_MISS){if(path&&age<GAME_EFFECT_IMPACT_TICKS)shot_sprite(p,&sp,f,0,RED);callout_room(p,cx,ry,ry+rh,"MISS",MUTED);} /* Flies on past the hull. */
  else if(f->result==EFFECT_SHIELD){int sx,sy,rx2,ry2;shot_shield(g,shown,view,&sx,&sy,&rx2,&ry2);
   ellipse(p,sx,sy,rx2,ry2,CYAN);if(path)burst(p,sp.x1,sp.y1,age,CYAN);callout_room(p,cx,ry,ry+rh,"SHIELD",CYAN);}
 }
}
static void ship(uint16_t*p,const Game*g,const Ui*u,int enemy){
 const Ship*s=enemy?&g->enemy:&g->player;int compact=g->weapon_count>0&&!enemy&&u->aiming&&g->weapons[u->weapon>=0&&u->weapon<g->weapon_count?u->weapon:0].bomb;int ox,oy,t;geometry(s,&ox,&oy,&t,enemy||compact);
 {const uint16_t*art;const ImportedEnemyArtRect*a;int hx,hy,hw,hh;int has_art=enemy&&enemy_art(g,&art,&a,&hx,&hy,&hw,&hh);
 if(has_art)imported_hull(p,g,ox,oy,t);
 if(enemy&&!has_art){
  int mx=1000,my=1000,xx=-1000,yy=-1000;
  for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];if(r->x<mx)mx=r->x;if(r->y<my)my=r->y;if(r->x+r->w>xx)xx=r->x+r->w;if(r->y+r->h>yy)yy=r->y+r->h;}
  procedural_hull(p,ox+mx*t-3,oy+my*t-3,(xx-mx)*t+6,(yy-my)*t+6);
 }}
 if(!enemy&&!compact){player_art_use(g->player_ship);bitmap(p,TACTICAL_HULL_SCREEN_X,TACTICAL_HULL_SCREEN_Y,player_art_tactical(),PLAYER_ART_TACTICAL_W,PLAYER_ART_TACTICAL_H);}
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];int x=ox+r->x*t,y=oy+r->y*t,w=r->w*t,h=r->h*t;
 int visible=game_room_visible(g,enemy,r->id);
 int damaged=visible&&((r->system>=0&&s->systems[r->system].health<s->systems[r->system].capacity)||(!enemy&&g->drones.installed&&r->id==g->drones.room&&g->drones.health<g->drones.capacity));
 if(!visible){panel(p,x,y,w,h,(enemy||compact)&&g->target_room==r->id?GOLD:EDGE);fog_room_label(p,g,r,enemy,x,y,w);room_icon_draw(p,g,enemy,r,x,y,w,h);continue;}
 for(int row=0;row<r->h;row++)for(int col=0;col<r->w;col++)
  floor_tile(p,x+col*t,y+row*t,t,s->automated?1000:r->oxygen);
 box(p,x,y,w,h,RGB(38,43,42));
 if((enemy&&g->target_room==r->id)||(!enemy&&(((u->doors||u->controller_focus)&&u->focus_room==r->id)||(compact&&g->target_room==r->id))))box(p,x,y,w,h,GOLD);
 else if(damaged)box(p,x,y,w,h,RED);
 if(room_ion_ticks(g,enemy,r->id))box(p,x+1,y+1,w-2,h-2,CYAN);
 if(!enemy&&!compact){int fw,fh;const uint16_t*art=furnishing(g,r->id,&fw,&fh);if(art)bitmap(p,x+(w-fw)/2,y+(h-fh)/2,art,fw,fh);}
 {int color;if(room_icon(g,enemy,r->id,&color)>=0&&(color==ICON_ORANGE||color==ICON_RED))tint(p,x+1,y+1,w-2,h-2,color,color==ICON_RED?6:4);}
 if(!enemy&&r->system==SYS_WEAPONS&&!game_weapons_manned(g,0)&&(g->ticks/10)%2)box(p,x+1,y+1,w-2,h-2,RED);
 room_icon_draw(p,g,enemy,r,x,y,w,h);
 char number[16];snprintf(number,sizeof number,"%d",r->id+1);
 if(enemy&&g->enemy_drone.capacity>0&&r->id==g->enemy_drone.room)snprintf(number,sizeof number,"DRN");
 int icon_color;const char*label=enemy?(room_icon(g,1,r->id,&icon_color)>=0||w<16?"":number):((u->doors||compact)?room_label(g,r):"");
 if(!enemy&&g->drones.installed&&r->id==g->drones.room)label="DRN";
 for(int q=0;q<r->fires;q++){int cells=r->w*r->h,c=q%cells;flame(p,x+(c%r->w)*t+t/2-(r->breaches&&cells==1?3:0),y+(c/r->w)*t+t/2+1,g->ticks+q*5);}
 for(int q=0;q<r->breaches;q++){int cells=r->w*r->h,c=cells-1-q%cells,bx=x+(c%r->w)*t+t-5,by=y+(c/r->w)*t+t-5;breach_marker(p,bx,by,g->ticks+q*3);
  if(!s->automated&&r->oxygen>0){int d=(g->ticks+q*4)%8;rect(p,bx-d,by-d/2,1,1,CYAN);rect(p,bx+d/2,by-d,1,1,WHITE);}}
 if(*label){int len=(int)strlen(label)*6;rect(p,x+2,y+2,len,8,PANEL);text(p,x+3,y+3,label,damaged?RED:WHITE);}
 }
 if(!enemy)draw_doors(p,g,u,ox,oy,t);else enemy_doors_view(p,g,ox,oy,t);
 /* Existing local sprite with small procedural activity frames. */
 for(int faction=0;faction<2;faction++)for(int k=0;k<(faction?g->enemy_crew_count:g->crew_count);k++){
  const Crew*c=faction?&g->enemy_crew[k]:&g->crew[k];const GameRoom*r=room_by_id(s,c->room);
  if(c->ship!=enemy||!r||c->health<=0||!game_room_visible(g,enemy,c->room))continue;
  int cx,cy,frame=(g->ticks/6)%2;crew_position(g,s,enemy,faction,k,ox,oy,t,&cx,&cy);
  if(c->destination!=c->room&&!game_crew_blocked(g,faction,k))cy-=frame;
  if(!compact&&!faction&&!u->crew_deselected&&k==u->selected_crew)box(p,cx,cy,COMBAT_CREW_WIDTH,COMBAT_CREW_HEIGHT,GOLD);
  if(compact){rect(p,cx+1,cy+1,t>3?t-2:2,t>3?t-2:2,faction?RED:GREEN);continue;}
  bitmap(p,cx,cy,species_art[c->species>=0&&c->species<SPEC_COUNT?c->species:SPEC_HUMAN],SPECIES_ART_WIDTH,SPECIES_ART_HEIGHT);
  rect(p,cx,cy+COMBAT_CREW_HEIGHT-1,COMBAT_CREW_WIDTH,1,faction?RED:GREEN);
  if(c->destination!=c->room)rect(p,cx+2+frame*4,cy+COMBAT_CREW_HEIGHT,2,2,WHITE);
  if(boarding_contested(g,c,faction)){line(p,cx+5,cy+4,cx+12+frame*3,cy+1,frame?WHITE:RED);rect(p,cx+9,cy+4,2,2,RED);}
  if(c->repair_ticks>0){rect(p,cx+8,cy+3-frame*3,4,2,GOLD);rect(p,cx,cy-3,7*c->repair_ticks/(5*GAME_HZ),1,GREEN);}
  if(r->fires>0){for(int q=0;q<3;q++)rect(p,cx+8+q*2,cy+2+q+frame,2,1,CYAN);}
  else if(r->breaches>0&&c->ship==faction&&c->destination==c->room&&!boarding_contested(g,c,faction)){rect(p,cx+8,cy+3-frame*2,4,2,CYAN);rect(p,cx,cy-3,COMBAT_CREW_WIDTH*r->breach_repair_ticks/(5*GAME_HZ),1,CYAN);}
  else if(!faction&&game_crew_healing(g,k)){rect(p,cx+9,cy+1,5,1,frame?GREEN:CYAN);rect(p,cx+11,cy-1,1,5,frame?GREEN:CYAN);}
 }
 if(!enemy)for(int i=0;i<GAME_MAX_EFFECTS;i++){const GameEffect*f=&g->effects[i];if(f->active&&f->side==1&&f->result==EFFECT_HIT&&f->age>=f->duration){const GameRoom*r=room_by_id(s,f->room);if(r&&game_room_visible(g,0,r->id))burst(p,ox+(r->x*2+r->w)*t/2,oy+(r->y*2+r->h)*t/2,f->age-f->duration,f->kind==EFFECT_ION?CYAN:RED);}}
 /* V56 interior and boarding drones: a cyan drone body in the bottom-right of their room. */
 for(int i=0;i<g->drones.slot_count&&i<GAME_MAX_DRONES;i++){const DroneState*d=&g->drones.slots[i];int kind=drones_kind(d->catalog_id);const GameRoom*r;
  if(!d->deployed||d->unit_room<0||(kind==6)!=enemy||(kind!=4&&kind!=5&&kind!=6)||!(r=room_by_id(s,d->unit_room)))continue;
  {int x=ox+(r->x+r->w)*t-8,y=oy+(r->y+r->h)*t-8,frame=(g->ticks/8)%2;box(p,x,y,6,6,CYAN);rect(p,x+2,y+2,2,2,kind==5?RED:kind==6?GOLD:GREEN);rect(p,x+1+frame*3,y-1,1,1,WHITE);}}
 /* V70: the room a shot just hit flashes red with its damage, like the original's impact flash. */
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];if(r->hit_ticks<=0||(enemy&&!game_room_visible(g,1,r->id)))continue;
  {int x=ox+r->x*t,y=oy+r->y*t,w=r->w*t,h=r->h*t;char n[16];if((r->hit_ticks/4)%2){for(int yy=y+1;yy<y+h-1;yy+=2){rect(p,x+1+(yy&2?1:0),yy,w-2,1,RED);}}
   box(p,x,y,w,h,RED);box(p,x+1,y+1,w-2,h-2,RED);
   if(r->hit_damage>0){snprintf(n,sizeof n,"-%d",r->hit_damage);int tw=(int)strlen(n)*6+3,tx=x+w/2-tw/2,ty=y+h/2-5-(GAME_ROOM_HIT_TICKS-r->hit_ticks)/6;rect(p,tx,ty,tw,10,RED);text(p,tx+2,ty+1,n,WHITE);}}}
 /* V69: one numbered marker per weapon in the room it is aimed at, like the original's target icons. */
 for(int i=0;i<g->weapon_count;i++){const WeaponState*w=&g->weapons[i];int aim=game_weapon_aim(g,i),side=w->bomb&&w->bomb_side?0:1;const GameRoom*r;char n[12];
  if(w->beam_length||aim<0||side!=enemy||(enemy&&g->enemy_departed)||!(r=room_by_id(s,aim)))continue;
  {int x=ox+r->x*t+1+i*9,y=oy+r->y*t+1,sel=u&&u->weapon==i;rect(p,x,y,9,10,sel?GOLD:enemy?RED:CYAN);box(p,x,y,9,10,WHITE);snprintf(n,sizeof n,"%d",i+1);text(p,x+2,y+1,n,BG);}}
 if(!compact)combat_overlay(p,g,enemy,ox,oy,t);

}
static void miniature(uint16_t*p,const Game*g,int enemy){
 const Ship*s=enemy?&g->enemy:&g->player;
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];int x,y,w,h;
  shot_room_rect(g,enemy,SHOT_VIEW_OVERVIEW,r,&x,&y,&w,&h); /* V68: per-layout hull placement. */
  if(!game_room_visible(g,enemy,r->id)){rect(p,x,y,w,h,PANEL);box(p,x,y,w,h,enemy&&r->id==g->target_room?GOLD:EDGE);room_icon_draw(p,g,enemy,r,x,y,w,h);continue;}
  rect(p,x,y,w,h,!s->automated&&r->oxygen<250?RGB(108,42,34):RGB(208,208,186));
  box(p,x,y,w,h,enemy&&r->id==g->target_room?GOLD:RGB(49,66,69));
  room_icon_draw(p,g,enemy,r,x,y,w,h);
  if(r->fires>0)rect(p,x+w/2,y+1,2,3,GOLD);
  if(r->breaches>0)rect(p,x+1,y+1,2,2,CYAN);
  if(room_ion_ticks(g,enemy,r->id))box(p,x,y,w,h,CYAN);
  if(r->hit_ticks>0){box(p,x,y,w,h,RED);if((r->hit_ticks/4)%2)rect(p,x+1,y+1,w-2,h-2,RED);} /* V70 impact flash. */
  for(int faction=0;faction<2;faction++)for(int k=0;k<(faction?g->enemy_crew_count:g->crew_count);k++){const Crew*c=faction?&g->enemy_crew[k]:&g->crew[k];if(c->ship==enemy&&c->room==r->id&&c->health>0)rect(p,x+1+k%2*2,y+h-3,2,2,faction?RED:GREEN);}
 }
}
static void beam_mini_point(const Game*g,int side,int bx,int by,int*x,int*y){shot_ship_point(g,!side,SHOT_VIEW_OVERVIEW,bx*35/GAME_BEAM_UNITS,by*35/GAME_BEAM_UNITS,x,y);}
static void battle(uint16_t*p,const Game*g,const Ui*u){
 player_art_use(g->player_ship);bitmap(p,4,47,player_art_combat(),COMBAT_KESTREL_WIDTH,COMBAT_KESTREL_HEIGHT);
 /* V55: a cloaked Kestrel shimmers like the original's translucent hull. */
 if(game_player_cloaked(g))for(int j=0;j<COMBAT_KESTREL_HEIGHT;j++)for(int i=(j&1);i<COMBAT_KESTREL_WIDTH;i+=2)if(player_art_combat()[j*COMBAT_KESTREL_WIDTH+i])rect(p,4+i,47+j,1,1,RGB(40,70,90));
 {const uint16_t*art;const ImportedEnemyArtRect*a;int hx,hy,hw,hh;
  if(!g->enemy_departed&&enemy_art(g,&art,&a,&hx,&hy,&hw,&hh))bitmap(p,156,50,art,IMPORTED_ENEMY_ART_WIDTH,IMPORTED_ENEMY_ART_HEIGHT);
  else if(!g->enemy_departed)procedural_hull(p,157,51,90,60);}
 miniature(p,g,0);if(!g->enemy_departed)miniature(p,g,1);
 if(game_enemy_cloaked(g)){rect(p,174,96,52,10,BG);text(p,180,97,"CLOAKED",CYAN);}
 if(g->enemy_departed)text(p,157,73,g->result_reason==RESULT_ENEMY_ESCAPE?"JUMPED AWAY":"NO CONTACT",CYAN);
 if(g->player.hull>0&&g->player.shields)ellipse(p,58,82,55,38,RGB(35,128,159));
 if(g->enemy.hull>0&&g->enemy.shields)ellipse(p,200,83,48,37,RGB(35,128,159));
 if(game_enemy_super_shield(g)){ellipse(p,200,83,50,39,GREEN);ellipse(p,200,83,51,40,GREEN);}
 if(g->player.hull>0&&g->player.systems[SYS_ENGINES].power){int flicker=(g->ticks/3)%3;line(p,4,71,9+flicker,71,CYAN);line(p,4,97,9+flicker,97,CYAN);}
 for(int i=0;i<GAME_MAX_EFFECTS;i++){const GameEffect*f=&g->effects[i];if(!f->active)continue;
  if(f->kind==EFFECT_BOMB){const GameRoom*r=room_by_id(f->side?&g->player:&g->enemy,f->room);if(!r)continue;int x,y;beam_mini_point(g,f->side,(r->x*2+r->w)*GAME_BEAM_UNITS/2,(r->y*2+r->h)*GAME_BEAM_UNITS/2,&x,&y);
   if(f->age<f->duration){int radius=2+(f->age%5);ellipse(p,x,y,radius,radius,CYAN);rect(p,x-1,y-1,3,3,WHITE);}
   else if(f->result==EFFECT_MISS)text(p,x-12,y-9,"MISS",MUTED);
   else burst(p,x,y,f->age-f->duration,f->age%2?GOLD:WHITE);
   continue;
  }
  if(f->kind==EFFECT_BEAM){int x0,y0,x1,y1;beam_mini_point(g,f->side,f->beam_x0,f->beam_y0,&x0,&y0);beam_mini_point(g,f->side,f->beam_x1,f->beam_y1,&x1,&y1);int progress=f->age<f->duration?f->age:f->duration,denom=f->duration?f->duration:1;line(p,x0,y0,x0+(x1-x0)*progress/denom,y0+(y1-y0)*progress/denom,f->age%2?WHITE:GOLD);continue;}
  /* V68: from the firing weapon's mount to the targeted room (shield ellipse on a block, beside the hull on a miss). */
  ShotPath sp;if(!shot_path(g,i,SHOT_VIEW_OVERVIEW,&sp))continue;int k=f->age-f->duration;
  if(f->age<f->duration)shot_sprite(p,&sp,f,2,f->side?RED:GREEN);
  else if(f->result==EFFECT_INTERCEPT){/* Interception is shown in space, never as a hull strike. */
   burst(p,sp.x1,sp.y1,k,CYAN);text(p,101,51,"INTERCEPT",CYAN);
  }else if(f->result==EFFECT_MISS){if(k<GAME_EFFECT_IMPACT_TICKS)shot_sprite(p,&sp,f,2,f->side?RED:GREEN);text(p,sp.cx-12,sp.cy-10,"MISS",MUTED);}
  else if(f->result==EFFECT_SHIELD){int ex,ey,erx,ery;if((f->side?g->player.hull:g->enemy.hull)<=0)continue;shot_shield(g,!f->side,SHOT_VIEW_OVERVIEW,&ex,&ey,&erx,&ery);ellipse(p,ex,ey,erx,ery,CYAN);burst(p,sp.x1,sp.y1,k,CYAN);}
  else burst(p,sp.x1,sp.y1,k,f->kind==EFFECT_ION?CYAN:RED);
 }
 int selected=u->weapon>=0&&u->weapon<g->weapon_count?u->weapon:0,home=g->weapon_count>0&&g->weapons[selected].bomb&&g->weapons[selected].bomb_side;
 /* V69: each weapon's own target, numbered; the selected weapon's is highlighted. */
 for(int i=0;i<g->weapon_count;i++){const WeaponState*w=&g->weapons[i];int side=w->bomb&&w->bomb_side,aim=game_weapon_aim(g,i);const GameRoom*r;char n[12];
  if(w->beam_length||aim<0||(!side&&g->enemy_departed)||!(r=room_by_id(side?&g->player:&g->enemy,aim)))continue;
  {int x,y;beam_mini_point(g,side,(r->x*2+r->w)*GAME_BEAM_UNITS/2,(r->y*2+r->h)*GAME_BEAM_UNITS/2,&x,&y);if(i==selected){box(p,x-5,y-5,11,11,home?RED:GOLD);}snprintf(n,sizeof n,"%d",i+1);text(p,x-5+i*7,y-15,n,i==selected?GOLD:WHITE);}}
}
static int selected_weapon(const Game*g,const Ui*u){return u->weapon>=0&&u->weapon<g->weapon_count?u->weapon:0;}
/* Boarding uses a separate schematic so weapon targeting retains its layout. */
static void board_geometry(const Ship*s,int*ox,int*oy,int*t){geometry(s,ox,oy,t,1);}
void ui_board_action(Game*g,Ui*u,int action){
 int ok=0;
 if(action==0)ok=boarding_deploy(g,u->board_party,u->board_room);
 else if(action==1)ok=boarding_recall(g,u->board_party);
 else if(action==2){if(!g->teleporter[0].installed){ui_feedback(u,"Install a teleporter at a store.",1);return;}
  for(int i=0;i<g->crew_count;i++)if((u->board_party&(1u<<i))&&g->crew[i].ship==0)ok+=game_order_crew(g,i,g->teleporter[0].room);
  ui_feedback(u,ok?"Party ordered to teleporter.":"Select living crew aboard your ship.",!ok);return;
 }else ok=boarding_power(g,0,action==3?-1:1);
 ui_feedback(u,g->message,!ok);
}
static void board_view(const Game*g,const Ui*u,uint16_t*p){
 char msg[64];rect(p,0,0,256,192,BG);
 if(u->abandon_confirm){panel(p,3,20,250,135,RED);text(p,10,32,"ABANDON ALL AWAY CREW?",RED);text(p,10,53,"THEY WILL BE LOST PERMANENTLY.",WHITE);text(p,10,72,"RECALL THEM IF POSSIBLE.",WHITE);button(p,6,112,120,32,"CANCEL",CYAN);button(p,132,112,118,32,"ABANDON",RED);return;}
 panel(p,0,0,256,20,CYAN);snprintf(msg,sizeof msg,"BOARD %d/%d / MAX2",u->board_page+1,(g->crew_count+5)/6);text(p,5,7,msg,CYAN);button(p,136,0,46,20,"PAGE",CYAN);
 if(g->outcome==GAME_VICTORY&&game_away_crew(g))button(p,184,0,72,20,"ABANDON",RED);
 const Teleporter*tport=&g->teleporter[0];
 if(tport->installed&&tport->ion_ticks)snprintf(msg,sizeof msg,"TELEPORT ION LOCK %dS",(tport->ion_ticks+GAME_HZ-1)/GAME_HZ);
 else if(tport->installed)snprintf(msg,sizeof msg,"TELEPORT P%d HP%d/%d WAIT %dS",species_teleporter_power(g,0),tport->health,tport->capacity,(tport->cooldown_ticks+GAME_HZ-1)/GAME_HZ);
 else {snprintf(msg,sizeof msg,"NO TELEPORTER / BUY AT STORE");}
 text(p,5,24,msg,tport->health?GOLD:RED);
 int invaders=0;for(int i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].ship==0&&g->enemy_crew[i].health>0&&game_room_visible(g,0,g->enemy_crew[i].room))invaders++;
 const GameRoom*selected=room_by_id(u->board_side?&g->enemy:&g->player,u->board_room);
 if(!game_room_visible(g,u->board_side,u->board_room))snprintf(msg,sizeof msg,"R%d %s / INTERIOR HIDDEN",u->board_room+1,u->board_side?target_name(g,selected):selected&&selected->system>=0?game_system_name(selected->system):"HOME");
 else if(selected&&(selected->breaches||selected->fires))snprintf(msg,sizeof msg,"%s R%d FIRE%d BREACH%d O2 %d%%",u->board_side?"ENEMY":"HOME",u->board_room+1,selected->fires,selected->breaches,selected->oxygen/10);
 else snprintf(msg,sizeof msg,"%s ROOM %d / HOSTILES %d",u->board_side?"ENEMY":"HOME",u->board_room+1,invaders);
 if(room_ion_ticks(g,u->board_side,u->board_room))snprintf(msg,sizeof msg,"ROOM %d ION LOCK %dS",u->board_room+1,(room_ion_ticks(g,u->board_side,u->board_room)+GAME_HZ-1)/GAME_HZ);
 if(u->board_side&&g->enemy_departed)snprintf(msg,sizeof msg,"SHIP DEPARTED / USE HOME");
 text(p,5,34,msg,invaders?RED:WHITE);
 const Ship*s=u->board_side?&g->enemy:&g->player;int ox,oy,t;board_geometry(s,&ox,&oy,&t);
 for(int i=0;i<s->room_count&&!(u->board_side&&g->enemy_departed);i++){const GameRoom*r=&s->rooms[i];int x=ox+r->x*t,y=oy+r->y*t;
  panel(p,x,y,r->w*t,r->h*t,r->id==u->board_room?GOLD:EDGE);
  if(!game_room_visible(g,u->board_side,r->id)){fog_room_label(p,g,r,u->board_side,x,y,r->w*t);continue;}
  if(room_ion_ticks(g,u->board_side,r->id))box(p,x+1,y+1,r->w*t-2,r->h*t-2,CYAN);
  if(r->fires)flame(p,x+r->w*t-5,y+6,g->ticks);
  if(r->breaches)breach_marker(p,x+r->w*t-5,y+r->h*t-5,g->ticks);
  snprintf(msg,sizeof msg,"%d",r->id+1);text(p,x+2,y+2,msg,WHITE);
  for(int faction=0;faction<2;faction++)for(int j=0;j<(faction?g->enemy_crew_count:g->crew_count);j++){const Crew*c=faction?&g->enemy_crew[j]:&g->crew[j];if(c->health>0&&c->ship==u->board_side&&c->room==r->id)rect(p,x+2+(j%3)*4,y+r->h*t-5-faction*4,3,3,faction?RED:GREEN);}
 }
 for(int j=0;j<6;j++){int i=u->board_page*6+j;if(i>=g->crew_count)continue;snprintf(msg,sizeof msg,"%.7s %s",game_crew_name(g,i),g->crew[i].health<=0?"X":g->crew[i].ship?"E":"H");button(p,2+(j%3)*85,111+(j/3)*15,83,14,msg,u->board_party&(1u<<i)?GOLD:MUTED);}
 static const char*const actions[]={"P-","P+","GATHER","SEND","RECALL"};
 for(int i=0;i<5;i++)button(p,1+i*51,143,50,20,actions[i],i>=3?GREEN:CYAN);
 button(p,1,170,62,22,"HOME",!u->board_side?GOLD:MUTED);button(p,65,170,62,22,"ENEMY",u->board_side?GOLD:MUTED);
 button(p,129,170,62,22,g->paused?"GO X":"PAUSE X",GOLD);button(p,193,170,62,22,"BACK B",CYAN);
}
static void board_touch(Game*g,Ui*u,int x,int y){
 if(u->abandon_confirm){if(y>=112&&y<144){u->abandon_confirm=x>=132?2:0;}return;}
 if(y<20&&x>=136&&x<183){u->board_page=(u->board_page+1)%((g->crew_count+5)/6);return;}
 if(y<20&&x>=184&&g->outcome==GAME_VICTORY&&game_away_crew(g)){u->abandon_confirm=1;return;}
 if(y>=170){if(x<128){u->board_side=x>=64;u->board_room=(u->board_side?g->enemy.rooms:g->player.rooms)[0].id;}else if(x<192)game_toggle_pause(g);else u->boarding=0;return;}
 if(y>=143&&y<163){int button_index=x/51;static const int actions[]={3,4,2,0,1};if(button_index<5)ui_board_action(g,u,actions[button_index]);return;}
 if(y>=111&&y<141){if(x<2||x>=255||(x-2)%85>=83)return;int i=u->board_page*6+(y-111)/15*3+(x-2)/85;if(i<g->crew_count){unsigned mask=1u<<i;if(u->board_party&mask)u->board_party&=~mask;else {int n=0;for(int j=0;j<g->crew_count;j++)if(u->board_party&(1u<<j))n++;if(n>=2){ui_feedback(u,"Select no more than two crew.",1);return;}u->board_party|=mask;}u->selected_crew=i;}return;}
 if(y>=44&&y<109){const Ship*s=u->board_side?&g->enemy:&g->player;int ox,oy,t;board_geometry(s,&ox,&oy,&t);
  for(int r=0;r<s->room_count;r++){const GameRoom*q=&s->rooms[r];if(x>=ox+q->x*t&&x<ox+(q->x+q->w)*t&&y>=oy+q->y*t&&y<oy+(q->y+q->h)*t){u->board_room=q->id;for(int i=0;i<g->crew_count;i++)if((u->board_party&(1u<<i))&&g->crew[i].ship==u->board_side)game_order_crew(g,i,q->id);ui_feedback(u,u->board_side?"Enemy room selected.":"Home room selected.",0);return;}}
 }
}
static void crew_species_view(const Game*g,const Ui*u,uint16_t*p){
 if(u->selected_crew<0||u->selected_crew>=g->crew_count)return;
 const Crew*c=&g->crew[u->selected_crew];char linebuf[64];space_background(p);
 panel(p,4,4,248,28,GOLD);snprintf(linebuf,sizeof linebuf,"%s / %s",game_crew_name(g,u->selected_crew),species_name(c->species));text(p,10,14,linebuf,GOLD);
 bitmap(p,12,40,species_art[c->species>=0&&c->species<SPEC_COUNT?c->species:SPEC_HUMAN],SPECIES_ART_WIDTH,SPECIES_ART_HEIGHT);
 snprintf(linebuf,sizeof linebuf,"MAX HEALTH %d",species_max_health(c->species));text(p,38,44,linebuf,WHITE);
 static const char*const traits[SPEC_COUNT][3]={
  {"LEARNS STATION SKILLS FASTER.","BALANCED MOVEMENT AND REPAIR.",""},
  {"REPAIRS TWICE AS FAST.","DEALS HALF MELEE DAMAGE.",""},
  {"FASTER MOVEMENT, STRONGER MELEE.","REPAIRS AT HALF SPEED.",""},
  {"IMMUNE TO FIRE DAMAGE.","HIGH HEALTH, HALF MOVEMENT SPEED.",""},
  {"SENSES ROOM AND LIFEFORM CONTACTS.","SENSING WORKS WITHOUT SENSORS.",""},
  {"BOOSTS OWN OCCUPIED SYSTEM POWER.","LOW MAXIMUM HEALTH.",""},
  {"TAKES LESS SUFFOCATION DAMAGE.","MOVES SLOWER. LOCKS A ROOM 12S.","TELEPORTING STILL WORKS."}
 };
 int species=c->species>=0&&c->species<SPEC_COUNT?c->species:SPEC_HUMAN;
 for(int i=0;i<3;i++)text(p,10,72+i*16,traits[species][i],WHITE);
 if(species==SPEC_CRYSTAL){snprintf(linebuf,sizeof linebuf,"LOCKDOWN READY IN %d SECONDS",(c->ability_ticks+GAME_HZ-1)/GAME_HZ);text(p,10,128,linebuf,c->ability_ticks?MUTED:CYAN);}
 text(p,10,154,"DETAILS HOLD TIME WHILE OPEN.",MUTED);
}
static void crew_detail_view(const Game*g,const Ui*u,uint16_t*p,int management){
 if(u->selected_crew<0||u->selected_crew>=g->crew_count)return;
 const Crew*c=&g->crew[u->selected_crew];char linebuf[64];
 rect(p,0,0,256,192,BG);panel(p,0,0,256,23,GOLD);
 if(u->dismiss_confirm){text(p,8,8,"DISMISS CREW?",RED);text(p,8,38,game_crew_name(g,u->selected_crew),WHITE);text(p,8,60,"THIS CREW MEMBER WILL LEAVE.",WHITE);button(p,4,112,122,32,"CANCEL",CYAN);button(p,130,112,122,32,"DISMISS",RED);return;}
 text(p,8,8,u->renaming?"RENAME CREW / MAX 12 LETTERS":"CREW DETAILS / TIME HELD",GOLD);
 if(u->renaming){
  panel(p,6,28,244,27,CYAN);text(p,12,38,u->rename_text,WHITE);
  for(int i=0;i<26;i++){char key[2]={(char)('A'+i),0};button(p,2+(i%7)*36,62+(i/7)*23,34,21,key,WHITE);}
  button(p,2,158,80,18,"SPACE",CYAN);button(p,87,158,80,18,"DELETE",GOLD);
  button(p,2,178,122,14,"CANCEL B",MUTED);button(p,130,178,124,14,"DONE",GREEN);return;
 }
 snprintf(linebuf,sizeof linebuf,"%s / %s",game_crew_name(g,u->selected_crew),species_name(c->species));text(p,8,31,linebuf,WHITE);
 snprintf(linebuf,sizeof linebuf,"HP %d/%d / %s",c->health,species_max_health(c->species),game_crew_action(g,u->selected_crew));text(p,8,45,linebuf,c->health<25?RED:CYAN);
 snprintf(linebuf,sizeof linebuf,"%s ROOM %d > %d",c->ship?"ENEMY":"HOME",c->room+1,c->destination+1);text(p,8,59,linebuf,WHITE);
 snprintf(linebuf,sizeof linebuf,"SAVED STATION %d",c->station+1);text(p,8,73,linebuf,GOLD);
 static const char*const skills[]={"PILOT","ENGINE","WEAPON","SHIELD"};
 for(int i=0;i<4;i++){snprintf(linebuf,sizeof linebuf,"%s %d/3600",skills[i],c->skill[i]);text(p,8+(i%2)*126,89+(i/2)*13,linebuf,MUTED);}
 const GameRoom*room=room_by_id(c->ship?&g->enemy:&g->player,c->room);
 if(room&&game_room_visible(g,c->ship,c->room)&&(room->fires||room->breaches)){snprintf(linebuf,sizeof linebuf,"ROOM: FIRE %d / BREACH %d",room->fires,room->breaches);text(p,8,111,linebuf,room->fires?RED:CYAN);}
 button(p,4,119,122,23,"RENAME",CYAN);button(p,130,119,122,23,"DISMISS",management&&c->ship==0&&c->health>0&&game_alive_crew(g)>1?RED:MUTED);
 button(p,4,146,122,23,"SAVE STNS X",GOLD);button(p,130,146,122,23,"RETURN Y",GREEN);
 button(p,4,175,c->species==SPEC_CRYSTAL?122:248,17,"CLOSE / B",WHITE);if(c->species==SPEC_CRYSTAL)button(p,130,175,122,17,"LOCKDOWN",c->ability_ticks?MUTED:CYAN);
}
static void crew_detail_touch(Game*g,Ui*u,int x,int y,int management){
 if(u->dismiss_confirm){if(y>=112&&y<144)u->dismiss_confirm=x>=130?2:0;return;}
 if(u->renaming){
  size_t n=strlen(u->rename_text);
  if(y>=62&&y<154){int row=(y-62)/23,col=x/36,i=row*7+col;if(col<7&&i<26&&n<12){u->rename_text[n]=(char)('A'+i);u->rename_text[n+1]=0;}}
  else if(y>=158&&y<176){if(x<82&&n<12){u->rename_text[n]=' ';u->rename_text[n+1]=0;}else if(x>=87&&x<167&&n)u->rename_text[n-1]=0;}
  else if(y>=178){if(x<128)u->renaming=0;else {int ok=game_rename_crew(g,u->selected_crew,u->rename_text);ui_feedback(u,ok?"Crew renamed.":"Enter a name with letters.",!ok);if(ok)u->renaming=0;}}
  return;
 }
 if(y>=175){if(x>=130&&g->crew[u->selected_crew].species==SPEC_CRYSTAL){int ok=game_crystal_lockdown(g,u->selected_crew);ui_feedback(u,g->message,!ok);if(ok)u->crew_details=0;}else u->crew_details=0;}
 else if(y>=146&&y<169){if(x<128){game_save_stations(g);ui_feedback(u,"Crew stations saved.",0);}else {game_return_stations(g);ui_feedback(u,"Returning crew to saved stations.",0);}}
 else if(y>=119&&y<142){if(x>=130){if(!management)ui_feedback(u,"Dismiss crew between battles.",1);else if(g->crew[u->selected_crew].ship||g->crew[u->selected_crew].health<=0||game_alive_crew(g)<=1)ui_feedback(u,"Cannot dismiss: away, dead or last crew.",1);else u->dismiss_confirm=1;return;}u->renaming=1;snprintf(u->rename_text,sizeof u->rename_text,"%s",game_crew_name(g,u->selected_crew));}
}
static void beam_pick(Game*g,Ui*u,int x,int y){
 int w=selected_weapon(g,u);if(!g->weapon_count||!g->weapons[w].beam_length)return;
 int mx=1000,my=1000,xx=-1000,yy=-1000;for(int i=0;i<g->enemy.room_count;i++){const GameRoom*r=&g->enemy.rooms[i];if(r->x<mx)mx=r->x;if(r->y<my)my=r->y;if(r->x+r->w>xx)xx=r->x+r->w;if(r->y+r->h>yy)yy=r->y+r->h;}
 if(x<mx*GAME_BEAM_UNITS||x>=xx*GAME_BEAM_UNITS||y<my*GAME_BEAM_UNITS||y>=yy*GAME_BEAM_UNITS){ui_feedback(u,"Pick a point inside the enemy layout.",1);return;}
 if(!u->beam_pending||u->beam_weapon!=w){u->beam_pending=1;u->beam_weapon=w;u->beam_x=x;u->beam_y=y;ui_feedback(u,"Beam start set. Pick the end point.",0);}
 else{int ok=game_set_beam_target(g,w,u->beam_x,u->beam_y,x,y);ui_feedback(u,ok?"Beam line set. Press FIRE when ready.":g->message,!ok);if(ok)u->beam_pending=0;}
}
void ui_select_weapon(Game*g,Ui*u,int index){if(index<0||index>=g->weapon_count)return;u->weapon=index;u->beam_pending=0;WeaponState*w=&g->weapons[index];const Ship*s=w->bomb&&w->bomb_side?&g->player:&g->enemy;int room=room_by_id(s,w->aim_room)?w->aim_room:room_by_id(s,g->target_room)?g->target_room:s->rooms[0].id;g->target_room=room;if(w->aim_room==GAME_AIM_HOLD)return; /* V69: a cleared weapon stays unarmed until a room is picked. */
 if(w->bomb)game_set_bomb_target(g,index,w->bomb_side,room);else if(!w->beam_length)game_set_weapon_target(g,index,room);}
void ui_bomb_toggle_side(Game*g,Ui*u){int w=selected_weapon(g,u);if(!g->weapon_count||!g->weapons[w].bomb)return;int side=!g->weapons[w].bomb_side;const Ship*s=side?&g->player:&g->enemy;int room=s->rooms[0].id;int ok=game_set_bomb_target(g,w,side,room);if(ok)g->target_room=room;ui_feedback(u,ok?(side?"Bomb targets HOME. Friendly fire enabled.":"Bomb targets ENEMY."):g->message,!ok);}
void ui_beam_pick_room(Game*g,Ui*u){const GameRoom*r=room_by_id(&g->enemy,g->target_room);if(r)beam_pick(g,u,(2*r->x+r->w)*GAME_BEAM_UNITS/2,(2*r->y+r->h)*GAME_BEAM_UNITS/2);}
static void beam_target_view(const Game*g,const Ui*u,uint16_t*p){
 int w=selected_weapon(g,u);const WeaponState*b=&g->weapons[w];if(!b->beam_length)return;
 int ox,oy,t;geometry(&g->enemy,&ox,&oy,&t,1);
 if(b->beam_target_valid){int x0=ox+b->beam_x0*t/GAME_BEAM_UNITS,y0=oy+b->beam_y0*t/GAME_BEAM_UNITS,x1=ox+b->beam_x1*t/GAME_BEAM_UNITS,y1=oy+b->beam_y1*t/GAME_BEAM_UNITS;line(p,x0,y0,x1,y1,CYAN);box(p,x0-2,y0-2,5,5,GOLD);box(p,x1-2,y1-2,5,5,CYAN);}
 if(u->beam_pending&&u->beam_weapon==w){int x=ox+u->beam_x*t/GAME_BEAM_UNITS,y=oy+u->beam_y*t/GAME_BEAM_UNITS;box(p,x-3,y-3,7,7,GOLD);}
}
void ui_tick(Ui*u){if(u->feedback_ticks>0)u->feedback_ticks--;if(u->tap_ticks>0)u->tap_ticks--;}
void ui_feedback(Ui*u,const char*message,int error){audio_feedback(error);snprintf(u->feedback,sizeof u->feedback,"%.*s",(int)sizeof u->feedback-1,message);u->feedback_ticks=75;u->feedback_error=error;}
void ui_fire(Game*g,Ui*u){if(!g->weapon_count){ui_feedback(u,"No weapons installed. Use ship equipment.",1);return;}int w=selected_weapon(g,u);if(g->weapons[w].beam_length&&u->beam_pending&&u->beam_weapon==w){ui_feedback(u,"Finish the beam line before firing.",1);return;}if(g->weapons[w].aim_room==GAME_AIM_HOLD&&!g->weapons[w].bomb&&!g->weapons[w].beam_length)game_set_weapon_target(g,w,g->target_room);int ok=game_fire(g,w,game_weapon_aim(g,w)<0?g->target_room:game_weapon_aim(g,w));ui_feedback(u,g->outcome?"Combat has ended.":g->message,!ok);}
static void feedback_view(const Ui*u,uint16_t*top,uint16_t*bottom){
 if(u->feedback_ticks){rect(top,3,174,250,18,PANEL);box(top,3,174,250,18,u->feedback_error?RED:CYAN);char line[41];snprintf(line,sizeof line,"%.40s",u->feedback);text(top,8,180,line,u->feedback_error?RED:WHITE);}
 if(u->tap_ticks){int color=u->feedback_error?RED:GOLD;box(bottom,u->tap_x-5,u->tap_y-5,11,11,color);}
}
/* Target identity is public; current system health follows existing sensors. */
/* Weapon families are treated as visible exterior equipment. Sensors still
 * gate operating state, charge and ammunition availability. */
static const char* enemy_weapon_short(const WeaponState*w){
 if(w->catalog_id>=5&&w->catalog_id<GAME_CATALOG_COUNT)return game_weapon_short(w->catalog_id);
 if(w->ion_damage)return "ION";
 if(w->catalog_id==1)return "BURST II";
 if(w->catalog_id==3)return "HEAVY";
 return "BASIC";
}
/* Armament gets one reserved line above the ships, never an opaque hull card. */
static void enemy_loadout_view(const Game*g,uint16_t*p){
 /* V42 flagship artillery: one compact entry per surviving weapon room. */
 if(game_is_flagship(g)&&g->enemy.hull>0&&!g->enemy_departed){char label[24];
  if(!game_enemy_details_visible(g)){snprintf(label,sizeof label,"FLAGSHIP: %d ARTILLERY",g->artillery_count);text(p,8,41,label,GOLD);return;}
  for(int i=0;i<g->artillery_count;i++){const Artillery*a=&g->artillery[i];int pw=game_artillery_effective(g,i);
   const char*kind=a->w.beam_length?"BEAM":a->w.ion_damage?"ION":a->w.missiles?"MIS":"LAS";
   if(a->w.pending_shots)snprintf(label,sizeof label,"%s FIRE",kind);
   else if(!pw)snprintf(label,sizeof label,"%s OFF",kind);
   else if(!game_artillery_manned(g,i))snprintf(label,sizeof label,"%s UNMAN",kind);
   else snprintf(label,sizeof label,"%s %dS",kind,(a->w.cooldown_ticks*(6-pw)-a->charge+4*GAME_HZ-1)/(4*GAME_HZ));
   text(p,8+i*62,41,label,pw?GOLD:MUTED);}
  return;
 }
 if(!g->enemy_weapon_count||g->enemy.hull<=0||g->enemy_departed)return;
 int count=game_enemy_weapon_slots(g);char label[48];
 if(!game_enemy_details_visible(g)){
  int missiles=0,ion=0,lasers=0;for(int i=0;i<count;i++){const WeaponState*w=game_enemy_weapon_at(g,i);missiles|=w->missiles>0;ion|=w->ion_damage>0;lasers|=!w->missiles&&!w->ion_damage;}
  snprintf(label,sizeof label,"ENEMY: %s",missiles?(lasers?"LASER + MISSILE":ion?"ION + MISSILE":"MISSILES"):ion?(lasers?"ION + LASER":"ION"):g->enemy_drone.capacity?"LASER + DRONE":count>1?"DUAL LASERS":"LASER");
  text(p,8,41,label,GOLD);return;
 }
 for(int i=0;i<count;i++){
  const WeaponState*w=game_enemy_weapon_at(g,i);char state[12];int on=game_enemy_weapon_powered(g,i);
  if(w->pending_shots)snprintf(state,sizeof state,"FIRING");
  else if(w->missiles&&g->enemy_missiles<w->missiles)snprintf(state,sizeof state,"EMPTY");
  else if(!on)snprintf(state,sizeof state,"OFF");
  else if(!game_weapons_manned(g,1)&&w->charge_ticks>=w->cooldown_ticks)snprintf(state,sizeof state,"UNMAN");
  else snprintf(state,sizeof state,"%dS",(w->cooldown_ticks-w->charge_ticks+GAME_HZ-1)/GAME_HZ);
  snprintf(label,sizeof label,"E%d %.8s %s",i+1,enemy_weapon_short(w),state);
  text(p,8+i*124,41,label,on?GOLD:MUTED);
 }
}
static void target_card(const Game*g,const Ui*u,uint16_t*p){
 int slot=selected_weapon(g,u);const WeaponState*w=&g->weapons[slot];
 int side=w->bomb&&w->bomb_side?0:1,aim=game_weapon_aim(g,slot);
 if(aim<0){panel(p,4,44,248,72,EDGE);text(p,12,51,"TARGET: NONE - HOLDING FIRE",MUTED);text(p,12,67,"PICK A ROOM TO AIM THIS WEAPON",WHITE);return;}
 const Ship*ship=side?&g->enemy:&g->player;const GameRoom*r=room_by_id(ship,aim);
 char label[64];int visible=side?game_enemy_details_visible(g):game_room_visible(g,0,aim);
 panel(p,4,44,248,72,EDGE);
 if(side&&g->enemy_departed){text(p,12,55,"NO REMOTE TARGET",MUTED);return;}
 const char*name=side?target_name(g,r):r&&r->system>=0?game_system_name(r->system):"HOME ROOM";
 snprintf(label,sizeof label,"TARGET: %s",name);text(p,12,51,label,GOLD);
 const char*effect="HITS DAMAGE HULL AND ROOM OCCUPANTS";
 if(r&&r->system>=0){
  const GameSystemState*system=&ship->systems[r->system];
  if(visible||side){snprintf(label,sizeof label,"SYSTEM %d/%d%s",system->health,system->capacity,system->health?"":"  DISABLED");text(p,12,66,label,system->health?WHITE:RED);segments(p,162,66,77,system->capacity,system->health,system->health?GREEN:RED);}
  else text(p,12,66,"SYSTEM HEALTH: SENSORS LV3",MUTED);
  static const char*const effects[]={"DAMAGE LOWERS SHIELD CAPACITY","DAMAGE LOWERS EVASION / FTL","DAMAGE REDUCES WEAPON POWER","DAMAGE SLOWS OXYGEN SUPPLY","DAMAGE REDUCES CREW HEALING"};
  effect=effects[r->system];
 }else if(side&&aim==g->enemy_pilot_room){effect="PILOT DAMAGE LOWERS EVASION";text(p,12,66,visible?(g->enemy_pilot_health?"PILOT ACTIVE":"PILOT DISABLED"):"SYSTEM HEALTH: SENSORS LV3",MUTED);}
 else text(p,12,66,"NO MAIN SYSTEM IN THIS ROOM",MUTED);
 text(p,12,81,effect,CYAN);
 if(w->ion_damage)snprintf(label,sizeof label,"ION %d: TEMPORARY SYSTEM LOCK",w->ion_damage);
 else snprintf(label,sizeof label,"ON HIT: HULL %d / SYSTEM %d",w->damage,w->system_damage);
 /* A visible strike result does not reveal hidden interior state. Show the
  * freshest resolved effect at this target, including queued earlier volleys. */
 const GameEffect*impact=0;
 for(int i=0;i<GAME_MAX_EFFECTS;i++){
  const GameEffect*f=&g->effects[i];
  if(f->active&&f->side==!side&&f->room==aim&&f->age>=f->duration&&f->result!=EFFECT_PENDING&&(!impact||f->age-f->duration<impact->age-impact->duration))impact=f;
 }
 int color=WHITE;
 if(impact){const char*result=impact->result==EFFECT_MISS?"MISS":impact->result==EFFECT_SHIELD?"SHIELD BLOCK":impact->result==EFFECT_INTERCEPT?"INTERCEPTED":"HIT";snprintf(label,sizeof label,"IMPACT: %s",result);color=impact->result==EFFECT_HIT?GOLD:CYAN;}
 text(p,12,99,label,color);
}
#include "alarm_view.inc"
void render_game(const Game*g,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];rect(top,0,0,256,192,BG);rect(bottom,0,0,256,192,BG);
 if(u->logs){
  if(g->outcome==GAME_PLAYING&&!game_enemy_details_visible(g)){
   text(top,8,8,"DIAGNOSTICS / SIMULATION HELD",CYAN);
   text(top,8,34,"SENSOR CONTACT IS LIMITED.",WHITE);
   text(top,8,50,"DETAILED LOGS HIDDEN UNTIL",MUTED);
   text(top,8,64,"SENSORS REACH LEVEL 3.",MUTED);
   button(bottom,8,176,240,16,"SELECT OR TAP TO RETURN",CYAN);return;
  }
  text(top,8,8,"DIAGNOSTICS / SELECT TO CLOSE",CYAN);
  snprintf(s,sizeof s,"%.40s",diagnostics_status());text(top,8,24,s,GOLD);
  snprintf(s,sizeof s,"TICK %d HULL %d O2 %d",g->ticks,g->player.hull,g->player.oxygen);text(top,8,40,s,WHITE);
  text(top,8,58,"SIMULATION HELD",MUTED);
  unsigned count=diagnostics_line_count();if(count>21)count=21;
  for(unsigned i=0;i<count;i++){const char*entry=diagnostics_line(count-1-i);if(!entry)continue;snprintf(s,sizeof s,"%.40s",entry);if(i<8)text(top,8,78+(int)i*13,s,MUTED);else text(bottom,8,8+((int)i-8)*13,s,MUTED);}
  button(bottom,8,176,240,16,"SELECT OR TAP TO RETURN",CYAN);return;
 }
 space_background(top);space_background(bottom);
 panel(top,2,1,252,39,WHITE);text(top,9,6,g->enemy_departed&&g->result_reason!=RESULT_ENEMY_ESCAPE?"NO ENEMY CONTACT":game_enemy_name(g),WHITE);
 text(top,203,6,g->outcome==GAME_SURRENDER?"PEACE":g->paused?"PAUSE":g->outcome!=GAME_PLAYING?"RESULT":"LIVE",g->paused||g->outcome!=GAME_PLAYING?GOLD:GREEN);
 if(g->enemy_departed)snprintf(s,sizeof s,"%s",g->result_reason==RESULT_ENEMY_ESCAPE?"SHIP DEPARTED":"NO REMOTE SHIP");else snprintf(s,sizeof s,"HULL %d/%d",g->enemy.hull,g->enemy.max_hull);text(top,9,20,s,WHITE);
 segments(top,9,31,164,g->enemy.max_hull,g->enemy.hull,RED);
 int cap=g->enemy.hull>0?game_shield_capacity(&g->enemy):0;
 int enemy_shields=g->enemy.hull>0?g->enemy.shields:0;
 snprintf(s,sizeof s,"SH %d/%d",enemy_shields,cap);text(top,188,20,s,CYAN);
 for(int i=0;i<cap&&i<5;i++){box(top,188+i*12,30,9,6,CYAN);if(i<enemy_shields)rect(top,190+i*12,32,5,2,CYAN);}
 if(enemy_shields<cap)bar(top,188,38,61,g->enemy.shield_ticks,GAME_SHIELD_RECHARGE_TICKS,CYAN);
 if(u->aiming&&g->weapon_count)target_card(g,u,top);else battle(top,g,u);
 if(!u->aiming)for(int i=0;i<g->drones.slot_count;i++)if(g->drones.slots[i].deployed){int x=117+(g->ticks/8+i*7)%20;box(top,x,87-i*9,5,5,drones_powered(g,i)?CYAN:MUTED);text(top,114,98,drones_is_defense(g->drones.slots[i].catalog_id)?"DEF":"DRN",CYAN);}
 if(!u->aiming){
 snprintf(s,sizeof s,"SH %d/%d",g->player.hull>0?g->player.shields:0,g->player.hull>0?game_shield_capacity(&g->player):0);text(top,8,108,s,CYAN);
 if(g->player_super_shield>0){snprintf(s,sizeof s,"Z%d",g->player_super_shield);text(top,52,108,s,GREEN);}
 if(g->player_cloak.capacity>0){const EnemyCloak*k=&g->player_cloak;if(k->active_ticks)snprintf(s,sizeof s,"CLOAK %dS",(k->active_ticks+GAME_HZ-1)/GAME_HZ);else if(k->ion_bars)snprintf(s,sizeof s,"CLOAK WAIT");else snprintf(s,sizeof s,"CLOAK START");text(top,72,108,s,k->active_ticks?GREEN:k->ion_bars?CYAN:MUTED);}
 /* V60 beacon hazard banner between the enemy panel and the ships. */
 if(g->environment){char h[40];if(g->environment==ENV_ASTEROID)snprintf(h,sizeof h,"ASTEROID FIELD");else if(g->environment==ENV_SUN)snprintf(h,sizeof h,"SUN: SOLAR FLARE %dS",(g->env_ticks+GAME_HZ-1)/GAME_HZ);
  else if(g->environment==ENV_NEBULA)snprintf(h,sizeof h,"NEBULA: SENSORS BLIND");else snprintf(h,sizeof h,"ION STORM: REACTOR HALVED");
  text(top,(256-(int)strlen(h)*6)/2,41,h,g->environment==ENV_SUN&&g->env_ticks<=5*GAME_HZ?RED:GOLD);}
 if(g->player_artillery.capacity>0){const Artillery*a=&g->player_artillery;int need=game_player_artillery_need(g);if(need)snprintf(s,sizeof s,"ART %dS",(need-a->charge+GAME_HZ-1)/GAME_HZ);else snprintf(s,sizeof s,"ART OFF");text(top,g->player_cloak.capacity>0?140:72,108,s,need&&a->charge>=need?GOLD:need?GREEN:MUTED);}
 if(g->player.hull>0&&g->player.shields<game_shield_capacity(&g->player))bar(top,8,116,96,g->player.shield_ticks,GAME_SHIELD_RECHARGE_TICKS,CYAN);
 /* V43 flagship drones and the audible power-surge warning (shown without sensors). */
 if(game_is_flagship(g)&&g->enemy.hull>0){char line[24];
  if(g->flagship_drone_count&&game_enemy_details_visible(g)){snprintf(line,sizeof line,"DRONES %d/%d P%d",game_flagship_drones_active(g),g->flagship_drone_count,g->flagship_drone_parts);text(top,157,107,line,GOLD);}
  /* The super shield is visible without sensors, as in the original. */
  if(g->super_shield_max){snprintf(line,sizeof line,"SUPER %d/%d",game_enemy_super_shield(g),g->super_shield_max);text(top,157,107,line,game_enemy_super_shield(g)?GREEN:MUTED);}
  /* Stages 2-3 have at most three artillery entries, leaving the fourth slot free. */
  if(game_surge_warning(g)){snprintf(line,sizeof line,"SURGE %dS",game_surge_warning(g));text(top,8+3*62,41,line,RED);}}
 else if(g->enemy.hull>0&&g->enemy_drone.capacity>0&&game_enemy_details_visible(g)){text(top,157,107,g->enemy_drone.power>=2?"DRONE ACTIVE":"DRONE DOWN",g->enemy_drone.power>=2?GOLD:GREEN);bar(top,157,115,90,g->enemy_drone.charge_ticks,g->enemy_drone.cooldown_ticks,GOLD);rect(top,144+(g->ticks/6)%6,61,5,5,GOLD);}
 if(!g->enemy_weapon_count&&!game_is_flagship(g)&&g->enemy.hull>0&&game_enemy_details_visible(g)&&!g->enemy_drone.capacity){snprintf(s,sizeof s,"WPN%d NEED%d",game_system_power(&g->enemy,SYS_WEAPONS),g->enemy_weapon.power);if(g->enemy.systems[SYS_WEAPONS].ion_ticks)snprintf(s,sizeof s,"WPN ION %dS",(g->enemy.systems[SYS_WEAPONS].ion_ticks+GAME_HZ-1)/GAME_HZ);else if(!game_weapons_manned(g,1))snprintf(s,sizeof s,"WPN UNMANNED");text(top,157,107,s,g->enemy.systems[SYS_WEAPONS].ion_ticks?CYAN:GOLD);bar(top,157,115,90,g->enemy_fire_ticks,g->enemy_weapon.cooldown_ticks,GOLD);}
 if(g->enemy_escape_active&&!g->enemy_departed){
  rect(top,150,102,102,17,PANEL);
  if(game_enemy_escape_ready(g))snprintf(s,sizeof s,"FTL %dS",(g->enemy_escape_ticks+GAME_HZ-1)/GAME_HZ);
  else snprintf(s,sizeof s,"FTL STOPPED");
  text(top,157,104,s,GOLD);bar(top,157,114,90,g->enemy_escape_total-g->enemy_escape_ticks,g->enemy_escape_total,GOLD);
 }
 }
 panel(top,4,119,248,16,GOLD);
 if(!u->aiming)enemy_loadout_view(g,top);
 if(g->player.hull<=0)snprintf(s,sizeof s,"%s DESTROYED / SYSTEMS LOST",ship_label(g));
 else if(!game_room_visible(g,0,g->pilot_room))snprintf(s,sizeof s,"SENSORS DOWN / CREW ROOM SIGHT");
 else snprintf(s,sizeof s,"EVASION %d%% / PILOT %s",game_evasion(g),g->pilot_health<=0?"DOWN":game_station_bonus(g,-1)>0?"MANNED":"EMPTY");
 text(top,10,124,s,g->player.hull<=0?RED:GOLD);
 {int ion_count=0;snprintf(s,sizeof s,"ION");for(int i=0;i<GAME_SYSTEM_COUNT;i++)if(g->player.systems[i].ion_ticks&&game_room_visible(g,0,g->player.systems[i].room)){size_t n=strlen(s);snprintf(s+n,sizeof s-n," %s%dS",short_name(i),(g->player.systems[i].ion_ticks+GAME_HZ-1)/GAME_HZ);ion_count++;}if(ion_count&&g->player.hull>0){rect(top,5,122,244,11,PANEL);text(top,10,124,s,CYAN);}}
 {int fires=0,breaches=0;for(int i=0;i<g->player.room_count;i++){if(game_room_visible(g,0,g->player.rooms[i].id)){fires+=g->player.rooms[i].fires;breaches+=g->player.rooms[i].breaches;}}if((fires||breaches)&&g->player.hull>0){rect(top,5,122,244,11,PANEL);snprintf(s,sizeof s,"FIRE %d / BREACH %d / ORDER CREW",fires,breaches);text(top,10,124,s,fires?RED:CYAN);}}
 {int intruders=0;for(int i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].ship==0&&g->enemy_crew[i].health>0&&game_room_visible(g,0,g->enemy_crew[i].room))intruders++;
  if(intruders&&g->player.hull>0){rect(top,5,122,244,11,PANEL);snprintf(s,sizeof s,"INTRUDERS %d / OPEN BOARD CONTROLS",intruders);text(top,10,124,s,RED);}}
 {int alert=g->player.hull<=0||g->pilot_health<=0||!game_room_visible(g,0,g->pilot_room);
  for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];if(game_room_visible(g,0,r->id)&&(r->fires||r->breaches))alert=1;}
  for(int i=0;i<GAME_SYSTEM_COUNT;i++)if(g->player.systems[i].ion_ticks&&game_room_visible(g,0,g->player.systems[i].room))alert=1;
  for(int i=0;i<g->enemy_crew_count;i++){const Crew*r=&g->enemy_crew[i];if(r->ship==0&&r->health>0&&game_room_visible(g,0,r->room))alert=1;}
  if(!alert&&!u->crew_deselected&&u->controller_focus&&!u->aiming&&!u->boarding&&u->selected_crew>=0&&u->selected_crew<g->crew_count){const GameRoom*r=room_by_id(&g->player,u->focus_room);char destination[24];if(r&&r->system>=0)snprintf(destination,sizeof destination,"%s",game_system_name(r->system));else if(u->focus_room==g->pilot_room)snprintf(destination,sizeof destination,"PILOT");else snprintf(destination,sizeof destination,"ROOM %d",u->focus_room+1);rect(top,5,122,244,11,PANEL);snprintf(s,sizeof s,"%.12s TO %s / A MOVE",game_crew_name(g,u->selected_crew),destination);text(top,10,124,s,GOLD);}
 }
 alarm_warning_view(top,g);
 if(!g->weapon_count){panel(top,3,138,249,34,EDGE);text(top,12,146,"NO WEAPONS INSTALLED",GOLD);text(top,12,158,"BOARD OR FIT WEAPONS BETWEEN BATTLES",MUTED);}
 for(int i=0;i<g->weapon_count;i++){
  int width=252/g->weapon_count,x=3+i*width,w=i,ready=weapon_ready(g,w);
  panel(top,x,138,width-2,34,i==selected_weapon(g,u)?GOLD:(ready?GREEN:EDGE));
  snprintf(s,sizeof s,"%d %.*s",i+1,(width-8)/6-2,game_weapon_short(g->weapons[w].catalog_id));text(top,x+3,143,s,WHITE);
  int unmanned=g->weapons[w].enabled&&game_weapon_powered(g,w)&&!g->weapons[w].pending_shots&&!game_weapons_manned(g,0);
  const char*status=!g->weapons[w].enabled?"OFF":!game_weapon_powered(g,w)?"NO PWR":g->missiles<g->weapons[w].missiles?"EMPTY":g->weapons[w].pending_shots?"FIRING":unmanned?"UNMANNED":ready?"READY":g->autofire[w]?"AUTO":"CHARGE";
  text(top,x+3,154,status,unmanned?RED:ready?GREEN:MUTED);
  bar(top,x+3,165,width-9,g->weapons[w].charge_ticks,g->weapons[w].cooldown_ticks,ready?GREEN:CYAN);
 }
 if(!u->aiming||(!strstr(g->message,"Encounter started.")&&!strstr(g->message,"Select crew, then")&&!strstr(g->message,"Paused: issue orders")&&strcmp(g->message,"Combat resumed."))){snprintf(s,sizeof s,"%.40s",g->message);text(top,8,180,s,MUTED);}
 panel(bottom,0,0,256,19,WHITE);snprintf(s,sizeof s,"%s %d/%d",ship_label(g),g->player.hull,g->player.max_hull);text(bottom,5,5,s,WHITE);
 if(subsystem_level(g,0,SUB_SENSORS)>0)snprintf(s,sizeof s,"O2 %d%%",g->player.oxygen/10);else snprintf(s,sizeof s,"O2 ???");text(bottom,121,5,s,subsystem_level(g,0,SUB_SENSORS)>0&&g->player.oxygen<250?RED:CYAN);
 snprintf(s,sizeof s,"P%d/%d M%d",game_used_power(g),game_reactor(g),g->missiles);text(bottom,177,5,s,GOLD);
 int w=selected_weapon(g,u);
 if(u->aiming&&!g->weapon_count){panel(bottom,4,40,248,116,EDGE);text(bottom,12,55,"NO WEAPONS INSTALLED",GOLD);text(bottom,12,77,"OPEN EQUIPMENT BETWEEN BATTLES.",WHITE);text(bottom,12,99,"BACK RETURNS TO CREW CONTROLS.",CYAN);}
 else if(u->aiming){
  int target_side=g->weapons[w].bomb&&g->weapons[w].bomb_side?0:1;
  if(target_side&&g->enemy_departed){text(bottom,12,57,"NO REMOTE TARGET",CYAN);text(bottom,12,76,"BOMBS CAN STILL TARGET HOME",MUTED);}
  else {ship(bottom,g,u,target_side);beam_target_view(g,u,bottom);}
  panel(bottom,0,19,256,20,EDGE);text(bottom,8,25,target_side&&g->enemy_departed?(g->result_reason==RESULT_ENEMY_ESCAPE?"ENEMY HAS JUMPED AWAY":"NO REMOTE SHIP"):g->weapons[w].beam_length?(u->beam_pending?"BEAM: TAP END POINT":"BEAM: TAP START, THEN END POINT"):g->weapons[w].bomb?"BOMB: PICK ROOM / BYPASSES SHIELDS":"TAP ROOM, THEN FIRE / A",GOLD);
  rect(bottom,0,112,256,80,BG);
  for(int i=0;i<g->weapon_count;i++){snprintf(s,sizeof s,"W%d",i+1);button(bottom,2+i*64,113,60,18,s,i==w?GOLD:MUTED);}
  int aim=game_weapon_aim(g,w);
  if(g->weapons[w].bomb){button(bottom,3,133,120,14,target_side?"ENEMY / DOWN":"HOME / DOWN",target_side?CYAN:RED);text(bottom,130,136,target_side?"BYPASS SHIELDS":"FRIENDLY FIRE",target_side?WHITE:RED);}
  else if(aim<0)text(bottom,5,136,"NO TARGET / B CLEARED - HOLDING FIRE",MUTED);
  else {snprintf(s,sizeof s,"ROOM %d / %s",aim+1,g->weapons[w].beam_length?"SHIELDS REDUCE DAMAGE":g->weapons[w].shield_piercing?"PIERCES SHIELDS":g->enemy.shields?"SHIELDS BLOCK HITS":"SHIELDS DOWN");text(bottom,5,136,g->enemy_departed?"NO REMOTE TARGET":s,WHITE);}
  button(bottom,3,149,122,22,g->weapons[w].enabled?"POWER ON":"POWER OFF",g->weapons[w].enabled?GREEN:MUTED);
  button(bottom,131,149,122,22,g->autofire[w]?"AUTO ON":"AUTO OFF",g->autofire[w]?GREEN:MUTED);
 }else{
  ship(bottom,g,u,0);alarm_ship_view(bottom,g);
  if(u->doors){const GameRoom*r=room_by_id(&g->player,u->focus_room);snprintf(s,sizeof s,"R%d O2%d%% F%d B%d",u->focus_room+1,r?r->oxygen/10:0,r?r->fires:0,r?r->breaches:0);if(!game_room_visible(g,0,u->focus_room))snprintf(s,sizeof s,"R%d NO SENSOR CONTACT",u->focus_room+1);text(bottom,5,120,s,CYAN);button(bottom,155,115,97,19,"SEAL ALL",subsystem_level(g,0,SUB_DOORS)?GOLD:RED);
   int forced=0;for(int i=0;i<g->door_count;i++)if(g->doors[i].force_ticks&&(game_room_visible(g,0,g->doors[i].room_a)||game_room_visible(g,0,g->doors[i].room_b)))forced++;
   if(subsystem_level(g,0,SUB_DOORS))snprintf(s,sizeof s,"DOORS%d HIT%d",subsystem_level(g,0,SUB_DOORS),forced);else snprintf(s,sizeof s,"DOORS DOWN");text(bottom,5,129,s,forced||!subsystem_level(g,0,SUB_DOORS)?RED:CYAN);
  }
  else for(int j=0;j<3;j++){
   int i=u->crew_page*3+j,x=4+j*84;if(i>=g->crew_count)continue;
   panel(bottom,x,115,80,19,(u->group_mode?(u->crew_group&(1u<<i)):(!u->crew_deselected&&i==u->selected_crew))?GOLD:EDGE);
   snprintf(s,sizeof s,"%.8s %d",game_crew_name(g,i),g->crew[i].health);text(bottom,x+4,119,s,g->crew[i].health<25?RED:WHITE);
   bar(bottom,x+4,129,70,g->crew[i].health,species_max_health(g->crew[i].species),game_crew_healing(g,i)?CYAN:GREEN);
  }
  button(bottom,2,135,49,12,"GROUP",u->group_mode?GOLD:CYAN);button(bottom,53,135,29,12,"ALL",CYAN);
  button(bottom,84,135,50,12,"PAGE",CYAN);button(bottom,136,135,39,12,"BOARD",GOLD);button(bottom,177,135,76,12,u->doors?"CREW":"DOORS",GOLD);
 /* V54 original power bars: state-tinted system icon plus one segment per bar
  * (green powered, dark empty, red damaged, blue ionized); buttons unchanged. */
  for(int i=0;i<GAME_SYSTEM_COUNT;i++){int x=2+i*51;const GameSystemState*y=&g->player.systems[i];
   if(!game_room_visible(g,0,y->room)){snprintf(s,sizeof s,"%s ?/?",short_name(i));text(bottom,x,149,s,MUTED);}
   else{int tone=state_color(y->health,y->capacity,game_system_power(&g->player,i),y->ion_ticks);system_icon(bottom,x+1,146,i,SYSTEM_ICON_12,tone);
    int n=y->capacity<1?1:y->capacity,seg=n>8?3:4,power=game_system_power(&g->player,i);
    for(int k=0;k<n&&k<8;k++){int bx=x+15+k*seg,col=k<power?(y->ion_ticks?ICON_ION:ICON_GREEN):k>=y->health?ICON_RED:-1;
     if(col<0){rect(bottom,bx,147,seg-1,10,FTL_MINT);rect(bottom,bx+1,148,seg-3>0?seg-3:1,8,FTL_INK);}else rect(bottom,bx,147,seg-1,10,col);}}
   button(bottom,x,158,23,15,"-",MUTED);button(bottom,x+25,158,23,15,"+",CYAN);}
 }
 button(bottom,2,176,48,16,g->paused?"GO":"PAUSE",GOLD);
 if(g->weapon_count)snprintf(s,sizeof s,"FIRE W%d A",w+1);else snprintf(s,sizeof s,"NO WEAPONS");button(bottom,53,176,81,16,s,weapon_ready(g,w)?GREEN:MUTED);
 button(bottom,136,176,50,16,"DRONES",CYAN);
 button(bottom,189,176,65,16,u->aiming?"BACK Y":"AIM Y",CYAN);
 if(u->controller_focus&&(strstr(g->message,"Encounter started.")||strstr(g->message,"Select crew, then")||strstr(g->message,"Paused: issue orders")||!strcmp(g->message,"Combat resumed.")||!strcmp(g->message,"Crew order set."))){rect(top,3,174,250,18,PANEL);text(top,7,180,u->boarding?"DPAD ROOM / Y SIDE / START ACTIONS":u->aiming?"DPAD ROOM / L-R WEAPON / A FIRE":"DPAD ROOM / L-R CREW / START ACTIONS",MUTED);}
 if(u->boarding)board_view(g,u,bottom);
 if(u->crew_details){crew_species_view(g,u,top);crew_detail_view(g,u,bottom,0);}
}
static void ui_touch_action(Game*g,Ui*u,int x,int y){
 if(x<0||x>=256||y<0||y>=192)return;
 if(u->logs){u->logs=0;return;}
 if(g->outcome==GAME_DEFEAT)return;
 int w=selected_weapon(g,u);
 if(y>=176){if(x<51)game_toggle_pause(g);else if(x<188)ui_fire(g,u);else {u->aiming=!u->aiming;u->beam_pending=0;}return;}
 if(u->aiming){
  if(!g->weapon_count){ui_feedback(u,"No weapons installed.",1);return;}
  if(y>=113&&y<132){int i=x/64;if(i<g->weapon_count){ui_select_weapon(g,u,i);}return;}
  if(g->weapons[w].bomb&&y>=133&&y<147){if(x<124)ui_bomb_toggle_side(g,u);return;}
  if(y>=149&&y<172){if(x<128)game_toggle_weapon(g,w);else game_toggle_autofire(g,w);return;}
  if(y>=112||y<39)return;
  Ship*target_ship=g->weapons[w].bomb&&g->weapons[w].bomb_side?&g->player:&g->enemy;
  if(target_ship==&g->enemy&&g->enemy_departed){ui_feedback(u,"Enemy escaped. No remote target.",1);return;}
  int ox,oy,t;geometry(target_ship,&ox,&oy,&t,1);
  if(g->weapons[w].beam_length){beam_pick(g,u,(x-ox)*GAME_BEAM_UNITS/t,(y-oy)*GAME_BEAM_UNITS/t);return;}
  for(int i=0;i<target_ship->room_count;i++){GameRoom*r=&target_ship->rooms[i];if(x>=ox+r->x*t&&x<ox+(r->x+r->w)*t&&y>=oy+r->y*t&&y<oy+(r->y+r->h)*t){if(g->weapons[w].bomb)game_set_bomb_target(g,w,g->weapons[w].bomb_side,r->id);else game_set_weapon_target(g,w,r->id);g->target_room=r->id;ui_feedback(u,"Target set. Press FIRE or A.",0);return;}}
  return;
 }
 if(y>=158&&y<174){int sys=x/51;if(sys<GAME_SYSTEM_COUNT){int ok=game_adjust_power(g,sys,x%51<27?-1:1);ui_feedback(u,g->message,!ok);}return;}
 if(y>=135&&y<147){if(x<52){u->crew_deselected=0;u->group_mode=!u->group_mode;if(u->group_mode)u->crew_group=1u<<u->selected_crew;}else if(x<83){u->crew_deselected=0;u->group_mode=1;u->crew_group=0;for(int i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==0)u->crew_group|=1u<<i;}else if(x>=177)u->doors=!u->doors;else if(x>=136){u->boarding=1;u->board_page=0;u->board_side=0;u->board_room=g->teleporter[0].installed?g->teleporter[0].room:g->player.rooms[0].id;u->board_party=1u<<u->selected_crew;}else if(x>=84)u->crew_page=(u->crew_page+1)%((g->crew_count+2)/3);return;}
 if(u->doors&&y>=115&&y<134){if(x>=155){game_close_all_doors(g);ui_feedback(u,g->message,!subsystem_level(g,0,SUB_DOORS)||strstr(g->message,"broken")!=0);}return;}
 if(u->doors&&y>=19&&y<115){
  int ox,oy,t,best=-1,distance=1000;geometry(&g->player,&ox,&oy,&t,0);
  for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];int dx=x-(ox+d->x*t+(d->vertical?0:t/2)),dy=y-(oy+d->y*t+(d->vertical?t/2:0));int dist=dx*dx+dy*dy;if(dist<=49&&dist<distance){best=i;distance=dist;}}
  if(best>=0){int ok=game_toggle_door(g,best);ui_feedback(u,g->message,!ok);const GameDoor*d=&g->doors[best];u->focus_room=d->room_a<0?d->room_b:d->room_a;}return;
 }
 if(y>=115&&y<134){if(x<4||x>=252||(x-4)%84>=80)return;int i=u->crew_page*3+(x-4)/84;if(i<g->crew_count){if(u->group_mode){u->crew_group^=1u<<i;u->selected_crew=i;return;}if(!u->crew_deselected&&u->selected_crew==i){u->crew_details=1;u->renaming=0;}u->selected_crew=i;u->crew_deselected=0;ui_feedback(u,"Crew selected. Tap space to clear.",0);}return;}
 int ox,oy,t;geometry(&g->player,&ox,&oy,&t,0);
 for(int i=0;i<g->player.room_count;i++){GameRoom*r=&g->player.rooms[i];if(x>=ox+r->x*t&&x<ox+(r->x+r->w)*t&&y>=oy+r->y*t&&y<oy+(r->y+r->h)*t){if(u->crew_deselected){ui_feedback(u,"Select a crew portrait first.",0);return;}unsigned selected=u->group_mode?u->crew_group:1u<<u->selected_crew;
 for(int k=0;k<g->crew_count;k++)if((selected&(1u<<k))&&g->crew[k].ship){ui_feedback(u,"Use BOARD to order away crew.",1);return;}
 int ok=u->group_mode?game_order_group(g,u->crew_group,r->id):game_order_crew(g,u->selected_crew,r->id);ui_feedback(u,!u->group_mode&&g->crew[u->selected_crew].health<=0?"This crew member is dead.":g->message,!ok);return;}}
 if(y>=19&&y<115)ui_deselect_crew(u);
}

void ui_deselect_crew(Ui*u){u->crew_deselected=1;u->crew_group=0;u->group_mode=0;u->controller_focus=0;ui_feedback(u,"Crew deselected. L/R or tap to select.",0);}

void ui_touch(Game*g,Ui*u,int x,int y){
 if(x<0||x>=256||y<0||y>=192)return;
 if(u->crew_details){crew_detail_touch(g,u,x,y,0);return;}
 if(u->boarding){board_touch(g,u,x,y);return;}
 int paused=g->paused,crew=u->selected_crew,weapon=u->weapon,aim=u->aiming,doors=u->doors,page=u->crew_page;
 u->tap_x=x;u->tap_y=y;u->tap_ticks=8;u->feedback_error=0;
 char previous[96];snprintf(previous,sizeof previous,"%s",g->message);
 ui_touch_action(g,u,x,y);
 if(paused!=g->paused)ui_feedback(u,g->paused?"Paused. Orders are still available.":"Resumed.",0);
 else if(crew!=u->selected_crew)ui_feedback(u,"Crew selected. Tap a destination.",0);
 else if(weapon!=u->weapon)ui_feedback(u,g->weapons[u->weapon].ion_damage?"Ion shots temporarily lock power.":"Weapon selected. Tap an enemy room.",0);
 else if(aim!=u->aiming)ui_feedback(u,u->aiming?"Targeting. Select weapon and room.":"Crew controls.",0);
 else if(doors!=u->doors)ui_feedback(u,u->doors?"Doors selected. Tap door to toggle.":"Crew controls.",0);
 else if(page!=u->crew_page)ui_feedback(u,"Crew page changed.",0);
 else if(strcmp(previous,g->message)&&!u->feedback_error)ui_feedback(u,g->message,0);
}

/* Campaign presentation shares the exact combat framebuffer and touch controls. */
static void outcome_view(const Campaign*c,uint16_t*top,uint16_t*bottom){
 const Game*g=&c->game;int side=outcome_ship(g),duration=outcome_duration(g),age=c->result_ticks;
 if(g->outcome==GAME_SURRENDER){
  int carried=0;for(int i=GAME_MAX_ENEMY_CREW;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].health>0)carried++;
  rect(bottom,0,0,256,192,BG);panel(bottom,6,24,244,144,CYAN);
  text(bottom,16,40,"SURRENDER ACCEPTED",CYAN);text(bottom,16,65,"BOTH SHIPS HAVE CEASED FIRE",WHITE);
  text(bottom,16,89,"LIVING BOARDING PARTIES RETURNED",MUTED);
  text(bottom,16,115,carried?"EARLIER INTRUDERS REMAIN ABOARD":"BOTH HULLS REMAIN INTACT",carried?RED:GREEN);
  return;
 }

 if(side<0||duration<=0)return;
 /* Rebuild a clean scene: targeting, weapon cards and stale effects must not
  * obscure a hull result. Translate the ship band into the screen center. */
 rect(top,0,0,256,192,BG);
 player_art_use(g->player_ship);bitmap(top,4,82,player_art_combat(),COMBAT_KESTREL_WIDTH,COMBAT_KESTREL_HEIGHT);
 {const uint16_t*art;const ImportedEnemyArtRect*a;int hx,hy,hw,hh;if(enemy_art(g,&art,&a,&hx,&hy,&hw,&hh))bitmap(top,156,85,art,IMPORTED_ENEMY_ART_WIDTH,IMPORTED_ENEMY_ART_HEIGHT);
  else procedural_hull(top,157,86,90,60);}
 {char lost[40];snprintf(lost,sizeof lost,"%s LOST",ship_label(g));text(top,8,14,side?"ENEMY SHIP DESTROYED":lost,side?GOLD:RED);}
 text(top,8,174,g->paused?"PAUSED / X TO RESUME":"",MUTED);
 int x=side?156:3,y=82,w=side?94:111,h=70;
 if(age>duration/2)rect(top,x,y,w,h,BG);
 for(int i=0;i<OUTCOME_SAMPLE_COUNT;i++){
  int px,py,r,kind=outcome_sample(g,age,i,&px,&py,&r);if(!kind)continue;
  px=x+px*w/100;py=y+py*h/100;r=1+r*w/100;
  if(kind==OUTCOME_BLAST){ellipse(top,px,py,r,r*2/3,age%4<2?GOLD:RED);box(top,px-1,py-1,3,3,WHITE);}
  else{line(top,px,py,px+r,py+r/2,MUTED);rect(top,px,py,2,2,GOLD);}
 }
 rect(bottom,0,0,256,192,BG);panel(bottom,8,32,240,121,side?GOLD:RED);
 {char lost[40];snprintf(lost,sizeof lost,"%s DESTROYED",ship_label(g));text(bottom,20,49,side?"ENEMY HULL DESTROYED":lost,side?GOLD:RED);}
 text(bottom,20,73,g->paused?"PAUSED / X TO RESUME":side?"RECOVERING SALVAGE":"SIGNAL LOST",WHITE);
 text(bottom,20,95,"REPORT FOLLOWS AUTOMATICALLY",MUTED);
 if(!g->paused)text(bottom,20,110,"X / START: PAUSE",MUTED);
 bar(bottom,20,126,215,age,duration,side?GOLD:RED);
}
#include "retreat_view.inc"
#include "reward_view.inc"
#include "combat_controls.inc"
#include "campaign_view.inc"


#include "menu_view.inc"

#include "setup_view.inc"
#include "hangar_view.inc"
#include "tutorial_view.inc"
