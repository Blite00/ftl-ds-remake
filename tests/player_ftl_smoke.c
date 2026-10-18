/* Focused FTL charge fixtures; no departure or campaign acceptance implied. */
#include "game.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void setup(int power){game_init(&g);g.enemy_weapon.cooldown_ticks=100000;g.player.systems[SYS_ENGINES].capacity=g.player.systems[SYS_ENGINES].health=g.player.systems[SYS_ENGINES].power=power;}
static void tick(int n){while(n--)game_step(&g);}
int main(void){
 assert(!game_player_ftl_operational(NULL)&&!game_player_ftl_ready(NULL));
 /* Authored deterministic timing and saturation, without resource expenditure. */
 for(int bars=1;bars<=8;bars++){
  setup(bars);assert(game_player_ftl_operational(&g)&&!game_player_ftl_ready(&g));int ammo=g.missiles;
  int ticks=(GAME_PLAYER_FTL_CHARGE+bars-1)/bars;tick(ticks-1);assert(!game_player_ftl_ready(&g));tick(1);assert(game_player_ftl_ready(&g)&&g.player_ftl_charge==GAME_PLAYER_FTL_CHARGE&&g.missiles==ammo);
  tick(10);assert(g.player_ftl_charge==GAME_PLAYER_FTL_CHARGE);
  g.paused=1;Game before=g;tick(10);assert(game_player_ftl_ready(&g)&&!memcmp(&before,&g,sizeof g));
 }
 setup(2);tick(7);int charge=g.player_ftl_charge;assert(charge==14);g.paused=1;tick(5);assert(g.player_ftl_charge==charge);g.paused=0;
 g.player.systems[SYS_ENGINES].power=0;tick(5);assert(!game_player_ftl_operational(&g)&&g.player_ftl_charge==charge);
 g.player.systems[SYS_ENGINES].power=2;g.player.systems[SYS_ENGINES].health=0;tick(5);assert(!game_player_ftl_operational(&g)&&g.player_ftl_charge==charge);
 g.player.systems[SYS_ENGINES].health=g.player.systems[SYS_ENGINES].power=2;g.player.systems[SYS_ENGINES].ion_bars=2;g.player.systems[SYS_ENGINES].ion_ticks=10;tick(5);assert(g.player_ftl_charge==charge);
 g.player.systems[SYS_ENGINES].ion_bars=g.player.systems[SYS_ENGINES].ion_ticks=0;g.pilot_ion_ticks[0]=10;tick(5);assert(g.player_ftl_charge==charge);
 g.pilot_ion_ticks[0]=0;g.pilot_health=0;tick(5);assert(!game_player_ftl_operational(&g)&&g.player_ftl_charge==charge);g.pilot_health=1;
 g.crew[0].destination=g.player.systems[SYS_ENGINES].room;assert(!game_player_ftl_operational(&g));g.crew[0].destination=g.crew[0].room;
 g.crew[0].ship=1;assert(!game_player_ftl_operational(&g));g.crew[0].ship=0;g.crew[0].health=0;assert(!game_player_ftl_operational(&g));g.crew[0].health=100;assert(game_player_ftl_operational(&g));
 tick(1);assert(g.player_ftl_charge==charge+2);
 /* Zoltan local bonus can operate ionized engines, using the shared power rule. */
 setup(2);g.crew[1].species=SPEC_ZOLTAN;g.crew[1].room=g.crew[1].destination=g.player.systems[SYS_ENGINES].room;g.player.systems[SYS_ENGINES].ion_bars=2;g.player.systems[SYS_ENGINES].ion_ticks=20;species_refresh_power(&g);assert(game_player_ftl_operational(&g));tick(1);assert(g.player_ftl_charge==1);
 /* Service and terminal frames cannot charge; readiness respects terminal state. */
 setup(2);charge=g.player_ftl_charge;for(int i=0;i<20;i++)game_service_step(&g);assert(g.player_ftl_charge==charge);
 for(int outcome=GAME_VICTORY;outcome<=GAME_ESCAPE;outcome++){setup(2);g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;g.outcome=outcome;Game before=g;assert(!game_player_ftl_ready(&g));tick(5);assert(!memcmp(&before,&g,sizeof g));}
 /* Impacts resolve before the final charge unit and can interrupt that tick. */
 setup(1);g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE-1;assert(game_player_ftl_operational(&g));
 g.effects[0]=(GameEffect){.active=1,.duration=1,.side=1,.room=g.player.systems[SYS_ENGINES].room,.kind=EFFECT_BOMB,.bomb_friendly=1,.system_damage=8};tick(1);assert(!g.player.systems[SYS_ENGINES].health&&!game_player_ftl_ready(&g)&&g.player_ftl_charge==GAME_PLAYER_FTL_CHARGE-1);
 setup(1);g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE-1;g.effects[0]=(GameEffect){.active=1,.duration=1,.side=1,.room=g.pilot_room,.kind=EFFECT_BOMB,.bomb_friendly=1,.crew_damage=150};tick(1);assert(!g.crew[0].health&&!game_player_ftl_ready(&g)&&g.player_ftl_charge==GAME_PLAYER_FTL_CHARGE-1);
 setup(1);g.player_ftl_charge=GAME_PLAYER_FTL_CHARGE-1;g.player.hull=0;tick(1);assert(g.outcome==GAME_DEFEAT&&!game_player_ftl_ready(&g)&&g.player_ftl_charge==GAME_PLAYER_FTL_CHARGE-1);
 setup(2);g.player_ftl_charge=123;assert(game_enemy_escape_start(&g,40*GAME_HZ,22,22));assert(g.player_ftl_charge==123);
 /* Refused encounter changes retain charge; successful initialization resets it. */
 setup(2);g.player_ftl_charge=123;g.crew[0].ship=1;game_begin_encounter(&g,ENEMY_ASSAULT,1,0);assert(g.player_ftl_charge==123);g.crew[0].ship=0;
 g.enemy_crew_count=1;game_init_crew(&g,1,0,g.player.rooms[0].id);g.enemy_crew[0].ship=0;game_begin_encounter(&g,ENEMY_ASSAULT,1,0);assert(g.player_ftl_charge==123);
 g.enemy_crew[0].health=0;g.enemy_crew[0].death_processed=1;game_begin_encounter(&g,ENEMY_ASSAULT,1,0);assert(!g.player_ftl_charge&&g.enemy_kind==ENEMY_ASSAULT);
 puts("Player FTL charge: 1-8 engine timings, saturation, pause/terminal/service freeze, disruption and resumption, manning/species power, last-tick impacts and successful-encounter reset passed.");
}
