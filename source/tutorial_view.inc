/* Tutorial view in the V54 original-style frames (included at the end of
 * render.c). Top: the normal combat top screen with the instruction panel over
 * its lower part (step counter, topic, the imported prompt, the live goal or
 * last feedback). Bottom: the real combat controls (render_game) with SKIP in
 * the DRONES slot, NEXT on read-only steps, a blinking marker on the control
 * the step asks for and the key-mode cursor; the beacon-map step draws a small
 * map instead. Rendering never changes tutorial state. */
#include "tutorial.h"
#define TUT_PANEL_Y 115
/* Word wrap honouring '\n'; mirrors wrap() in tools/import_tutorial.py. */
static int tut_wrap(uint16_t*p,int x,int y,int cols,int max_lines,const char*s,int c){
 char line[48];int lines=0;
 if(cols>(int)sizeof line-1)cols=(int)sizeof line-1;
 while(*s&&lines<max_lines){
  if(*s=='\n'){s++;continue;}
  int n=0;while(s[n]&&s[n]!='\n')n++;
  int cut=n<=cols?n:cols;
  if(n>cols){while(cut>0&&s[cut]!=' ')cut--;if(!cut)cut=cols;}
  snprintf(line,sizeof line,"%.*s",cut,s);text(p,x,y+lines*9,line,c);lines++;s+=cut;
  while(*s==' ')s++;
 }
 return lines;}
static void tut_mark(uint16_t*p,int x,int y,int w,int h,int tick){
 if((tick/8)%2)return;
 box(p,x-2,y-2,w+4,h+4,FTL_YELLOW);box(p,x-1,y-1,w+2,h+2,FTL_YELLOW);}
static int tut_airlock_point(const Tutorial*t,int*x,int*y){const Game*g=&t->game;
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];
  if((d->room_a<0)!=(d->room_b<0)&&(d->room_a==t->fire_room||d->room_b==t->fire_room)&&(t->step!=TS_FIRE_OUT||d->open))return tutorial_door_point(t,i,x,y);}
 return 0;}
/* The control the current step is waiting for (bottom-screen pixels). */
static int tut_target(const Tutorial*t,int*x,int*y,int*w,int*h){
 const Game*g=&t->game;const Ui*u=&t->ui;int wsel=u->weapon>=0&&u->weapon<g->weapon_count?u->weapon:0;
#define TUT_RECT(X,Y,W,H) do{*x=(X);*y=(Y);*w=(W);*h=(H);return 1;}while(0)
 if(tutorial_kind(t)==TUT_READ)return 0;
 switch(t->step){
 case TS_POWER_ENGINES:case TS_REPOWER:TUT_RECT(2+SYS_ENGINES*51+25,158,23,15);
 case TS_SHIELDS:TUT_RECT(2+SYS_SHIELDS*51+25,158,23,15);
 case TS_SELECT_CREW:TUT_RECT(4,115,248,19);
 case TS_SEND_CREW:case TS_REPAIR:
  if(u->crew_deselected)TUT_RECT(4,115,248,19);
  return tutorial_room_rect(t,0,g->player.systems[SYS_ENGINES].room,x,y,w,h);
 case TS_FIRE_AIRLOCK:case TS_FIRE_OUT:{int fires=0;for(int i=0;i<g->player.room_count;i++)fires+=g->player.rooms[i].fires;
  if(t->step==TS_FIRE_OUT&&fires)return 0;
  if(!u->doors)TUT_RECT(177,135,76,12);
  if(t->step==TS_FIRE_OUT)TUT_RECT(155,115,97,19);
  if(tut_airlock_point(t,x,y)){*x-=4;*y-=4;*w=*h=9;return 1;}
  return 0;}
 case TS_PILOT:if(u->crew_deselected)TUT_RECT(4,115,248,19);
  return tutorial_room_rect(t,0,g->pilot_room,x,y,w,h);
 case TS_JUMP:TUT_RECT(53,176,81,16);
 case TS_PAUSE:TUT_RECT(2,176,48,16);
 case TS_WEAPON_POWER:case TS_TARGET:case TS_MISSILE:case TS_DEFEAT:{int want=t->step==TS_MISSILE?0:1;
  if(t->step==TS_TARGET&&t->laser_fired)return 0;
  if(g->paused&&t->step!=TS_WEAPON_POWER&&(t->armed[want]||t->step==TS_DEFEAT))TUT_RECT(2,176,48,16);
  if(t->armed[want])return 0;
  if(!u->aiming)TUT_RECT(189,176,65,16);
  if(t->step==TS_DEFEAT)TUT_RECT(53,176,81,16);
  if(wsel!=want)TUT_RECT(2+want*64,113,60,18);
  if(!game_weapon_powered(g,want))TUT_RECT(3,149,122,22);
  if(t->step==TS_MISSILE&&g->weapons[0].aim_room!=g->enemy.systems[SYS_SHIELDS].room)return tutorial_room_rect(t,1,g->enemy.systems[SYS_SHIELDS].room,x,y,w,h);
  if(t->step==TS_WEAPON_POWER)return 0;
  TUT_RECT(53,176,81,16);}
 default:return 0;}
#undef TUT_RECT
}
static void tut_panel(const Tutorial*t,uint16_t*top){
 char s[64],goal[64];const char*title=tutorial_title(t);
 panel(top,2,TUT_PANEL_Y,252,192-TUT_PANEL_Y,GOLD);
 snprintf(s,sizeof s,"TUTORIAL %d/%d",t->step+1,TS_COUNT);text(top,9,TUT_PANEL_Y+3,s,MUTED);
 text(top,247-(int)strlen(title)*6,TUT_PANEL_Y+3,title,FTL_YELLOW);
 tut_wrap(top,9,TUT_PANEL_Y+12,TUTORIAL_PROMPT_COLS,TUTORIAL_PROMPT_LINES,tutorial_text(t),WHITE);
 rect(top,8,TUT_PANEL_Y+64,240,1,EDGE);
 if(t->note_ticks>0){snprintf(goal,sizeof goal,"%.40s",t->note);text(top,9,TUT_PANEL_Y+67,goal,CYAN);}
 else{tutorial_goal(t,s,sizeof s);snprintf(goal,sizeof goal,"%.40s",s);text(top,9,TUT_PANEL_Y+67,goal,GOLD);}
}
static void tut_map(const Tutorial*t,uint16_t*b){
 char s[32];int x,y,x2,y2;
 space_background(b);
 panel(b,0,0,256,19,WHITE);text(b,5,6,"BEACON MAP",FTL_YELLOW);
 snprintf(s,sizeof s,"FUEL %d",t->fuel);text(b,251-(int)strlen(s)*6,6,s,GOLD);
 panel(b,4,22,248,150,FTL_MINT);
 for(int i=0;i<TUTORIAL_BEACONS;i++)for(int j=i+1;j<TUTORIAL_BEACONS;j++)if(tutorial_beacon_linked(i,j)){
  tutorial_beacon(t,i,&x,&y);tutorial_beacon(t,j,&x2,&y2);line(b,x,y,x2,y2,i==t->beacon||j==t->beacon?CYAN:EDGE);}
 for(int i=0;i<TUTORIAL_BEACONS;i++){int linked=tutorial_beacon(t,i,&x,&y),c=i==t->beacon?GREEN:i==t->map_sel?GOLD:linked?CYAN:MUTED;
  rect(b,x-3,y-3,7,7,BG);ellipse(b,x,y,5,5,c);rect(b,x-1,y-1,3,3,c);
  if(i==t->beacon){rect(b,x-5,y-14,11,5,GREEN);rect(b,x+6,y-13,3,3,GREEN);text(b,x-8,y+9,"YOU",GREEN);}
  if(i==t->map_sel)box(b,x-8,y-8,17,17,FTL_YELLOW);
  else if(linked&&t->map_sel<0&&!((t->ticks/8)%2))box(b,x-8,y-8,17,17,FTL_YELLOW);}
 text(b,12,160,t->map_sel<0?"ADJACENT BEACONS ARE BLUE":"SELECTED: JUMP TO TRAVEL (1 FUEL)",WHITE);
 button(b,2,176,132,16,"JUMP / A",t->map_sel>=0?GREEN:MUTED);
 if(t->map_sel>=0)tut_mark(b,2,176,132,16,t->ticks);
 button(b,136,176,50,16,"SKIP",GOLD);
}
void render_tutorial(const Tutorial*t,uint16_t*top,uint16_t*bottom){
 const Game*g=&t->game;const Ui*u=&t->ui;int x,y,w,h,read=tutorial_kind(t)==TUT_READ;
 render_game(g,u,top,bottom);
 tut_panel(t,top);
 if(t->step==TS_STAR_MAP){tut_map(t,bottom);return;}
 /* Key-mode cursor: selected system (power) or door (doors). */
 if(!read&&!u->aiming&&t->mode==TUT_MODE_POWER){box(bottom,1+t->sys_sel*51,145,50,29,FTL_YELLOW);rect(bottom,0,135,176,12,BG);text(bottom,4,137,"POWER: L/R SYSTEM, UP/DOWN",FTL_YELLOW);}
 if(!read&&!u->aiming&&u->doors&&tutorial_door_point(t,t->door_sel,&x,&y))box(bottom,x-5,y-5,11,11,FTL_YELLOW);
 /* Bottom row: DRONES becomes SKIP; read steps show NEXT. */
 if(read){rect(bottom,0,175,256,17,BG);button(bottom,2,176,132,16,t->step==TS_DONE?"CONTINUE / A":"NEXT / A",GREEN);tut_mark(bottom,2,176,132,16,t->ticks);}
 else if(t->step==TS_JUMP){rect(bottom,53,176,81,16,BG);button(bottom,53,176,81,16,"JUMP A",tutorial_can_jump(t)?GREEN:MUTED);}
 if(!read&&!t->combat){rect(bottom,189,176,65,16,BG);button(bottom,189,176,65,16,"AIM Y",MUTED);}
 rect(bottom,136,176,50,16,BG);button(bottom,136,176,50,16,"SKIP",GOLD);
 if(tut_target(t,&x,&y,&w,&h))tut_mark(bottom,x,y,w,h,t->ticks);
}
