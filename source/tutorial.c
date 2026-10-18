/* Guided Kestrel tutorial (see tutorial.h). Steps follow the original FTL
 * tutorial order: ship intro, power, damage and repairs with crew, venting a
 * fire through an airlock, the pilot and FTL jump, the beacon map, then combat
 * (pause, shields, weapon power, targeting, missiles, defeating the enemy).
 * DS adaptations (all tutorial-local, documented where applied):
 * - Its own Game/Ui. Nothing here reads or writes the campaign or any save.
 * - Weapon slot 2 holds a Basic Laser (the original tutorial's "2 shot laser")
 *   and the scout gets level 2 shields (two layers), so the original combat
 *   lesson holds: the laser is stopped until a missile damages the shields.
 * - Out of combat the ship is serviced with game_service_step (as on the map).
 * - Safety net: the Kestrel's hull and crew cannot drop below a small floor and
 *   missiles never run out during the missile/defeat lessons, so every step
 *   stays completable. */
#include "tutorial.h"
#include "fire.h"
#include "subsystems.h"
#include "tutorial_text.h"
#include <stdio.h>
#include <string.h>

typedef struct { int text,kind,sim; const char *title; } TutorialStepInfo;
/* sim: 1 when the simulation runs during the step (READ steps always hold time). */
static const TutorialStepInfo steps[TS_COUNT]={
 {TT_SHIP_INTRO_1,TUT_READ,0,"THE KESTREL"},
 {TT_SHIP_INTRO_2,TUT_READ,0,"SYSTEMS"},
 {TT_SHIP_INTRO_3,TUT_READ,0,"SUBSYSTEMS"},
 {TT_SHIP_INTRO_5,TUT_READ,0,"HULL"},
 {TT_POWER_SYSTEM_1,TUT_READ,0,"POWER"},
 {TT_POWER_SYSTEM_2,TUT_ACTION,1,"POWER"},
 {TT_DAMAGE_SYSTEM_1,TUT_READ,0,"DAMAGE"},
 {TT_DAMAGE_SYSTEM_2,TUT_ACTION,1,"CREW"},
 {TT_DAMAGE_SYSTEM_3,TUT_ACTION,1,"CREW"},
 {TT_DAMAGE_SYSTEM_4,TUT_ACTION,1,"REPAIRS"},
 {TT_DAMAGE_SYSTEM_5,TUT_ACTION,1,"REPAIRS"},
 {TT_DAMAGE_SYSTEM_5_5,TUT_READ,0,"MANNING"},
 {TT_FIRE_1,TUT_ACTION,0,"DOORS"},
 {TT_FIRE_END,TUT_ACTION,1,"DOORS"},
 {TT_DAMAGE_SYSTEM_6,TUT_ACTION,1,"PILOT"},
 {TT_DAMAGE_SYSTEM_7,TUT_ACTION,1,"FTL JUMP"},
 {TT_STAR_MAP_1,TUT_ACTION,0,"BEACON MAP"},
 {TT_COMBAT_0,TUT_ACTION,1,"COMBAT"},
 {TT_SHIELDS,TUT_ACTION,1,"SHIELDS"},
 {TT_COMBAT_1,TUT_ACTION,1,"WEAPONS"},
 {TT_COMBAT_2,TUT_ACTION,1,"TARGETING"},
 {TT_COMBAT_3,TUT_READ,0,"SHIELDS"},
 {TT_COMBAT_4,TUT_ACTION,1,"MISSILES"},
 {TT_COMBAT_4_5,TUT_READ,0,"ENEMY SYSTEMS"},
 {TT_COMBAT_4_6,TUT_ACTION,1,"VICTORY"},
 {TT_DONE_1,TUT_READ,0,"COMPLETE"},
};
typedef char tutorial_text_matches[TUTORIAL_TEXT_COUNT==TS_COUNT&&TUTORIAL_TEXT_COLS==TUTORIAL_PROMPT_COLS&&TUTORIAL_TEXT_LINES==TUTORIAL_PROMPT_LINES?1:-1];
#define TUT_LASER 1   /* Weapon slot of the 2-shot laser. */
#define TUT_MISSILE 0 /* Weapon slot of the Artemis. */
#define TUT_BASIC_LASER 2
#define TUT_HULL_FLOOR 5
#define TUT_CREW_FLOOR 10
/* Beacon map (bottom-screen pixels): the Kestrel starts at beacon 0. */
static const int beacon_x[TUTORIAL_BEACONS]={40,96,106,92,164,172,222};
static const int beacon_y[TUTORIAL_BEACONS]={98,50,100,150,64,134,98};
static const unsigned char beacon_links[][2]={{0,1},{0,2},{0,3},{1,4},{2,4},{2,5},{3,5},{4,6},{5,6}};

/* ---- Small game queries ---- */
static int room_index(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return i;return -1;}
static int manned(const Game*g,int room){
 for(int i=0;i<g->crew_count;i++){const Crew*c=&g->crew[i];if(c->health>0&&c->ship==0&&c->room==room&&c->destination==room)return 1;}
 return 0;}
static int ship_fires(const Game*g,int except){int n=0;for(int i=0;i<g->player.room_count;i++)if(g->player.rooms[i].id!=except)n+=g->player.rooms[i].fires;return n;}
static int airlock_open(const Game*g,int room){
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];if((d->room_a<0)==(d->room_b<0))continue;
  int inside=d->room_a<0?d->room_b:d->room_a;if(d->open&&(room<0||inside==room))return 1;}
 return 0;}
static int airlock_door(const Game*g,int room){
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];if((d->room_a<0)!=(d->room_b<0)&&(d->room_a==room||d->room_b==room))return i;}
 return 0;}
static int player_shots_in_flight(const Game*g){for(int i=0;i<GAME_MAX_EFFECTS;i++)if(g->effects[i].active&&g->effects[i].side==0)return 1;return 0;}
static int engines_full(const Game*g){return game_system_power(&g->player,SYS_ENGINES)>=g->player.systems[SYS_ENGINES].capacity;}
static int weapon_ready(const Game*g,int w){const WeaponState*s=&g->weapons[w];
 return game_weapon_powered(g,w)&&game_weapons_manned(g,0)&&s->charge_ticks>=s->cooldown_ticks&&!s->pending_shots&&g->missiles>=s->missiles;}
static int sel_weapon(const Tutorial*t){return t->ui.weapon>=0&&t->ui.weapon<t->game.weapon_count?t->ui.weapon:0;}
static void note(Tutorial*t,const char*s){snprintf(t->note,sizeof t->note,"%.*s",(int)sizeof t->note-1,s);t->note_ticks=2*GAME_HZ;}

/* Same layout rule as geometry() in render.c (bottom screen, non-compact). */
static void tut_geometry(const Ship*s,int*ox,int*oy,int*scale,int enemy){
 int mx=1000,my=1000,xx=-1000,yy=-1000;
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];if(r->x<mx)mx=r->x;if(r->y<my)my=r->y;if(r->x+r->w>xx)xx=r->x+r->w;if(r->y+r->h>yy)yy=r->y+r->h;}
 int height=enemy?64:84;
 int t=enemy?24:14;if(xx>mx&&(xx-mx)*t>232)t=232/(xx-mx);if(yy>my&&(yy-my)*t>height)t=height/(yy-my);if(t<1)t=1;
 *scale=t;*ox=(256-(xx-mx)*t)/2-mx*t;*oy=(enemy?44:25)+(height-(yy-my)*t)/2-my*t;
}
int tutorial_room_rect(const Tutorial*t,int side,int room,int*x,int*y,int*w,int*h){
 const Ship*s=side?&t->game.enemy:&t->game.player;int i=room_index(s,room),ox,oy,k;
 if(i<0||!s->room_count)return 0;
 tut_geometry(s,&ox,&oy,&k,side);
 *x=ox+s->rooms[i].x*k;*y=oy+s->rooms[i].y*k;*w=s->rooms[i].w*k;*h=s->rooms[i].h*k;return 1;
}
int tutorial_door_point(const Tutorial*t,int door,int*x,int*y){
 const Game*g=&t->game;int ox,oy,k;if(door<0||door>=g->door_count)return 0;
 const GameDoor*d=&g->doors[door];tut_geometry(&g->player,&ox,&oy,&k,0);
 *x=ox+d->x*k+(d->vertical?0:k/2);*y=oy+d->y*k+(d->vertical?k/2:0);return 1;
}
static int room_at(const Tutorial*t,int side,int x,int y){
 const Ship*s=side?&t->game.enemy:&t->game.player;
 for(int i=0;i<s->room_count;i++){int rx,ry,rw,rh;if(tutorial_room_rect(t,side,s->rooms[i].id,&rx,&ry,&rw,&rh)&&x>=rx&&x<rx+rw&&y>=ry&&y<ry+rh)return s->rooms[i].id;}
 return -1;}
/* D-pad room navigation, as combat_room() in combat_controls.inc. */
static int nav_room(const Ship*s,int current,int key){
 int fi=room_index(s,current);if(!s->room_count)return current;if(fi<0)return s->rooms[0].id;
 const GameRoom*from=&s->rooms[fi];int best=current,score=0x7fffffff,fx=2*from->x+from->w,fy=2*from->y+from->h;
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];int dx=2*r->x+r->w-fx,dy=2*r->y+r->h-fy;
  int forward=key==TUT_LEFT?-dx:key==TUT_RIGHT?dx:key==TUT_UP?-dy:dy;if(forward<=0)continue;
  int cross=key==TUT_LEFT||key==TUT_RIGHT?dy:dx;if(cross<0)cross=-cross;
  int rank=forward*forward+cross*cross*4;if(rank<score){score=rank;best=r->id;}}
 return best;}

/* ---- Public queries ---- */
int tutorial_step_count(void){return TS_COUNT;}
int tutorial_kind(const Tutorial*t){
 if(t->step==TS_PILOT&&!t->pilot_needed&&t->game.pilot_health>0&&manned(&t->game,t->game.pilot_room))return TUT_READ;
 return steps[t->step].kind;}
const char*tutorial_title(const Tutorial*t){return steps[t->step].title;}
const char*tutorial_text(const Tutorial*t){return tutorial_prompts[steps[t->step].text].text;}
const char*tutorial_text_id(int step){return step>=0&&step<TS_COUNT?tutorial_prompts[steps[step].text].id:"";}
int tutorial_can_jump(const Tutorial*t){const Game*g=&t->game;
 return !t->combat&&t->fuel>0&&game_system_power(&g->player,SYS_ENGINES)>0&&g->pilot_health>0&&manned(g,g->pilot_room);}
int tutorial_beacon_linked(int a,int b){
 for(unsigned i=0;i<sizeof beacon_links/sizeof beacon_links[0];i++)if((beacon_links[i][0]==a&&beacon_links[i][1]==b)||(beacon_links[i][0]==b&&beacon_links[i][1]==a))return 1;
 return 0;}
int tutorial_beacon(const Tutorial*t,int i,int*x,int*y){
 if(i<0||i>=TUTORIAL_BEACONS){return 0;}
 *x=beacon_x[i];*y=beacon_y[i];return tutorial_beacon_linked(t->beacon,i);}
int tutorial_step_complete(const Tutorial*t){const Game*g=&t->game;
 switch(t->step){
 case TS_POWER_ENGINES:case TS_REPOWER:return engines_full(g);
 case TS_SELECT_CREW:return !t->ui.crew_deselected&&t->ui.selected_crew>=0&&t->ui.selected_crew<g->crew_count&&g->crew[t->ui.selected_crew].health>0;
 case TS_SEND_CREW:{int room=g->player.systems[SYS_ENGINES].room;for(int i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==0&&g->crew[i].destination==room)return 1;return 0;}
 case TS_REPAIR:return g->player.systems[SYS_ENGINES].health>=g->player.systems[SYS_ENGINES].capacity;
 case TS_FIRE_AIRLOCK:return airlock_open(g,t->fire_room);
 case TS_FIRE_OUT:return !ship_fires(g,-1)&&!airlock_open(g,-1);
 case TS_PILOT:return g->pilot_health>0&&manned(g,g->pilot_room);
 case TS_JUMP:case TS_STAR_MAP:return 0; /* Advanced by the jump itself. */
 case TS_PAUSE:return g->paused;
 case TS_SHIELDS:return game_system_power(&g->player,SYS_SHIELDS)>=g->player.systems[SYS_SHIELDS].capacity;
 case TS_WEAPON_POWER:return game_weapon_powered(g,TUT_LASER);
 case TS_TARGET:return t->laser_fired&&!g->weapons[TUT_LASER].pending_shots&&!player_shots_in_flight(g);
 case TS_MISSILE:return g->enemy.systems[SYS_SHIELDS].health<g->enemy.systems[SYS_SHIELDS].capacity;
 case TS_DEFEAT:return g->outcome!=GAME_PLAYING&&g->outcome!=GAME_DEFEAT;
 default:return 1;}
}
void tutorial_goal(const Tutorial*t,char*out,int n){
 const Game*g=&t->game;const Ui*u=&t->ui;int w=sel_weapon(t);
 if(n<=0)return;
 if(tutorial_kind(t)==TUT_READ){snprintf(out,(size_t)n,"%s",t->step==TS_DONE?"A OR CONTINUE: FINISH THE TUTORIAL":t->step==TS_PILOT?"PILOT MANNED. A OR NEXT TO CONTINUE":"A OR NEXT: CONTINUE   SELECT: SKIP");return;}
 if(t->combat&&g->outcome==GAME_PLAYING&&t->step>=TS_MISSILE&&!game_weapons_manned(g,0)){snprintf(out,(size_t)n,"SEND A CREWMEMBER TO WEAPONS");return;}
 switch(t->step){
 case TS_POWER_ENGINES:case TS_REPOWER:snprintf(out,(size_t)n,"ENGINES %d/%d: TAP + BY ENG OR START",game_system_power(&g->player,SYS_ENGINES),g->player.systems[SYS_ENGINES].capacity);break;
 case TS_SELECT_CREW:snprintf(out,(size_t)n,"TAP A CREW PORTRAIT OR PRESS L/R");break;
 case TS_SEND_CREW:snprintf(out,(size_t)n,"%s",u->crew_deselected?"SELECT A CREWMEMBER FIRST (L/R)":"TAP THE ENGINE ROOM (OR D-PAD + A)");break;
 case TS_REPAIR:{int room=g->player.systems[SYS_ENGINES].room,going=0;for(int i=0;i<g->crew_count;i++)going|=g->crew[i].health>0&&g->crew[i].destination==room;
  if(going)snprintf(out,(size_t)n,"REPAIRING ENGINES %d/%d...",g->player.systems[SYS_ENGINES].health,g->player.systems[SYS_ENGINES].capacity);
  else {snprintf(out,(size_t)n,"SEND CREW TO THE ENGINE ROOM");}break;}
 case TS_FIRE_AIRLOCK:snprintf(out,(size_t)n,"%s",u->doors?"TAP A GOLD AIRLOCK DOOR BY THE FIRE":"TAP DOORS (LOWER RIGHT) OR START x2");break;
 case TS_FIRE_OUT:
  if(ship_fires(g,t->fire_room))snprintf(out,(size_t)n,"SEND CREW TO FIGHT THE OTHER FIRE");
  else if(ship_fires(g,-1)&&!airlock_open(g,t->fire_room))snprintf(out,(size_t)n,"OPEN THE AIRLOCK AGAIN TO VENT THE FIRE");
  else if(ship_fires(g,-1))snprintf(out,(size_t)n,"VENTING: WAIT FOR THE FIRE TO DIE");
  else {snprintf(out,(size_t)n,"NOW CLOSE THE AIRLOCK (OR SEAL ALL)");}
  break;
 case TS_PILOT:snprintf(out,(size_t)n,"SEND A CREWMEMBER TO THE PILOT ROOM");break;
 case TS_JUMP:
  if(tutorial_can_jump(t))snprintf(out,(size_t)n,"TAP JUMP OR PRESS A   FUEL %d",t->fuel);
  else if(game_system_power(&g->player,SYS_ENGINES)<=0)snprintf(out,(size_t)n,"POWER YOUR ENGINES TO JUMP");
  else {snprintf(out,(size_t)n,"SEND A CREWMEMBER TO THE PILOT ROOM");}
  break;
 case TS_STAR_MAP:snprintf(out,(size_t)n,"%s",t->map_sel<0?"TAP A LINKED BEACON OR USE THE D-PAD":"TAP JUMP OR PRESS A TO TRAVEL");break;
 case TS_PAUSE:snprintf(out,(size_t)n,"TAP PAUSE OR PRESS X");break;
 case TS_SHIELDS:snprintf(out,(size_t)n,"SHIELDS %d/%d: TAP + BY SHD OR START",game_system_power(&g->player,SYS_SHIELDS),g->player.systems[SYS_SHIELDS].capacity);break;
 case TS_WEAPON_POWER:snprintf(out,(size_t)n,"%s",!u->aiming?"TAP AIM OR PRESS Y":w!=TUT_LASER?"CHOOSE W2 (TAP IT OR L/R)":"TAP POWER ON OR PRESS START");break;
 case TS_TARGET:
  if(t->laser_fired)snprintf(out,(size_t)n,"WATCH YOUR SHOTS...");
  else if(!game_weapons_manned(g,0))snprintf(out,(size_t)n,"SEND A CREWMEMBER TO WEAPONS");
  else if(t->armed[TUT_LASER])snprintf(out,(size_t)n,"%s",g->paused?"UNPAUSE: TAP GO OR PRESS X":"LASER FIRES WHEN CHARGED...");
  else if(!u->aiming)snprintf(out,(size_t)n,"TAP AIM OR PRESS Y");
  else if(w!=TUT_LASER)snprintf(out,(size_t)n,"CHOOSE W2 (TAP IT OR L/R)");
  else {snprintf(out,(size_t)n,"TAP AN ENEMY ROOM, THEN FIRE (A)");}
  break;
 case TS_MISSILE:
  if(t->armed[TUT_MISSILE])snprintf(out,(size_t)n,"%s",g->paused?"UNPAUSE: TAP GO OR PRESS X":"MISSILE FIRES WHEN CHARGED...");
  else if(player_shots_in_flight(g))snprintf(out,(size_t)n,"WATCH YOUR SHOTS...");
  else if(!u->aiming)snprintf(out,(size_t)n,"TAP AIM OR PRESS Y");
  else if(w!=TUT_MISSILE)snprintf(out,(size_t)n,"CHOOSE W1 (TAP IT OR L/R)");
  else if(!game_weapon_powered(g,TUT_MISSILE))snprintf(out,(size_t)n,"TAP POWER ON OR PRESS START");
  else {snprintf(out,(size_t)n,"TARGET THE SHIELDS ROOM, THEN FIRE");}
  break;
 case TS_DEFEAT:
  if(g->paused)snprintf(out,(size_t)n,"PAUSED: TAP GO OR PRESS X");
  else {snprintf(out,(size_t)n,"ENEMY HULL %d: KEEP FIRING (A)",g->enemy.hull);}
  break;
 default:out[0]=0;break;}
}

/* ---- State changes ---- */
static void no_enemy(Game*g){
 memset(&g->enemy,0,sizeof g->enemy);memset(&g->enemy_weapon,0,sizeof g->enemy_weapon);memset(&g->enemy_weapon_secondary,0,sizeof g->enemy_weapon_secondary);g->enemy_weapon_count=0;
 memset(&g->enemy_drone,0,sizeof g->enemy_drone);g->enemy_drone.room=-1;
 memset(g->enemy_doors,0,sizeof g->enemy_doors);memset(g->enemy_adjacency,0,sizeof g->enemy_adjacency);g->enemy_door_count=0;
 memset(g->subsystems[1],0,sizeof g->subsystems[1]);memset(&g->teleporter[1],0,sizeof g->teleporter[1]);
 memset(g->enemy_crew,0,sizeof g->enemy_crew);g->enemy_crew_count=0;g->enemy_pilot_room=-1;g->enemy_pilot_health=0;
 memset(g->effects,0,sizeof g->effects);g->enemy_fire_ticks=g->enemy_pending_shots=g->enemy_shot_ticks=g->enemy_missiles=0;
 g->target_room=-1;g->enemy_departed=1;g->result_reason=RESULT_PLAYER_ESCAPE;g->outcome=GAME_PLAYING;g->paused=0;
}
static void enter(Tutorial*t){Game*g=&t->game;
 t->step_ticks=0;
 switch(t->step){
 case TS_DAMAGE:{GameSystemState*e=&g->player.systems[SYS_ENGINES];e->health=e->capacity>2?e->capacity-2:0;e->power=0;g->player.hull-=2;
  snprintf(g->message,sizeof g->message,"Engines damaged!");break;}
 case TS_SELECT_CREW:t->ui.crew_deselected=1;t->ui.group_mode=0;t->ui.crew_group=0;t->ui.doors=0;t->ui.controller_focus=0;t->mode=TUT_MODE_CREW;break;
 case TS_FIRE_AIRLOCK:fire_ignite(&g->player,t->fire_room,2);snprintf(g->message,sizeof g->message,"Fire in the starboard airlock!");break;
 case TS_PILOT:t->pilot_needed=!(g->pilot_health>0&&manned(g,g->pilot_room));break;
 case TS_STAR_MAP:t->map_sel=-1;t->ui.aiming=0;t->ui.doors=0;t->mode=TUT_MODE_CREW;break;
 case TS_TARGET:t->laser_fired=0;break;
 case TS_MISSILE:if(g->missiles<1)g->missiles=1;break;
 case TS_DONE:t->ui.aiming=0;t->ui.doors=0;t->mode=TUT_MODE_CREW;memset(t->armed,0,sizeof t->armed);break;
 default:break;}
}
static void advance(Tutorial*t){if(t->step<TS_DONE){t->step++;enter(t);}}
static int finish(Tutorial*t,int skipped){t->active=0;t->skipped=skipped;t->done=!skipped;return TUTORIAL_EXIT;}
static void check(Tutorial*t){
 while(t->active&&t->step<TS_DONE){
  if(t->step==TS_PILOT&&!tutorial_step_complete(t))t->pilot_needed=1;
  if(tutorial_kind(t)==TUT_ACTION&&tutorial_step_complete(t))advance(t);else break;}
}
static int next(Tutorial*t){if(t->step==TS_DONE)return finish(t,0);advance(t);check(t);return TUTORIAL_NONE;}
/* Hull, crew and missiles floors keep every lesson completable. */
static void protect(Tutorial*t){Game*g=&t->game;
 if(g->player.hull<TUT_HULL_FLOOR)g->player.hull=TUT_HULL_FLOOR;
 for(int i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].health<TUT_CREW_FLOOR)g->crew[i].health=TUT_CREW_FLOOR;
 if(t->step>=TS_MISSILE&&g->missiles<1)g->missiles=1;
}
static void start_combat(Tutorial*t){Game*g=&t->game;
 game_begin_encounter(g,ENEMY_SCOUT,0,0);
 /* Tutorial enemy: level 2 shields (two layers) in the scout's first empty room. */
 for(int i=0;i<g->enemy.room_count;i++)if(g->enemy.rooms[i].system<0){GameSystemState*s=&g->enemy.systems[SYS_SHIELDS];
  s->room=g->enemy.rooms[i].id;s->capacity=s->health=s->power=4;g->enemy.rooms[i].system=SYS_SHIELDS;g->enemy.shields=2;g->enemy.shield_ticks=0;break;}
 /* The lessons power shields and weapons, so both start unpowered. */
 g->player.systems[SYS_SHIELDS].power=0;g->player.shields=0;g->player.shield_ticks=0;
 for(int w=0;w<g->weapon_count;w++){g->weapons[w].enabled=0;g->weapons[w].charge_ticks=0;g->autofire[w]=0;t->armed[w]=0;}
 t->ui.aiming=0;t->ui.doors=0;t->mode=TUT_MODE_CREW;t->ui.weapon=TUT_LASER;t->combat=1;g->paused=0;
 snprintf(g->message,sizeof g->message,"Enemy ship ahead!");
}
static void jump(Tutorial*t){Game*g=&t->game;
 if(t->step!=TS_JUMP)return;
 if(!tutorial_can_jump(t)){note(t,game_system_power(&g->player,SYS_ENGINES)<=0?"FTL needs powered engines.":"FTL needs a crewmember piloting.");return;}
 advance(t);
}
static void map_jump(Tutorial*t){
 if(t->map_sel<0||!tutorial_beacon_linked(t->beacon,t->map_sel)){note(t,"Select an adjacent beacon first.");return;}
 t->fuel--;t->beacon=t->map_sel;t->map_sel=-1;start_combat(t);advance(t);
}
static void select_crew(Tutorial*t,int i){Game*g=&t->game;Ui*u=&t->ui;
 if(i<0||i>=g->crew_count)return;
 u->selected_crew=i;u->crew_deselected=0;u->group_mode=0;u->crew_page=i/3;u->focus_room=g->crew[i].room;
 note(t,g->crew[i].health>0?"Crew selected. Choose a room.":"This crew member is dead.");
}
static void select_weapon(Tutorial*t,int i){if(i<0||i>=t->game.weapon_count)return;t->ui.weapon=i;note(t,i==TUT_MISSILE?"W1 Artemis selected.":"W2 laser selected.");}
static void fire_weapon(Tutorial*t,int w){Game*g=&t->game;
 if(!t->combat){note(t,"No enemy ship to fire at.");return;}
 if(g->outcome!=GAME_PLAYING||w<0||w>=g->weapon_count)return;
 if(!game_weapon_powered(g,w)){note(t,"Weapon needs power: POWER ON.");return;}
 int room=g->weapons[w].aim_room>=0?g->weapons[w].aim_room:g->target_room;
 if(room_index(&g->enemy,room)<0){note(t,"Target an enemy room first.");return;}
 if(weapon_ready(g,w)&&game_fire(g,w,room)){t->armed[w]=0;if(w==TUT_LASER)t->laser_fired=1;note(t,"Firing!");return;}
 game_set_weapon_target(g,w,room);t->armed[w]=1;
 note(t,game_weapons_manned(g,0)?"Armed: fires when charged.":"Armed, but WEAPONS is unmanned.");
}
static void fire_armed(Tutorial*t){Game*g=&t->game;
 for(int w=0;w<g->weapon_count;w++){if(!t->armed[w]||!weapon_ready(g,w))continue;
  int room=g->weapons[w].aim_room>=0?g->weapons[w].aim_room:g->target_room;
  if(game_fire(g,w,room)){t->armed[w]=0;if(w==TUT_LASER)t->laser_fired=1;}}
}
static void door_mode(Tutorial*t,int on){t->ui.doors=on;t->mode=on?TUT_MODE_DOORS:TUT_MODE_CREW;if(on)t->door_sel=airlock_door(&t->game,t->fire_room);}
static void toggle_aim(Tutorial*t){
 if(!t->combat){note(t,"Aiming is available in combat.");return;}
 t->ui.aiming=!t->ui.aiming;t->ui.doors=0;t->mode=TUT_MODE_CREW;
 note(t,t->ui.aiming?"Targeting: choose weapon and room.":"Crew controls.");
}
static void toggle_pause(Tutorial*t){game_toggle_pause(&t->game);note(t,t->game.paused?"Paused. Orders still work.":"Resumed.");}

/* ---- Input ---- */
static void map_key(Tutorial*t,int key){
 int linked[TUTORIAL_BEACONS],n=0,at=-1;
 if(key==TUT_A){map_jump(t);return;}
 for(int i=0;i<TUTORIAL_BEACONS;i++)if(tutorial_beacon_linked(t->beacon,i)){if(i==t->map_sel)at=n;linked[n++]=i;}
 if(!n)return;
 if(key==TUT_LEFT||key==TUT_UP||key==TUT_L)t->map_sel=linked[at<0?n-1:(at+n-1)%n];
 else if(key==TUT_RIGHT||key==TUT_DOWN||key==TUT_R)t->map_sel=linked[at<0?0:(at+1)%n];
}
static void action_key(Tutorial*t,int key){Game*g=&t->game;Ui*u=&t->ui;
 if(key==TUT_X){toggle_pause(t);return;}
 if(key==TUT_Y){toggle_aim(t);return;}
 if(u->aiming){int w=sel_weapon(t),n=g->weapon_count;
  if(key==TUT_B){u->aiming=0;return;}
  if(!n)return;
  if(key==TUT_L||key==TUT_R)select_weapon(t,(w+n+(key==TUT_L?-1:1))%n);
  else if(key==TUT_START){game_toggle_weapon(g,w);note(t,g->message);}
  else if(key==TUT_A)fire_weapon(t,w);
  else if(key<=TUT_RIGHT&&g->enemy.room_count){int room=nav_room(&g->enemy,g->target_room,key);game_set_weapon_target(g,w,room);g->target_room=room;}
  return;}
 if(key==TUT_A&&t->step==TS_JUMP){jump(t);return;}  /* A is JUMP in every key mode. */
 if(key==TUT_START){int mode=(t->mode+1)%3;door_mode(t,mode==TUT_MODE_DOORS);t->mode=mode;
  note(t,mode==TUT_MODE_POWER?"Power: L/R system, UP/DOWN power.":mode==TUT_MODE_DOORS?"Doors: L/R door, A open/close.":"Crew: L/R crew, D-PAD room, A go.");return;}
 if(key==TUT_B){if(t->mode!=TUT_MODE_CREW)door_mode(t,0);return;}
 if(t->mode==TUT_MODE_POWER){
  if(key==TUT_LEFT||key==TUT_L)t->sys_sel=(t->sys_sel+GAME_SYSTEM_COUNT-1)%GAME_SYSTEM_COUNT;
  else if(key==TUT_RIGHT||key==TUT_R)t->sys_sel=(t->sys_sel+1)%GAME_SYSTEM_COUNT;
  else if(key==TUT_UP||key==TUT_A||key==TUT_DOWN){game_adjust_power(g,t->sys_sel,key==TUT_DOWN?-1:1);note(t,g->message);}
  return;}
 if(t->mode==TUT_MODE_DOORS){if(!g->door_count)return;
  if(key==TUT_LEFT||key==TUT_UP||key==TUT_L)t->door_sel=(t->door_sel+g->door_count-1)%g->door_count;
  else if(key==TUT_RIGHT||key==TUT_DOWN||key==TUT_R)t->door_sel=(t->door_sel+1)%g->door_count;
  else if(key==TUT_A){game_toggle_door(g,t->door_sel);note(t,g->message);}
  return;}
 if(key==TUT_L||key==TUT_R){int n=g->crew_count,i=u->selected_crew;if(!n)return;
  for(int tries=0;tries<n;tries++){i=(i+n+(key==TUT_L?-1:1))%n;if(g->crew[i].health>0)break;}
  if(u->crew_deselected&&g->crew[u->selected_crew].health>0)i=u->selected_crew;
  select_crew(t,i);u->controller_focus=1;return;}
 if(key<=TUT_RIGHT||key==TUT_A){
  if(u->crew_deselected){note(t,"Select crew with L/R first.");return;}
  if(!u->controller_focus){u->focus_room=g->crew[u->selected_crew].room;u->controller_focus=1;}
  if(key==TUT_A){int ok=game_order_crew(g,u->selected_crew,u->focus_room);note(t,ok?"Crew order set.":g->message);}
  else u->focus_room=nav_room(&g->player,u->focus_room,key);
 }
}
int tutorial_key(Tutorial*t,int key){
 if(!t||!t->active||key<0||key>=TUT_KEY_COUNT)return TUTORIAL_NONE;
 if(key==TUT_SELECT)return finish(t,1);
 if(tutorial_kind(t)==TUT_READ)return key==TUT_A?next(t):TUTORIAL_NONE;
 if(t->step==TS_STAR_MAP)map_key(t,key);else action_key(t,key);
 check(t);return TUTORIAL_NONE;
}
static void map_touch(Tutorial*t,int x,int y){
 if(y>=176){if(x<135)map_jump(t);return;}
 for(int i=0;i<TUTORIAL_BEACONS;i++){int dx=x-beacon_x[i],dy=y-beacon_y[i];if(dx*dx+dy*dy>100)continue;
  if(i==t->beacon)note(t,"Your ship is here.");
  else if(tutorial_beacon_linked(t->beacon,i)){t->map_sel=i;note(t,"Beacon selected. JUMP to travel.");}
  else note(t,"Not connected: pick an adjacent beacon.");
  return;}
}
static void action_touch(Tutorial*t,int x,int y){Game*g=&t->game;Ui*u=&t->ui;int w=sel_weapon(t);
 if(y>=176){if(x<51)toggle_pause(t);else if(x<135){if(t->step==TS_JUMP)jump(t);else fire_weapon(t,w);}else if(x>=189)toggle_aim(t);return;}
 if(u->aiming){
  if(y>=113&&y<132){select_weapon(t,x/64);return;}
  if(y>=149&&y<172&&g->weapon_count){if(x<128)game_toggle_weapon(g,w);else game_toggle_autofire(g,w);note(t,g->message);return;}
  if(y<39||y>=112)return;
  int room=room_at(t,1,x,y);if(room<0||!g->weapon_count)return;
  game_set_weapon_target(g,w,room);g->target_room=room;note(t,"Target set. Tap FIRE or press A.");return;}
 if(y>=158&&y<174){int sys=x/51;if(sys<GAME_SYSTEM_COUNT){t->sys_sel=sys;game_adjust_power(g,sys,x%51<27?-1:1);note(t,g->message);}return;}
 if(y>=135&&y<147){if(x>=177)door_mode(t,!u->doors);else note(t,"Not needed in the tutorial.");return;}
 if(u->doors&&y>=115&&y<134){if(x>=155){game_close_all_doors(g);note(t,g->message);}return;}
 if(u->doors&&y>=19&&y<115){int best=-1,distance=1000;
  for(int i=0;i<g->door_count;i++){int dx,dy;if(!tutorial_door_point(t,i,&dx,&dy))continue;dx-=x;dy-=y;if(dx*dx+dy*dy<=49&&dx*dx+dy*dy<distance){best=i;distance=dx*dx+dy*dy;}}
  if(best>=0){t->door_sel=best;game_toggle_door(g,best);note(t,g->message);}return;}
 if(y>=115&&y<134){if(x<4||x>=252||(x-4)%84>=80)return;select_crew(t,u->crew_page*3+(x-4)/84);u->controller_focus=0;return;}
 if(y>=19&&y<115){int room=room_at(t,0,x,y);if(room<0)return;
  if(u->crew_deselected){note(t,"Select a crew portrait first.");return;}
  u->focus_room=room;int ok=game_order_crew(g,u->selected_crew,room);note(t,ok?"Crew order set.":g->message);}
}
int tutorial_touch(Tutorial*t,int x,int y){
 if(!t||!t->active||x<0||x>=256||y<0||y>=192)return TUTORIAL_NONE;
 if(y>=176&&x>=136&&x<187)return finish(t,1);           /* SKIP, on every step. */
 if(tutorial_kind(t)==TUT_READ)return y>=176&&x<135?next(t):TUTORIAL_NONE;
 if(t->step==TS_STAR_MAP)map_touch(t,x,y);else action_touch(t,x,y);
 check(t);return TUTORIAL_NONE;
}

/* ---- Lifecycle ---- */
void tutorial_begin(Tutorial*t){Game*g=&t->game;int best=-1,low=-1;
 memset(t,0,sizeof *t);
 game_init_ship(g,0);                       /* Kestrel A. */
 game_equip(g,TUT_LASER,TUT_BASIC_LASER);   /* The original tutorial's 2-shot laser. */
 /* Crew at the helm, shields and weapons, so the engine repair needs an order. */
 {int shields=g->player.systems[SYS_SHIELDS].room;if(g->crew_count>1&&shields>=0){Crew*c=&g->crew[1];c->room=c->destination=c->station=shields;}}
 g->player.systems[SYS_ENGINES].power=0;    /* "Your engines are currently powered down." */
 no_enemy(g);
 /* The starboard airlock: the lowest room (largest y) with an exterior door. */
 for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];if((d->room_a<0)==(d->room_b<0))continue;
  int k=room_index(&g->player,d->room_a<0?d->room_b:d->room_a);if(k>=0&&g->player.rooms[k].y+g->player.rooms[k].h>low){low=g->player.rooms[k].y+g->player.rooms[k].h;best=g->player.rooms[k].id;}}
 t->fire_room=best;
 t->ui.crew_deselected=1;t->ui.weapon=TUT_LASER;t->ui.focus_room=g->player.systems[SYS_ENGINES].room;
 t->active=1;t->fuel=TUTORIAL_START_FUEL;t->map_sel=-1;t->sys_sel=SYS_SHIELDS;
 snprintf(g->message,sizeof g->message,"Tutorial: welcome aboard the Kestrel.");
 enter(t);
}
void tutorial_tick(Tutorial*t){Game*g;
 if(!t||!t->active)return;
 g=&t->game;t->ticks++;t->step_ticks++;if(t->note_ticks>0)t->note_ticks--;
 if(steps[t->step].sim&&tutorial_kind(t)==TUT_ACTION){
  protect(t);
  if(t->combat){if(!g->paused&&g->outcome==GAME_PLAYING){fire_armed(t);game_step(g);if(g->weapons[TUT_LASER].pending_shots>0)t->laser_fired=1;}}
  else game_service_step(g);
  protect(t);
 }
 check(t);
}
