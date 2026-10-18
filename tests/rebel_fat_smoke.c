#include "render.h"
#include "audio.h"
#include "boarding.h"
#include "enemy_catalog.h"
#include "enemy_catalog_art.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned head;uint16_t pixels[256*192];unsigned tail;} top,bottom;
static int used(const Game*g){int n=g->enemy_drone.power+g->teleporter[1].power;for(int i=0;i<GAME_SYSTEM_COUNT;i++)n+=g->enemy.systems[i].power;return n;}
static void draw(Campaign*c,Ui*u){top.head=bottom.head=123;top.tail=bottom.tail=456;render_campaign(c,u,top.pixels,bottom.pixels);assert(top.head==123&&bottom.head==123&&top.tail==456&&bottom.tail==456);}
int main(void){
 Campaign c;Ui u={.weapon=1};Game*g=&c.game;audio_init();campaign_init_legacy(&c);c.mode=CAM_COMBAT;
 assert(ENEMY_FLAGSHIP==3&&ENEMY_REBEL_BOARDER==6&&ENEMY_REBEL_FAT==7);
 const ImportedEnemy*d=&imported_enemies[3];assert(!strcmp(d->blueprint,"REBEL_FAT"));assert(d->required_capabilities==(ENEMY_CAP_CREW|ENEMY_CAP_DRONES));
 for(int sector=0;sector<8;sector++){
  game_begin_encounter(g,ENEMY_REBEL_FAT,sector,0);assert(game_enemy_import_index(g)==3);assert(!strcmp(game_enemy_name(g),d->name));
  assert(g->enemy.room_count==d->room_count&&g->enemy_door_count==d->door_count);assert(g->enemy.max_hull==d->hull+2*sector);
  for(int i=0;i<d->room_count;i++){const GameRoom*r=&g->enemy.rooms[i];const ImportedEnemyRoom*o=&d->rooms[i];assert(r->id==o->id&&r->x==o->x&&r->y==o->y&&r->w==o->w&&r->h==o->h);for(int j=0;j<d->room_count;j++)assert(game_next_room(g,1,r->id,g->enemy.rooms[j].id)>=0);}
  assert(g->enemy_pilot_room==0&&g->enemy.systems[SYS_OXYGEN].room==2&&g->enemy.systems[SYS_SHIELDS].room==4&&g->enemy.systems[SYS_ENGINES].room==7&&g->enemy.systems[SYS_WEAPONS].room==6&&g->enemy.systems[SYS_MEDBAY].room==3);
  assert(g->enemy_crew_count==3&&!g->enemy.automated&&g->enemy_drone.room==1&&g->enemy_drone.capacity==2);assert(!g->teleporter[1].installed&&!g->subsystems[1][SUB_DOORS].capacity);
  for(int i=0;i<3;i++)assert(g->enemy_crew[i].health>0&&g->enemy_crew[i].name[0]&&g->enemy_crew[i].station==g->enemy_crew[i].room);
  assert(g->enemy_weapon.power==1&&g->enemy.systems[SYS_WEAPONS].power==1&&g->enemy.systems[SYS_ENGINES].power==2&&used(g)==8);
  g->enemy_crew[0].health=20;g->ticks=GAME_HZ-1;game_step(g);assert(used(g)<=8&&g->enemy.systems[SYS_MEDBAY].power==1&&g->enemy.systems[SYS_ENGINES].power==1);
 }
 game_begin_encounter(g,ENEMY_REBEL_FAT,0,0);g->enemy_drone.charge_ticks=g->enemy_drone.cooldown_ticks-1;g->enemy_fire_ticks=g->enemy_weapon.cooldown_ticks-1;game_step(g);
 int launches=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)launches+=g->effects[i].active&&g->effects[i].side==1;assert(launches==2);
 /* A direct hit to control disables its charging, with no hidden drone shot. */
 game_begin_encounter(g,ENEMY_REBEL_FAT,0,0);g->enemy.systems[SYS_ENGINES].power=0;g->ticks=1;g->enemy_drone.charge_ticks=g->enemy_drone.cooldown_ticks-1;
 g->effects[0]=(GameEffect){.active=1,.duration=1,.side=0,.room=1,.kind=EFFECT_LASER,.system_damage=2,.piercing=99};game_step(g);
 assert(!g->enemy_drone.health&&!g->enemy_drone.power&&!g->enemy_drone.charge_ticks);for(int i=1;i<GAME_MAX_EFFECTS;i++)assert(!g->effects[i].active);
 /* A damaged drone room is a repair destination; living crew restore it. */
 game_begin_encounter(g,ENEMY_REBEL_FAT,0,0);g->enemy_drone.health=g->enemy_drone.power=0;g->enemy.systems[SYS_WEAPONS].power=0;
 int assigned=0;for(int tick=0;tick<35*GAME_HZ&&g->enemy_drone.health<2;tick++){game_step(g);assert(used(g)<=g->enemy.reactor);for(int i=0;i<3;i++)assigned|=g->enemy_crew[i].destination==1;}
 assert(assigned&&g->enemy_drone.health==2);for(int i=0;i<GAME_HZ;i++)game_step(g);assert(g->enemy_drone.power==2);
 /* Every enemy room is selectable for normal targeting and boarding. */
 game_begin_encounter(g,ENEMY_REBEL_FAT,0,0);assert(boarding_install(g,0));g->player.systems[SYS_MEDBAY].power=0;g->player.systems[SYS_ENGINES].power=0;assert(boarding_power(g,0,1));
 for(int i=0;i<g->enemy.room_count;i++){assert(game_set_weapon_target(g,1,g->enemy.rooms[i].id));g->crew[0].ship=0;g->crew[0].room=g->crew[0].destination=g->teleporter[0].room;g->teleporter[0].cooldown_ticks=0;assert(boarding_deploy(g,1,g->enemy.rooms[i].id));assert(g->crew[0].ship==1&&g->crew[0].room==g->enemy.rooms[i].id);}
 g->crew[0].ship=0;g->crew[0].room=g->crew[0].destination=g->teleporter[0].room;
 g->subsystems[0][SUB_SENSORS].capacity=g->subsystems[0][SUB_SENSORS].health=3;draw(&c,&u);u.aiming=1;draw(&c,&u);u.aiming=0;u.boarding=1;u.board_side=1;draw(&c,&u);
 int pixels=0;for(int i=0;i<IMPORTED_ENEMY_ART_WIDTH*IMPORTED_ENEMY_ART_HEIGHT;i++)pixels+=imported_enemy_art[3][i]!=0;assert(pixels>500);
 game_begin_encounter(g,ENEMY_REBEL,0,0);assert(game_enemy_import_index(g)==2&&!g->enemy_drone.capacity);
 puts("Rebel Rigger: original topology/art, supported weapons+drone, reactor-budget AI/healing/repair, all-room boarding/targeting and framebuffer guards passed");
}
