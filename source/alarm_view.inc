/* V68 hit and alarm cues (included by render.c; stateless, derived from Game each
 * frame). A resolved hull hit on the player shakes the tactical view and flashes it
 * red; burning rooms pulse; flashing warning chips use the original text_misc.xml
 * strings (warning_hull_25, warning_oxygen, warning_intruders, on_fire) and follow
 * the same causes as the audio alarms (audio_alarm_mask), limited to what sensors show. */
#define ALARM_HIT_TICKS 6
static int alarm_hull_hit_age(const Game*g){int best=-1;
 for(int i=0;i<GAME_MAX_EFFECTS;i++){const GameEffect*f=&g->effects[i];
  if(f->active&&f->side==1&&f->result==EFFECT_HIT&&f->damage>0&&f->age>=f->duration&&f->age-f->duration<ALARM_HIT_TICKS&&(best<0||f->age-f->duration<best))best=f->age-f->duration;}
 return best;
}
/* Bottom-screen player ship, drawn right after ship(): room pulse, then shake/flash. */
static void alarm_ship_view(uint16_t*p,const Game*g){
 int ox,oy,t,flash=(g->ticks/8)%2,age=alarm_hull_hit_age(g);geometry(&g->player,&ox,&oy,&t,0);
 for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];int x=ox+r->x*t,y=oy+r->y*t,w=r->w*t,h=r->h*t;
  if(r->fires<=0||!game_room_visible(g,0,r->id))continue;
  if(flash)tint(p,x+1,y+1,w-2,h-2,RED,5);
  box(p,x,y,w,h,flash?RED:GOLD);box(p,x+1,y+1,w-2,h-2,flash?RED:GOLD);
 }
 if(age<0)return;
 {int dx=age<4?(age%2?-2:2):0;
  if(dx)for(int j=20;j<114;j++){uint16_t*row=p+j*256;
   if(dx>0){memmove(row+dx,row,(size_t)(256-dx)*sizeof *row);for(int i=0;i<dx;i++)row[i]=BG;}
   else{memmove(row,row-dx,(size_t)(256+dx)*sizeof *row);for(int i=256+dx;i<256;i++)row[i]=BG;}}}
 tint(p,0,20,256,94,RED,age<3?4:2);box(p,0,19,256,95,RED);box(p,1,20,254,93,RED);
}
static int alarm_chip(uint16_t*p,int*x,int*y,const char*label,int icon,int color,int tick){
 int w=(int)strlen(label)*6+(icon?16:6);
 if(*x+w>252){*x=4;*y+=12;}
 rect(p,*x,*y,w,11,PANEL);box(p,*x,*y,w,11,color);
 if(icon==1)flame(p,*x+7,*y+6,tick);
 else if(icon==2)breach_marker(p,*x+7,*y+5,tick);
 text(p,*x+(icon?14:3),*y+2,label,color);
 *x+=w+2;return w;
}
/* Top-screen warning chips over the player ship, flashing like the original popups. */
static void alarm_warning_view(uint16_t*p,const Game*g){
 unsigned m=audio_alarm_mask(g),shown=0;
 /* A fresh hull hit also flashes the top-screen ship. */
 if(alarm_hull_hit_age(g)>=0){box(p,3,46,112,72,RED);box(p,4,47,110,70,RED);}
 if(!m)return;
 for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];if(!game_room_visible(g,0,r->id))continue;
  if(r->fires>0)shown|=1u<<ALARM_FIRE;
  if(r->breaches>0)shown|=1u<<ALARM_BREACH;}
 if(subsystem_level(g,0,SUB_SENSORS)>0)shown|=1u<<ALARM_OXYGEN;
 shown|=1u<<ALARM_HULL;
 for(int i=0;i<g->enemy_crew_count;i++){const Crew*c=&g->enemy_crew[i];if(c->ship==0&&c->health>0&&game_room_visible(g,0,c->room))shown|=1u<<ALARM_INTRUDER;}
 m&=shown;if(!m)return;
 {int flash=(g->ticks/8)%2,warn=flash?RED:WHITE,x=4,y=g->environment?51:43;
  if(m&(1u<<ALARM_HULL))alarm_chip(p,&x,&y,"HULL CRITICAL",0,warn,g->ticks);
  if(m&(1u<<ALARM_OXYGEN))alarm_chip(p,&x,&y,"O2 LOW!",0,warn,g->ticks);
  if(m&(1u<<ALARM_INTRUDER))alarm_chip(p,&x,&y,"INTRUDERS DETECTED",0,warn,g->ticks);
  if(m&(1u<<ALARM_FIRE))alarm_chip(p,&x,&y,"ON FIRE",1,flash?RED:GOLD,g->ticks);
  if(m&(1u<<ALARM_BREACH))alarm_chip(p,&x,&y,"BREACH",2,flash?CYAN:WHITE,g->ticks);}
}
