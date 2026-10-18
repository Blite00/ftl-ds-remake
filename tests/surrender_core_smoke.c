/* Core ceasefire fixtures. Party exchange is an authored DS adaptation. */
#include "game.h"
#include "boarding.h"
#include "species.h"
#include "outcome.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static void setup(void){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL_BOARDER,1,0));}
static void reject(void){before=g;assert(!game_can_accept_surrender(&g));assert(!memcmp(&before,&g,sizeof g));assert(!game_accept_surrender(&g)&&!memcmp(&before,&g,sizeof g));}
static void identity(const Crew*a,const Crew*b){assert(a->id==b->id&&a->health==b->health&&a->species==b->species&&a->station==b->station&&a->ability_ticks==b->ability_ticks&&!strcmp(a->name,b->name)&&!memcmp(a->skill,b->skill,sizeof a->skill));}
int main(void){
 assert(RESULT_PLAYER_ESCAPE==6&&RESULT_ENEMY_SURRENDER==7&&GAME_ESCAPE==3&&GAME_SURRENDER==4);assert(!game_can_accept_surrender(NULL)&&!game_accept_surrender(NULL));
 setup();g.outcome=GAME_VICTORY;reject();setup();g.player.hull=0;reject();setup();g.enemy.hull=0;reject();setup();g.enemy_departed=1;reject();setup();for(int i=0;i<g.enemy_crew_count;i++)g.enemy_crew[i].health=0;reject();
 /* Capacity failure is atomic even with a valid living party to return. */
 setup();g.crew_count=2;g.crew[1].ship=1;g.crew[1].room=g.crew[1].destination=g.enemy.rooms[0].id;g.player.room_count=1;g.player.rooms[0].w=g.player.rooms[0].h=1;reject();
 setup();g.enemy.room_count=1;g.enemy.rooms[0].w=g.enemy.rooms[0].h=1;g.enemy_crew[0].room=g.enemy_crew[0].destination=g.enemy.rooms[0].id;g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=g.pilot_room;g.enemy_crew[2].health=0;g.enemy_crew[2].death_processed=1;reject();
 /* Forty-eight occupied carried slots cannot swallow a pending local death. */
 setup();int slot=GAME_MAX_ENEMY_CREW;for(int r=0;r<g.player.room_count;r++)for(int n=0;n<g.player.rooms[r].w*g.player.rooms[r].h;n++){game_init_crew(&g,1,slot,g.player.rooms[r].id);g.enemy_crew[slot++].ship=0;}assert(slot==GAME_MAX_HOSTILES);g.enemy_crew_count=GAME_MAX_HOSTILES;g.enemy_crew[2].ship=0;g.enemy_crew[2].room=g.enemy_crew[2].destination=g.pilot_room;g.enemy_crew[2].health=0;g.enemy_crew[2].death_processed=0;reject();
 /* Mixed living parties, pending local/remote deaths and an unrelated carried foe. */
 setup();g.crew_count=4;game_init_crew(&g,0,3,g.pilot_room);g.crew[1].ship=1;g.crew[1].station=g.pilot_room;g.crew[1].room=g.crew[1].destination=g.enemy.rooms[0].id;g.crew[1].health=83;g.crew[1].skill[2]=123;g.crew[1].combat_ticks=9;
 g.crew[2].ship=1;g.crew[2].room=g.crew[2].destination=g.enemy.rooms[0].id;g.crew[2].health=0;g.crew[2].species=SPEC_ZOLTAN;g.crew[2].death_processed=0;
 g.enemy_crew[1].ship=0;g.enemy_crew[1].room=g.enemy_crew[1].destination=g.pilot_room;g.enemy_crew[1].health=77;g.enemy_crew[1].skill[1]=87;
 g.enemy_crew[2].ship=0;g.enemy_crew[2].room=g.enemy_crew[2].destination=g.pilot_room;g.enemy_crew[2].health=0;g.enemy_crew[2].species=SPEC_ZOLTAN;g.enemy_crew[2].death_processed=0;
 game_init_crew(&g,1,GAME_MAX_HOSTILES-1,g.player.systems[SYS_WEAPONS].room);g.enemy_crew[GAME_MAX_HOSTILES-1].ship=0;g.enemy_crew_count=GAME_MAX_HOSTILES;Crew carried=g.enemy_crew[GAME_MAX_HOSTILES-1],dead=g.enemy_crew[2],ally=g.crew[1],foe=g.enemy_crew[1];
 g.effects[0]=(GameEffect){.active=1,.duration=1,.side=1,.damage=99};g.weapons[0].pending_shots=1;g.weapons[0].queued_damage.hull=99;g.enemy_pending_shots=2;g.enemy_weapon.pending_shots=2;g.enemy_drone.charge_ticks=200;g.enemy_escape_active=1;g.enemy_escape_total=g.enemy_escape_ticks=90;g.autofire[0]=1;
 g.drones.slots[0].deployed=g.drones.slots[0].enabled=1;g.paused=1;int ammo=g.missiles,parts=g.drone_parts,hull=g.player.hull,remote_hull=g.enemy.hull;
 before=g;assert(game_can_accept_surrender(&g)&&!memcmp(&before,&g,sizeof g));assert(game_accept_surrender(&g));
 assert(g.outcome==GAME_SURRENDER&&g.result_reason==RESULT_ENEMY_SURRENDER&&!g.enemy_departed&&g.enemy.hull==remote_hull&&g.player.hull==hull&&g.missiles==ammo&&g.drone_parts==parts);
 identity(&ally,&g.crew[1]);identity(&foe,&g.enemy_crew[1]);assert(!g.crew[1].ship&&g.crew[1].room!=g.pilot_room&&g.crew[1].room==g.crew[1].destination&&!g.crew[1].combat_ticks);assert(g.enemy_crew[1].ship==1);
 assert(g.crew[2].ship==1&&g.crew[2].death_processed&&!g.crew[2].health);assert(!memcmp(&carried,&g.enemy_crew[GAME_MAX_HOSTILES-1],sizeof carried));assert(!memcmp(&dead,&g.enemy_crew[GAME_MAX_ENEMY_CREW],sizeof dead));
 assert(!g.enemy_pending_shots&&!g.enemy_weapon.pending_shots&&!g.enemy_drone.charge_ticks&&!g.enemy_escape_active&&!g.weapons[0].pending_shots&&!g.autofire[0]&&!g.drones.slots[0].deployed);
 for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g.effects[i].active);
 assert(outcome_ship(&g)==-1&&outcome_duration(&g)==0);before=g;for(int i=0;i<10;i++)game_step(&g);assert(!memcmp(&before,&g,sizeof g));assert(!game_accept_surrender(&g)&&!memcmp(&before,&g,sizeof g));
 game_service_step(&g);assert(!memcmp(&before,&g,sizeof g));g.paused=0;int hp=g.crew[0].health;Crew remote=g.enemy_crew[1];game_service_step(&g);assert(g.crew[0].health==hp-15&&g.enemy_crew[GAME_MAX_ENEMY_CREW].death_processed&&!memcmp(&remote,&g.enemy_crew[1],sizeof remote));
 boarding_prepare_departure(&g);g.crew[2].room=g.crew[2].destination=g.pilot_room;game_service_step(&g);assert(g.crew[0].health==hp-15);
 /* Carried threats remain hostile while the surrendered remote crew stay frozen. */
 g.crew[1].room=g.crew[1].destination=g.enemy_crew[GAME_MAX_HOSTILES-1].room;g.crew[1].combat_ticks=g.enemy_crew[GAME_MAX_HOSTILES-1].combat_ticks=GAME_HZ-1;int friendly=g.crew[1].health,hostile=g.enemy_crew[GAME_MAX_HOSTILES-1].health;game_service_step(&g);assert(g.crew[1].health<friendly&&g.enemy_crew[GAME_MAX_HOSTILES-1].health<hostile&&!memcmp(&remote,&g.enemy_crew[1],sizeof remote));
 puts("Surrender core: pure atomic preflight, capacity/station fallback, negotiated party identity preservation, carried threats/pending deaths, intact hull/no animation, queued ceasefire and terminal/service clocks passed.");
}
