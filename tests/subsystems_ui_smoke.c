#include "render.h"
#include "subsystems.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} a,b;
static uint16_t expected_top[256*192],expected_bottom[256*192];
static void draw(Campaign*c,Ui*u){a.before=b.before=123;a.after=b.after=456;render_campaign(c,u,a.pixels,b.pixels);assert(a.before==123&&b.before==123&&a.after==456&&b.after==456);}
static void snapshot(Campaign*c,Ui*u){draw(c,u);memcpy(expected_top,a.pixels,sizeof expected_top);memcpy(expected_bottom,b.pixels,sizeof expected_bottom);}
static void equal(Campaign*c,Ui*u){draw(c,u);assert(!memcmp(expected_top,a.pixels,sizeof expected_top)&&!memcmp(expected_bottom,b.pixels,sizeof expected_bottom));}
static void enemy_changes(Game*g){for(int i=0;i<g->enemy.room_count;i++){g->enemy.rooms[i].oxygen=0;g->enemy.rooms[i].fires=1;g->enemy.rooms[i].breaches=1;}for(int i=0;i<g->enemy_crew_count;i++){g->enemy_crew[i].health=10;snprintf(g->enemy_crew[i].name,sizeof g->enemy_crew[i].name,"HIDDEN");}g->enemy_fire_ticks=90;} /* V47: enemy system damage is public (icon colour), so it is not a hidden change. */
int main(void){Campaign c,altered;Ui u={.weapon=1,.aiming=1};audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,1,0);
 assert(subsystem_level(&c.game,0,SUB_SENSORS)==1);altered=c;enemy_changes(&altered.game);
 snapshot(&c,&u);equal(&altered,&u); /* targets and minimaps never reveal hidden rooms */
 /* Sensor loss conceals conditions, not the known target system identity. */
 c.game.target_room=c.game.weapons[1].aim_room=c.game.enemy.systems[SYS_WEAPONS].room;
 snapshot(&c,&u);Campaign visible=c;visible.game.subsystems[0][SUB_SENSORS].capacity=visible.game.subsystems[0][SUB_SENSORS].health=2;draw(&visible,&u);
 assert(!memcmp(expected_top+51*256,a.pixels+51*256,8*256*sizeof(uint16_t)));
 Campaign relabeled=c;for(int i=0;i<relabeled.game.enemy.room_count;i++)if(relabeled.game.enemy.rooms[i].id==c.game.target_room)relabeled.game.enemy.rooms[i].system=SYS_ENGINES;
 draw(&relabeled,&u);assert(memcmp(expected_top+51*256,a.pixels+51*256,8*256*sizeof(uint16_t)));
 altered=c;enemy_changes(&altered.game);
 u.boarding=1;u.board_side=1;u.board_room=c.game.enemy.rooms[0].id;snapshot(&c,&u);equal(&altered,&u);
 u.boarding=0;u.logs=1;snapshot(&c,&u);equal(&altered,&u);u.logs=0;
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=2;altered=c;enemy_changes(&altered.game);snapshot(&c,&u);draw(&altered,&u);assert(memcmp(expected_bottom,b.pixels,sizeof expected_bottom));
 /* Enemy firing charge lives in the normal combat overview. Level2 hides
  * it and level3 reveals it; aiming uses that space for the target card. */
 u.aiming=0;
 altered=c;altered.game.enemy_fire_ticks=90;snapshot(&c,&u);equal(&altered,&u);
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;altered=c;altered.game.enemy_fire_ticks=90;snapshot(&c,&u);draw(&altered,&u);assert(memcmp(expected_top,a.pixels,sizeof expected_top));
 /* Real simulation drives the visible charge bar; no test-only mirrored field. */
 snapshot(&c,&u);int charge=c.game.enemy_fire_ticks;
 for(int i=0;i<30;i++)game_step(&c.game);
 assert(c.game.enemy_fire_ticks>charge);draw(&c,&u);
 assert(memcmp(expected_top+115*256+157,a.pixels+115*256+157,90*sizeof(uint16_t)));
 /* Allocated system power and required weapon power are separately visible. */
 snapshot(&c,&u);altered=c;altered.game.enemy.systems[SYS_WEAPONS].power=0;draw(&altered,&u);
 assert(memcmp(expected_top+107*256+157,a.pixels+107*256+157,8*256*sizeof(uint16_t)));
 /* Broken sensors: only crew-present rooms; away party provides local sight. */
 c.game.subsystems[0][SUB_SENSORS].health=0;u.aiming=0;altered=c;int hidden=-1;for(int i=0;i<c.game.player.room_count;i++)if(!game_room_visible(&c.game,0,c.game.player.rooms[i].id)){hidden=i;break;}assert(hidden>=0);
 altered.game.player.rooms[hidden].oxygen=0;altered.game.player.rooms[hidden].fires=1;altered.game.player.rooms[hidden].breaches=1;altered.game.player.oxygen=0;snapshot(&c,&u);equal(&altered,&u);
 c.game.crew[0].ship=1;c.game.crew[0].room=c.game.crew[0].destination=c.game.enemy.rooms[0].id;assert(game_room_visible(&c.game,1,c.game.enemy.rooms[0].id));u.aiming=1;altered=c;altered.game.enemy.rooms[0].fires=1;snapshot(&c,&u);draw(&altered,&u);assert(memcmp(expected_bottom,b.pixels,sizeof expected_bottom));
 campaign_init_legacy(&c);c.scrap=1000;u=(Ui){.page=2};campaign_touch(&c,&u,100,28);assert(u.page==5);int cap=c.game.subsystems[0][SUB_DOORS].capacity;campaign_touch(&c,&u,100,55);assert(c.game.subsystems[0][SUB_DOORS].capacity==cap+1);draw(&c,&u);campaign_touch(&c,&u,100,162);assert(u.page==2);
 c.mode=CAM_COMBAT;u=(Ui){.doors=1};c.game.subsystems[0][SUB_DOORS].health=0;int open=c.game.doors[0].open;assert(!game_toggle_door(&c.game,0)&&c.game.doors[0].open==open);campaign_touch(&c,&u,200,124);assert(u.feedback_error&&strstr(u.feedback,"repair"));draw(&c,&u);
 puts("Subsystem UI: hidden-state framebuffer invariance, sensor levels/away sight, diagnostic gating, upgrade controls and bounds passed");}
