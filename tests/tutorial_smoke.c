/* Tutorial logic smoke test: walks every step with keys and touch, proves each
 * step's completion condition, skipping from several points, and that the
 * campaign save is never touched. Links only the game core and tutorial.c:
 * neither render.c (so render_tutorial is not needed for the logic) nor
 * campaign.c/save.c (so the tutorial cannot call campaign_save/campaign_load). */
#include "tutorial.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Tutorial t;
static long file_size(const char*path){FILE*f=fopen(path,"rb");long n;if(!f)return -1;fseek(f,0,SEEK_END);n=ftell(f);fclose(f);return n;}
static void key(int k){assert(tutorial_key(&t,k)==TUTORIAL_NONE);}
static void tap(int x,int y){assert(tutorial_touch(&t,x,y)==TUTORIAL_NONE);}
static void tap_room(int side,int room){int x,y,w,h;assert(tutorial_room_rect(&t,side,room,&x,&y,&w,&h));tap(x+w/2,y+h/2);}
static void ticks(int n){for(int i=0;i<n;i++)tutorial_tick(&t);}
/* Ticks until the step changes; fails after the limit (seconds). */
static void until_step(int step,int seconds){for(int i=0;i<seconds*GAME_HZ&&t.step!=step;i++)tutorial_tick(&t);assert(t.step==step);}
static void read_step(int step,int by_touch){char goal[64];
 assert(t.active&&t.step==step&&tutorial_kind(&t)==TUT_READ&&*tutorial_text(&t)&&*tutorial_title(&t));
 tutorial_goal(&t,goal,sizeof goal);assert(*goal);
 Game before=t.game;ticks(GAME_HZ);assert(t.game.ticks==before.ticks); /* READ steps hold time. */
 key(TUT_B);key(TUT_X);assert(t.step==step&&!t.game.paused);         /* Other keys are ignored. */
 if(by_touch)tap(60,184);else key(TUT_A);
}
static int shots_in_flight(const Game*g){for(int i=0;i<GAME_MAX_EFFECTS;i++)if(g->effects[i].active&&g->effects[i].side==0)return 1;return 0;}
static int fires(const Game*g){int n=0;for(int i=0;i<g->player.room_count;i++)n+=g->player.rooms[i].fires;return n;}
int main(void){
 const char*saves[]={"/ftl-ds-campaign.sav","ftl-ds-campaign.sav","tests/ftl-ds-campaign.sav","tests/tutorial.sav"};
 long before[4];for(int i=0;i<4;i++)before[i]=file_size(saves[i]);
 char goal[64];
 assert(tutorial_step_count()==TS_COUNT);
 for(int s=0;s<TS_COUNT;s++)assert(!strncmp(tutorial_text_id(s),"tutorial_",9));
 assert(!strcmp(tutorial_text_id(TS_POWER_ENGINES),"tutorial_POWER_SYSTEM_2_PC")&&!strcmp(tutorial_text_id(TS_STAR_MAP),"tutorial_STAR_MAP_1_IPAD"));

 /* Skip at any time: SELECT on the first step, SKIP touch mid-way, in combat and on the map. */
 tutorial_begin(&t);assert(t.active&&t.step==TS_INTRO_SHIP);
 assert(tutorial_key(&t,TUT_SELECT)==TUTORIAL_EXIT&&!t.active&&t.skipped&&!t.done);
 assert(tutorial_key(&t,TUT_A)==TUTORIAL_NONE&&tutorial_touch(&t,60,184)==TUTORIAL_NONE); /* Inactive: ignored. */
 tutorial_begin(&t);key(TUT_A);key(TUT_A);assert(t.step==TS_INTRO_SUBSYSTEMS);
 assert(tutorial_touch(&t,150,184)==TUTORIAL_EXIT&&t.skipped);

 /* Full walk. */
 tutorial_begin(&t);Game*g=&t.game;
 int engines=g->player.systems[SYS_ENGINES].room,shields=g->player.systems[SYS_SHIELDS].room;
 assert(g->player_ship==0&&g->weapons[1].catalog_id==2&&g->weapons[0].catalog_id==0); /* Kestrel, Artemis + 2-shot laser. */
 assert(g->player.systems[SYS_ENGINES].power==0&&g->enemy_departed&&!g->enemy.hull&&t.fire_room>=0);
 read_step(TS_INTRO_SHIP,0);read_step(TS_INTRO_SYSTEMS,1);read_step(TS_INTRO_SUBSYSTEMS,0);read_step(TS_INTRO_HULL,1);read_step(TS_POWER_INFO,0);

 /* Power: tap + beside ENG twice (system 1, bars at y158-173). */
 assert(t.step==TS_POWER_ENGINES&&tutorial_kind(&t)==TUT_ACTION&&!tutorial_step_complete(&t));
 tap(51+30,165);assert(g->player.systems[SYS_ENGINES].power==1&&t.step==TS_POWER_ENGINES);
 tap(51+5,165);assert(g->player.systems[SYS_ENGINES].power==0); /* - depowers. */
 tap(51+30,165);tap(51+30,165);
 /* Damage: engines broken, hull -2. */
 assert(t.step==TS_DAMAGE&&g->player.systems[SYS_ENGINES].health==0&&g->player.systems[SYS_ENGINES].power==0&&g->player.hull==28);
 read_step(TS_DAMAGE,0);
 /* Select crew (touch the second portrait), send it to the engine room. */
 assert(t.step==TS_SELECT_CREW&&t.ui.crew_deselected);
 tap_room(0,engines);assert(t.step==TS_SELECT_CREW&&t.note_ticks); /* No crew selected yet. */
 tap(4+84+10,120);assert(t.ui.selected_crew==1&&t.step==TS_SEND_CREW&&g->crew[1].room==shields);
 tap_room(0,engines);assert(g->crew[1].destination==engines&&t.step==TS_REPAIR);
 /* Repairs happen in real time as the crewmember arrives. */
 until_step(TS_REPOWER,90);assert(g->player.systems[SYS_ENGINES].health==2&&g->crew[1].room==engines);
 /* Repower with keys: START (power mode), RIGHT to ENG, UP twice. */
 key(TUT_START);assert(t.mode==TUT_MODE_POWER&&t.sys_sel==SYS_ENGINES); /* Last touched system. */
 key(TUT_LEFT);assert(t.sys_sel==SYS_SHIELDS);key(TUT_RIGHT);assert(t.sys_sel==SYS_ENGINES);key(TUT_UP);key(TUT_UP);
 assert(game_system_power(&g->player,SYS_ENGINES)==2&&t.step==TS_MANNING);
 read_step(TS_MANNING,1);

 /* Fire in the starboard airlock: open DOORS, open an airlock door, wait, close it. */
 assert(t.step==TS_FIRE_AIRLOCK&&fires(g)==2);
 tap(200,140);assert(t.ui.doors&&t.mode==TUT_MODE_DOORS);
 {int door=-1,x,y;for(int i=0;i<g->door_count;i++){const GameDoor*d=&g->doors[i];if((d->room_a==t.fire_room&&d->room_b<0)||(d->room_b==t.fire_room&&d->room_a<0)){door=i;break;}}
  assert(door>=0&&tutorial_door_point(&t,door,&x,&y));tap(x,y);assert(g->doors[door].open);}
 assert(t.step==TS_FIRE_OUT);
 for(int i=0;i<60*GAME_HZ&&fires(g);i++){tutorial_tick(&t);
  if(i%GAME_HZ==0)for(int r=0;r<g->player.room_count;r++)if(g->player.rooms[r].fires&&g->player.rooms[r].id!=t.fire_room)game_order_crew(g,2,g->player.rooms[r].id);}
 assert(!fires(g)&&t.step==TS_FIRE_OUT);
 tutorial_goal(&t,goal,sizeof goal);assert(strstr(goal,"CLOSE"));
 tap(200,120);assert(t.step==TS_PILOT); /* SEAL ALL. */

 /* Pilot: still manned at the helm, so it reads; then jump. */
 assert(tutorial_kind(&t)==TUT_READ);read_step(TS_PILOT,0);
 assert(t.step==TS_JUMP&&tutorial_can_jump(&t));
 key(TUT_A);assert(t.step==TS_STAR_MAP&&t.fuel==TUTORIAL_START_FUEL);
 /* Beacon map: A without a selection does nothing, unlinked beacons refuse. */
 key(TUT_A);assert(t.step==TS_STAR_MAP);
 {int x,y;assert(!tutorial_beacon(&t,6,&x,&y));tap(x,y);assert(t.map_sel<0);
  key(TUT_RIGHT);assert(t.map_sel==1);key(TUT_RIGHT);assert(t.map_sel==2);
  assert(tutorial_beacon(&t,3,&x,&y));tap(x,y);assert(t.map_sel==3);}
 tap(60,184);assert(t.step==TS_PAUSE&&t.fuel==TUTORIAL_START_FUEL-1&&t.beacon==3&&t.combat);
 assert(g->outcome==GAME_PLAYING&&g->enemy.hull>0&&g->enemy.shields==2&&g->player.shields==0&&!game_weapon_powered(g,1));

 /* Combat: pause, shields, weapon power, target and fire. */
 key(TUT_X);assert(g->paused&&t.step==TS_SHIELDS);
 tap(30,165);tap(30,165);assert(game_system_power(&g->player,SYS_SHIELDS)==2&&t.step==TS_WEAPON_POWER);
 key(TUT_START);assert(!t.ui.aiming&&t.mode==TUT_MODE_POWER);key(TUT_B);  /* START outside aiming is power mode. */
 key(TUT_Y);assert(t.ui.aiming&&t.ui.weapon==1);
 tap(60,160);assert(game_weapon_powered(g,1)&&t.step==TS_TARGET);
 {int hull=g->enemy.hull;
  key(TUT_DOWN);key(TUT_RIGHT);assert(g->weapons[1].aim_room==g->target_room);
  tap(90,184);assert(t.armed[1]);                                  /* FIRE arms: fires when charged. */
  ticks(GAME_HZ);assert(!g->weapons[1].charge_ticks);              /* Still paused. */
  key(TUT_X);assert(!g->paused);
  until_step(TS_SHIELDS_BLOCK,30);
  assert(g->enemy.hull==hull);}                                    /* Two shots stopped by two shield layers. */
 read_step(TS_SHIELDS_BLOCK,1);

 /* Missile: W1, power it, target the shields room, fire. */
 assert(t.step==TS_MISSILE);
 key(TUT_L);assert(t.ui.weapon==0);key(TUT_START);assert(game_weapon_powered(g,0));
 for(int tries=0;tries<6&&t.step==TS_MISSILE;tries++){
  tap_room(1,g->enemy.systems[SYS_SHIELDS].room);assert(g->weapons[0].aim_room==g->enemy.systems[SYS_SHIELDS].room);
  key(TUT_A);assert(t.armed[0]||g->weapons[0].pending_shots);
  for(int i=0;i<30*GAME_HZ&&t.step==TS_MISSILE&&(t.armed[0]||g->weapons[0].pending_shots||shots_in_flight(g));i++)tutorial_tick(&t);
 }
 assert(t.step==TS_ENEMY_SYSTEMS&&g->enemy.systems[SYS_SHIELDS].health<4);
 read_step(TS_ENEMY_SYSTEMS,0);

 /* Defeat the enemy: keep both weapons firing (touch W buttons + FIRE). */
 assert(t.step==TS_DEFEAT);
 for(int i=0;i<300*GAME_HZ&&t.step==TS_DEFEAT;i++){
  if(i%GAME_HZ==0){tap(10,120);tap_room(1,g->enemy.systems[SYS_WEAPONS].room);tap(90,184);tap(70,120);tap(90,184);}
  tutorial_tick(&t);assert(g->player.hull>=5&&g->outcome!=GAME_DEFEAT);}
 assert(t.step==TS_DONE&&g->outcome==GAME_VICTORY&&g->enemy.hull<=0);

 /* Finish. */
 assert(tutorial_kind(&t)==TUT_READ&&tutorial_touch(&t,60,184)==TUTORIAL_EXIT&&!t.active&&t.done&&!t.skipped);

 /* Skip from the beacon map and from combat by touch and SELECT. */
 tutorial_begin(&t);t.step=TS_JUMP;assert(tutorial_touch(&t,140,180)==TUTORIAL_EXIT&&t.skipped);
 tutorial_begin(&t);t.step=TS_STAR_MAP;assert(tutorial_touch(&t,186,191)==TUTORIAL_EXIT);
 tutorial_begin(&t);t.step=TS_DEFEAT;assert(tutorial_key(&t,TUT_SELECT)==TUTORIAL_EXIT);

 /* The pilot step becomes an action when nobody pilots. */
 tutorial_begin(&t);g=&t.game;g->player.systems[SYS_ENGINES].power=2;
 g->crew[0].room=g->crew[0].destination=1;t.step=TS_PILOT;tutorial_tick(&t);
 assert(t.step==TS_PILOT&&t.pilot_needed&&tutorial_kind(&t)==TUT_ACTION&&!tutorial_can_jump(&t));
 key(TUT_A);assert(t.step==TS_PILOT);                                        /* A is not NEXT here. */
 tap(4+10,120);assert(t.ui.selected_crew==0);tap_room(0,g->pilot_room);until_step(TS_JUMP,30);
 assert(tutorial_can_jump(&t));tap(60,184);assert(t.step==TS_STAR_MAP);

 for(int i=0;i<4;i++)assert(file_size(saves[i])==before[i]); /* No save created or changed. */
 printf("PASS: tutorial %d steps (power, crew, repairs, doors, pilot, FTL jump, beacon map, pause, shields, weapons, targeting, missiles, victory), keys+touch, skip anywhere, no campaign save.\n",TS_COUNT);
 return 0;
}
