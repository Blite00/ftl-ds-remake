/* V52: every imported base-game enemy ship (indices 4..35) sets up with its own
 * rooms, doors, systems, crew species and an original weapon loadout, renders,
 * and fights: crewed ships man their weapons and fire at an idle player. */
#include "render.h"
#include "species.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static uint16_t top[256*192],bottom[256*192];
static int incoming(void){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].age==0;return n;}
int main(void){
 audio_init();
 assert(game_ship_count()==39&&!strcmp(game_ship_blueprint(4),"REBEL_SKINNY_ELITE")&&!strcmp(game_ship_blueprint(35),"FED_BOMBER_P"));
 int cloaks=0,teleporters=0,drones=0,species_seen=0;
 for(int ship=4;ship<game_ship_count();ship++)for(int sector=0;sector<8;sector+=3){
  game_init(&g);g.enemy_ship=ship;assert(game_begin_encounter(&g,ENEMY_SHIP,sector,0));
  assert(g.enemy_kind==ENEMY_SHIP&&game_enemy_import_index(&g)==ship&&strlen(game_enemy_name(&g))>3);
  assert(g.enemy.room_count>=3&&g.enemy.hull>=6&&g.enemy_door_count>0&&(g.enemy_weapon_count>=1||!strcmp(game_ship_blueprint(ship),"ZOLTAN_PEACE")||!strcmp(game_ship_blueprint(ship),"BOSS_SPECIAL"))); /* V61/V66: the Zoltan envoy and the half-built flagship carry no weapons system. */
  int power=0;for(int i=0;i<g.enemy_weapon_count;i++)power+=game_enemy_weapon_at(&g,i)->power;assert(power<=g.enemy.systems[SYS_WEAPONS].capacity);
  if(g.enemy.automated)assert(!game_alive_enemy_crew(&g));
  else{assert(game_alive_enemy_crew(&g)>=2&&g.enemy_pilot_room>=0);int s=g.enemy_crew[0].species;for(int i=1;i<game_alive_enemy_crew(&g);i++)assert(g.enemy_crew[i].species==s);species_seen|=1<<s;}
  /* The weapons room is reachable from the pilot (some originals, like the Slug
   * Interceptor, have sealed engine/oxygen rooms by design). */
  if(g.enemy_pilot_room>=0&&g.enemy.systems[SYS_WEAPONS].room>=0&&g.enemy.systems[SYS_WEAPONS].room!=g.enemy_pilot_room)assert(game_next_room(&g,1,g.enemy_pilot_room,g.enemy.systems[SYS_WEAPONS].room)>=0);
  cloaks+=g.enemy_cloak.capacity>0;teleporters+=g.teleporter[1].installed;drones+=g.enemy_drone.capacity>0;
  {Ui u={0};render_game(&g,&u,top,bottom);u.aiming=1;render_game(&g,&u,top,bottom);}
  if(sector==3){for(int i=0;i<g.weapon_count;i++){g.weapons[i].enabled=0;}g.player.hull=g.player.max_hull=1000;
   int shots=0;for(int t=0;t<60*GAME_HZ&&g.outcome==GAME_PLAYING;t++){game_step(&g);shots+=incoming();}
   if(!shots){fprintf(stderr,"%s fired nothing\n",game_ship_blueprint(ship));}assert(shots>0||!strcmp(game_ship_blueprint(ship),"ZOLTAN_PEACE")||!strcmp(game_ship_blueprint(ship),"BOSS_SPECIAL"));}
 }
 assert(cloaks>0&&teleporters>0&&drones>0);
 assert(species_seen==((1<<SPEC_HUMAN)|(1<<SPEC_ENGI)|(1<<SPEC_MANTIS)|(1<<SPEC_ROCK)|(1<<SPEC_SLUG)|(1<<SPEC_ZOLTAN)|(1<<SPEC_CRYSTAL)));
 /* Out-of-range ship indices fall back to the Rebel fighter. */
 game_init(&g);g.enemy_ship=99;assert(game_begin_encounter(&g,ENEMY_SHIP,1,0)&&g.enemy_kind==ENEMY_REBEL);
 puts("PASS: 32 original enemy ships (plus pirate variants) set up, render, crew by species and fire.");
 return 0;
}
