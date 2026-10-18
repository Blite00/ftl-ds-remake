#include "game.h"
#include "species.h"
#include "fire.h"
#include "breach.h"
#include "subsystems.h"
#include "boarding.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;
static int last=GAME_MAX_HOSTILES-1;
static GameRoom*room(int side,int id){Ship*s=side?&g.enemy:&g.player;for(int i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];assert(0);return NULL;}
static void init(void){
 game_init(&g);g.enemy_weapon.shots=0;g.enemy_weapon.cooldown_ticks=999999;
 memset(g.enemy_crew,0,sizeof g.enemy_crew);g.enemy_crew_count=GAME_MAX_HOSTILES;
 for(int i=0;i<GAME_MAX_HOSTILES;i++)g.enemy_crew[i].death_processed=1;
 g.crew_count=1;g.crew[0].ship=0;g.crew[0].room=g.crew[0].destination=g.pilot_room;
}
static Crew*intruder(int index,int where,int species){
 Crew*c=&g.enemy_crew[index];memset(c,0,sizeof *c);c->id=g.next_crew_id++;c->ship=0;c->room=c->destination=c->station=where;
 assert(game_set_crew_species(&g,1,index,species));return c;
}
int main(void){int target;
 /* Last carried slot takes fire damage; hostile Engi cannot fight home fires. */
 init();target=g.player.systems[SYS_WEAPONS].room;Crew*c=intruder(last,target,SPEC_ENGI);fire_ignite(&g.player,target,1);
 room(0,target)->fire_ticks=GAME_HZ-1;g.ticks=1;fire_step(&g,1);assert(c->health==98&&room(0,target)->fires==1&&!room(0,target)->firefight_ticks);
 assert(!game_set_crew_species(&g,1,GAME_MAX_HOSTILES,SPEC_HUMAN));assert(!game_set_crew_species(&g,0,GAME_MAX_CREW,SPEC_HUMAN));
 /* Carried Rock immunity applies, and remote hazards remain frozen in service. */
 init();target=g.player.systems[SYS_WEAPONS].room;c=intruder(last,target,SPEC_ROCK);fire_ignite(&g.player,target,1);room(0,target)->fire_ticks=GAME_HZ-1;
 g.enemy.rooms[0].fires=1;g.enemy.rooms[0].fire_ticks=12;before=g;g.ticks=1;fire_step(&g,1);assert(c->health==150&&!memcmp(&g.enemy,&before.enemy,sizeof g.enemy));
 /* Hostile workers neither seal home breaches nor repair damaged home systems. */
 init();target=g.player.systems[SYS_WEAPONS].room;c=intruder(last,target,SPEC_ENGI);breach_add(&g.player,target,1);
 for(int i=0;i<BREACH_SEAL_TICKS*2;i++){g.ticks++;breach_step(&g,1);}assert(room(0,target)->breaches==1&&!room(0,target)->breach_repair_ticks);
 room(0,target)->breaches=0;g.player.systems[SYS_WEAPONS].health=1;int health=g.player.systems[SYS_WEAPONS].health;game_service_step(&g);assert(g.player.systems[SYS_WEAPONS].health==health);
 /* A hostile in powered medbay cannot heal; oxygen loss reaches lastslot. */
 init();target=g.player.systems[SYS_MEDBAY].room;c=intruder(last,target,SPEC_HUMAN);c->health=50;g.ticks=GAME_HZ-1;game_service_step(&g);assert(c->health<=50);
 init();target=g.player.systems[SYS_WEAPONS].room;c=intruder(last,target,SPEC_HUMAN);g.player.systems[SYS_OXYGEN].power=0;
 for(int i=0;i<g.player.room_count;i++)g.player.rooms[i].oxygen=0;
 g.ticks=GAME_HZ-1;game_service_step(&g);assert(c->health==95);
 /* Physical home location controls ability clock, not hostile ownership. */
 init();target=g.player.systems[SYS_WEAPONS].room;c=intruder(last,target,SPEC_CRYSTAL);c->ability_ticks=10;
 Crew*remote=&g.enemy_crew[0];remote->health=100;remote->ship=1;remote->room=remote->destination=g.enemy.rooms[0].id;remote->ability_ticks=20;
 species_step(&g,1);assert(c->ability_ticks==9&&remote->ability_ticks==20);
 g.paused=1;before=g;species_step(&g,1);assert(!memcmp(&g,&before,sizeof g));g.paused=0;
 /* Carried Zoltan cannot power either hull from the player's weapon room. */
 init();target=g.player.systems[SYS_WEAPONS].room;c=intruder(last,target,SPEC_ZOLTAN);species_refresh_power(&g);assert(!g.player.systems[SYS_WEAPONS].bonus_power);
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)assert(!g.enemy.systems[i].bonus_power);
 /* Home death chains include final hostile slot, once only; remote frozen. */
 init();target=g.pilot_room;assert(game_set_crew_species(&g,0,0,SPEC_ZOLTAN));g.crew[0].health=10;
 c=intruder(last,target,SPEC_ZOLTAN);c->health=0;Crew*other=intruder(last-1,target,SPEC_HUMAN);other->health=10;
 remote=&g.enemy_crew[0];remote->species=SPEC_ZOLTAN;remote->ship=1;remote->health=0;remote->death_processed=0;
 species_step(&g,1);assert(!g.crew[0].health&&!other->health&&c->death_processed&&other->death_processed&&g.crew[0].death_processed&&!remote->death_processed);
 before=g;species_step(&g,1);assert(!memcmp(&g,&before,sizeof g));
 /* Home melee service reaches the final index in the expanded scratch array. */
 init();target=g.pilot_room;c=intruder(last,target,SPEC_MANTIS);c->combat_ticks=g.crew[0].combat_ticks=GAME_HZ-1;boarding_step(&g,1);assert(g.crew[0].health==91&&c->health==94);
 puts("Carried hazards: slot55 fire/oxygen/melee, no hostile repairs/healing/power, service ability clocks, remote freeze, Rock immunity and once-only Zoltan death chains passed");return 0;
}
