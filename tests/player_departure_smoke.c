/* Departure fixtures: no campaign resources or arrival behavior simulated. */
#include "game.h"
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static void setup(void){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL_BOARDER,1,0));g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;}
static void rejected(int abandon){before=g;assert(!game_can_depart(&g,abandon));assert(!memcmp(&before,&g,sizeof g));assert(!game_depart(&g,abandon));assert(!memcmp(&before,&g,sizeof g));}
int main(void){
 assert(RESULT_ENEMY_ESCAPE==5&&RESULT_PLAYER_ESCAPE==6);assert(!game_can_depart(NULL,0)&&!game_depart(NULL,0)&&!game_begin_encounter(NULL,0,0,0));
 setup();g.player_ftl_charge--;rejected(0);setup();rejected(-1);rejected(2);g.pilot_health=0;rejected(0);setup();g.pilot_ion_ticks[0]=5;rejected(0);setup();g.outcome=GAME_VICTORY;rejected(1);
 setup();g.crew[1].ship=1;g.crew[1].room=g.crew[1].destination=g.enemy.rooms[0].id;rejected(0);assert(game_can_depart(&g,1));
 setup();for(int i=0;i<g.crew_count;i++)g.crew[i].ship=1;rejected(1);
 setup();game_init_crew(&g,1,GAME_MAX_HOSTILES-1,0);g.enemy_crew_count=GAME_MAX_HOSTILES;rejected(0);g.enemy_crew[GAME_MAX_HOSTILES-1].ship=0;g.enemy_crew[GAME_MAX_HOSTILES-1].room=999;rejected(0);
 setup();for(int i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++){game_init_crew(&g,1,i,0);g.enemy_crew[i].ship=0;}g.enemy_crew_count=GAME_MAX_HOSTILES;g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=0;rejected(0);
 /* Preserve home systems/crew/hazards and complete hostile records, discard only
  * remote encounter state and departures explicitly authorized by the caller. */
 setup();g.crew[1].species=SPEC_ZOLTAN;g.crew[1].ship=1;g.crew[1].room=g.crew[1].destination=g.enemy.rooms[0].id;unsigned lost=g.crew[1].id;
 g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=g.player.systems[SYS_WEAPONS].room;Crew carried=g.enemy_crew[1];
 g.enemy_crew[2].ship=0;g.enemy_crew[2].room=g.enemy_crew[2].destination=g.pilot_room;g.enemy_crew[2].species=SPEC_ZOLTAN;g.enemy_crew[2].health=0;g.enemy_crew[2].death_processed=0;Crew pending=g.enemy_crew[2];
 g.player.rooms[2].fires=1;g.player.rooms[3].breaches=1;g.player.rooms[4].oxygen=321;g.doors[0].open=1;
 g.effects[0]=(GameEffect){.active=1,.duration=20,.damage=99};g.weapons[0].pending_shots=1;g.weapons[0].queued_damage.hull=99;g.autofire[0]=1;g.enemy_pending_shots=2;g.drones.slots[0].deployed=g.drones.slots[0].enabled=1;
 before=g;assert(game_can_depart(&g,1)&&!memcmp(&before,&g,sizeof g));assert(game_depart(&g,1));
 assert(!memcmp(&before.player,&g.player,sizeof g.player)&&!memcmp(before.doors,g.doors,sizeof g.doors));assert(!memcmp(&before.crew[0],&g.crew[0],sizeof(Crew))&&!memcmp(&before.crew[2],&g.crew[2],sizeof(Crew)));
 assert(g.crew[1].id==lost&&!g.crew[1].health&&g.crew[1].death_processed&&g.crew[1].ship==0);
 assert(g.enemy_crew_count==GAME_MAX_ENEMY_CREW+2&&!memcmp(&carried,&g.enemy_crew[GAME_MAX_ENEMY_CREW],sizeof carried)&&!memcmp(&pending,&g.enemy_crew[GAME_MAX_ENEMY_CREW+1],sizeof pending));
 assert(g.outcome==GAME_PLAYING&&g.result_reason==RESULT_PLAYER_ESCAPE&&g.enemy_departed&&!g.enemy.hull&&!g.enemy.room_count&&!g.player_ftl_charge&&g.target_room==-1);
 assert(!g.enemy_pending_shots&&!g.enemy_fire_ticks&&!g.enemy_weapon.pending_shots&&!g.enemy_drone.capacity&&!g.teleporter[1].installed&&!g.drones.slots[0].deployed&&!g.drones.slots[0].enabled);
 for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g.effects[i].active);
 for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)assert(!g.enemy_crew[i].id);
 assert(!g.weapons[0].pending_shots&&!g.weapons[0].queued_damage.hull&&!g.autofire[0]);
 /* Pending HOME death fires once. Remote abandoned Zoltan cannot burst at home. */
 int hp=g.crew[0].health;game_service_step(&g);assert(g.crew[0].health==hp-15&&g.enemy_crew[GAME_MAX_ENEMY_CREW+1].death_processed);game_service_step(&g);assert(g.crew[0].health==hp-15);
 g.enemy_crew[GAME_MAX_ENEMY_CREW].health=0;g.enemy_crew[GAME_MAX_ENEMY_CREW].death_processed=1;game_step(&g);assert(g.outcome==GAME_PLAYING&&g.result_reason==RESULT_PLAYER_ESCAPE&&!g.player_ftl_charge);g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;rejected(1);
 assert(game_begin_encounter(&g,ENEMY_REBEL,2,0));assert(g.result_reason==RESULT_NONE&&!g.enemy_departed&&!g.player_ftl_charge&&game_alive_enemy_crew(&g)==3);
 /* A new enemy outcome owns only its current crew. Older carried intruders
  * remain alive for service defense and do not block that enemy's hull result. */
 setup();game_init_crew(&g,1,GAME_MAX_HOSTILES-1,g.player.systems[SYS_WEAPONS].room);g.enemy_crew[GAME_MAX_HOSTILES-1].ship=0;g.enemy_crew_count=GAME_MAX_HOSTILES;g.enemy.hull=0;game_step(&g);assert(g.outcome==GAME_VICTORY&&g.result_reason==RESULT_ENEMY_HULL&&g.enemy_crew[GAME_MAX_HOSTILES-1].health>0);
 setup();g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=g.player.systems[SYS_WEAPONS].room;g.enemy.hull=0;game_step(&g);assert(g.outcome==GAME_PLAYING&&g.enemy_crew[1].health>0);
 /* A refused encounter must not normalize dead remote friendly crew first. */
 setup();g.crew[1].ship=1;g.crew[1].health=0;g.crew[2].ship=1;Crew dead=g.crew[1];assert(!game_begin_encounter(&g,ENEMY_SCOUT,0,0)&&!memcmp(&dead,&g.crew[1],sizeof dead));
 puts("Player departure: pure atomic refusal, explicit away loss, pending home deaths, carried records/home hazards preserved, remote queues cleared, no false enemy reward and encounter success/refusal passed.");
}
