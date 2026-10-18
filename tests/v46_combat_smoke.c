/* V46: weapons need a manned weapons room (house rule), enemy and flagship
 * gunners keep firing, manned sensors bonus, door-only crew walking, in-room
 * fire growth, and the bottom-screen icon / UNMANNED presentation. */
#include "render.h"
#include "fire.h"
#include "subsystems.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static uint16_t top[256*192],bottom[256*192],top2[256*192],bottom2[256*192];
static int incoming(void){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].age==0;return n;}
static int outgoing(void){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==0;return n;}
static const GameRoom*room(const Ship*s,int id){for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return 0;}
static int inside(const GameRoom*r,int x,int y,int t){return x>=r->x*t&&x<=(r->x+r->w)*t&&y>=r->y*t&&y<=(r->y+r->h)*t;}
int main(void){
 audio_init();
 /* Player: a charged weapon will not fire until someone mans the weapons room. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 int wroom=g.player.systems[SYS_WEAPONS].room,gunner=-1;
 for(int i=0;i<g.crew_count;i++)if(g.crew[i].room==wroom)gunner=i;
 assert(gunner>=0&&game_weapons_manned(&g,0));
 g.crew[gunner].room=g.crew[gunner].destination=g.pilot_room==0?1:0;
 assert(!game_weapons_manned(&g,0));
 g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
 assert(!game_fire(&g,0,g.enemy.systems[SYS_SHIELDS].room)&&strstr(g.message,"unmanned"));
 g.autofire[0]=1;for(int t=0;t<5;t++)game_step(&g);assert(!g.weapons[0].pending_shots&&!outgoing());
 {Ui u={0};render_game(&g,&u,top,bottom);g.crew[gunner].room=g.crew[gunner].destination=wroom;render_game(&g,&u,top2,bottom2);
  assert(memcmp(top+150*256,top2+150*256,10*256*sizeof(uint16_t))); /* UNMANNED status on the weapon panel. */}
 assert(game_weapons_manned(&g,0));game_step(&g);assert(g.weapons[0].pending_shots||outgoing());
 /* A crewmember still walking to the room does not man it. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 for(int i=0;i<g.crew_count;i++)if(g.crew[i].room==wroom){g.crew[i].room=g.crew[i].destination=0;assert(game_order_crew(&g,i,wroom));}
 assert(!game_weapons_manned(&g,0));
 /* Enemy: crewed ships fire only while their gunner stands in the weapons room. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 for(int i=0;i<g.weapon_count;i++){g.weapons[i].enabled=0;}g.player.hull=g.player.max_hull=1000;
 {int shots=0;for(int t=0;t<60*GAME_HZ;t++){game_step(&g);shots+=incoming();}assert(shots>0&&game_weapons_manned(&g,1));}
 for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(g.enemy_crew[i].ship==1&&g.enemy_crew[i].health>0&&g.enemy_crew[i].room!=g.enemy_pilot_room)g.enemy_crew[i].health=0;
 {int shots=0;for(int t=0;t<60*GAME_HZ;t++){game_step(&g);shots+=incoming();}assert(!shots&&!game_weapons_manned(&g,1));}
 /* Every enemy kind still fires at an idle player: automated ships count as manned. */
 for(int kind=0;kind<ENEMY_KIND_COUNT;kind++){game_init(&g);assert(game_begin_encounter(&g,kind,2,0));
  for(int i=0;i<g.weapon_count;i++){g.weapons[i].enabled=0;}g.player.hull=g.player.max_hull=1000;
  int shots=0;for(int t=0;t<45*GAME_HZ&&g.outcome==GAME_PLAYING;t++){game_step(&g);shots+=incoming();}
  assert(shots>0);if(g.enemy.automated)assert(game_weapons_manned(&g,1));}
 /* Flagship: every artillery room starts with its isolated gunner and fires; a dead gunner silences it. */
 for(int stage=0;stage<3;stage++){game_init(&g);assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,stage));
  for(int i=0;i<g.artillery_count;i++)assert(game_artillery_manned(&g,i));}
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,0));g.enemy_cloak.capacity=0;
 {int a=0;for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(g.enemy_crew[i].room==g.artillery[a].room)g.enemy_crew[i].health=0;
  assert(!game_artillery_manned(&g,a));g.artillery[a].charge=1<<20;for(int t=0;t<10;t++)game_step(&g);assert(!g.artillery[a].w.pending_shots&&!g.artillery[a].w.queued_target_count);
  g.enemy.automated=1;assert(game_artillery_manned(&g,a));}
 /* V49 (V47 behaviour): a dead gunner silences that sealed artillery room for good;
  * the other rooms keep firing, and an AI-run ship with no crew counts as manned. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,0));g.enemy_cloak.capacity=0;g.player.hull=g.player.max_hull=1000;
 {int a=0,room=g.artillery[a].room;for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(g.enemy_crew[i].room==room)g.enemy_crew[i].health=0;
  assert(game_next_room(&g,1,0,room)<0);
  for(int t=0;t<20*GAME_HZ;t++){game_step(&g);}
  assert(!game_artillery_manned(&g,a));for(int k=1;k<g.artillery_count;k++)assert(game_artillery_manned(&g,k));}
 /* Sensors: a manned working sensors room reads one level higher. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 {Subsystem*s=&g.subsystems[0][SUB_SENSORS];int sroom=s->room,enemy_room=g.enemy.rooms[0].id;assert(sroom>=0);
  for(int i=0;i<g.crew_count;i++)assert(g.crew[i].room!=sroom);
  s->capacity=s->health=2;assert(game_sensor_level(&g)==2&&game_room_visible(&g,1,enemy_room)&&!game_enemy_details_visible(&g));
  g.crew[0].room=g.crew[0].destination=sroom;assert(game_sensor_level(&g)==3&&game_enemy_details_visible(&g));
  s->health=1;assert(game_sensor_level(&g)==2&&game_room_visible(&g,1,enemy_room));
  s->health=0;assert(game_sensor_level(&g)==0&&!game_room_visible(&g,1,enemy_room));}
 /* Crew walk only through doors: every sampled sprite centre stays inside the two rooms. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0));
 {int pairs=0;for(int d=0;d<g.door_count;d++){const GameDoor*door=&g.doors[d];if(door->room_a<0||door->room_b<0)continue;
   for(int dir=0;dir<2;dir++){const GameRoom*a=room(&g.player,dir?door->room_b:door->room_a),*b=room(&g.player,dir?door->room_a:door->room_b);
    Crew*c=&g.crew[0];c->room=a->id;c->destination=b->id;assert(game_crew_next_room(&g,0)==b->id);
    for(int m=0;m<GAME_HZ;m++){int x,y;c->move_ticks=m;render_crew_path_point(&g,0,0,64,&x,&y);assert(inside(a,x+32,y+32,64)||inside(b,x+32,y+32,64));}
    pairs++;}}
  assert(pairs>=10);}
 /* A boarder facing a sealed level-2 door waits at that door instead of walking into the wall. */
 {g.enemy_crew_count=1;Crew*e=&g.enemy_crew[0];memset(e,0,sizeof *e);e->ship=0;e->health=100;e->id=1;
  const GameDoor*door=0;for(int d=0;d<g.door_count&&!door;d++)if(g.doors[d].room_a>=0&&g.doors[d].room_b>=0)door=&g.doors[d];
  e->room=door->room_a;e->destination=door->room_b;g.doors[door-g.doors].open=0;g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=2;
  assert(game_crew_blocked(&g,1,0));g.doors[door-g.doors].open=1;assert(!game_crew_blocked(&g,1,0));}
 /* Fire grows inside its room while it has air, and goes out in vacuum. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0));
 {GameRoom*r=0;for(int i=0;i<g.player.room_count;i++){GameRoom*q=&g.player.rooms[i];int crewed=0;for(int k=0;k<g.crew_count;k++)crewed|=g.crew[k].room==q->id;if(!crewed&&q->w*q->h==4){r=q;break;}}
  assert(r);fire_ignite(&g.player,r->id,1);int most=1;for(int t=0;t<20*GAME_HZ&&r->fires;t++){game_service_step(&g);if(r->fires>most)most=r->fires;}
  assert(most>1);g.player.systems[SYS_OXYGEN].power=0;for(int i=0;i<g.player.room_count;i++)g.player.rooms[i].oxygen=0;r->fires=2;for(int t=0;t<3*GAME_HZ;t++)game_service_step(&g);if(r->fires)fprintf(stderr,"fires%d o2 %d outcome%d paused%d\n",r->fires,r->oxygen,g.outcome,g.paused);assert(r->fires==0);}
 /* Room icons follow system state: damaging shields changes the home view. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_SCOUT,0,0));
 {Ui u={0};render_game(&g,&u,top,bottom);g.player.systems[SYS_SHIELDS].health=1;render_game(&g,&u,top2,bottom2);assert(memcmp(bottom,bottom2,sizeof bottom));
  g.player.systems[SYS_SHIELDS].health=g.player.systems[SYS_SHIELDS].capacity;g.player.systems[SYS_MEDBAY].power=1;render_game(&g,&u,top2,bottom2);assert(memcmp(bottom,bottom2,sizeof bottom));}
 /* V47: enemy damage colours are public at sensors level 1; ion state is not. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 {Ui u={.aiming=1};assert(!game_enemy_details_visible(&g));render_game(&g,&u,top,bottom);
  g.enemy.systems[SYS_SHIELDS].ion_ticks=90;g.enemy.systems[SYS_SHIELDS].ion_bars=1;render_game(&g,&u,top2,bottom2);assert(!memcmp(bottom,bottom2,sizeof bottom));
  g.enemy.systems[SYS_SHIELDS].ion_ticks=g.enemy.systems[SYS_SHIELDS].ion_bars=0;g.enemy.systems[SYS_SHIELDS].health=1;render_game(&g,&u,top2,bottom2);assert(memcmp(bottom,bottom2,sizeof bottom));}
 puts("PASS: manned-weapons rule (player, enemy, flagship, automated), sensors manning bonus, door-only crew paths, fire growth and icon states.");
 return 0;
}
