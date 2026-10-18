#include "species.h"
#include "subsystems.h"
#include "boarding.h"
#include "breach.h"
#include "fire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void init(void){game_init(&g);g.enemy_weapon.cooldown_ticks=999999;g.enemy_weapon.shots=0;}
static void tick(int n){while(n--)game_service_step(&g);}
static GameRoom*room(Ship*s,int id){int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return NULL;}
int main(void){int i,a,b,d=-1,target,used;
 init();for(i=3;i<GAME_MAX_CREW;i++)game_init_crew(&g,0,i,g.pilot_room);g.crew_count=GAME_MAX_CREW;
 assert(g.crew_count==8&&game_order_group(&g,255u,g.player.systems[SYS_MEDBAY].room));
 g.crew[7].health=0;snapshot=g;assert(!game_order_group(&g,255u,g.pilot_room));assert(!memcmp(g.crew,snapshot.crew,sizeof g.crew));
 for(i=0;i<SPEC_COUNT;i++){assert(game_set_crew_species(&g,0,0,i));assert(g.crew[0].health==species_max_health(i));}
 assert(!game_set_crew_species(&g,0,0,SPEC_COUNT));
 /* Actual one-edge movement honors Mantis and Rock speed. */
 init();for(i=0;i<g.door_count;i++)if(g.doors[i].room_a>=0&&g.doors[i].room_b>=0){d=i;break;}assert(d>=0);a=g.doors[d].room_a;b=g.doors[d].room_b;
 g.crew_count=1;g.crew[0].room=g.crew[0].destination=a;game_set_crew_species(&g,0,0,SPEC_ROCK);assert(game_order_crew(&g,0,b));tick(59);assert(g.crew[0].room==a);tick(1);assert(g.crew[0].room==b);
 init();g.crew_count=1;g.crew[0].room=g.crew[0].destination=a;game_set_crew_species(&g,0,0,SPEC_MANTIS);game_order_crew(&g,0,b);tick(25);assert(g.crew[0].room==b);
 /* Repair rates remain fractional without losing accumulated half-rate work. */
 init();target=g.player.systems[SYS_WEAPONS].room;game_set_crew_species(&g,0,2,SPEC_ENGI);g.player.systems[SYS_WEAPONS].health=2;tick(74);assert(g.player.systems[SYS_WEAPONS].health==2);tick(1);assert(g.player.systems[SYS_WEAPONS].health==3);
 init();game_set_crew_species(&g,0,2,SPEC_MANTIS);g.player.systems[SYS_WEAPONS].health=2;tick(299);assert(g.player.systems[SYS_WEAPONS].health==2);tick(1);assert(g.player.systems[SYS_WEAPONS].health==3);
 breach_add(&g.player,target,1);for(i=0;i<300;i++){g.ticks++;breach_step(&g,1);}assert(!room(&g.player,target)->breaches);
 /* Species health cap and fire/suffocation traits affect simulation. */
 init();game_set_crew_species(&g,0,2,SPEC_ROCK);target=g.crew[2].room;fire_ignite(&g.player,target,1);tick(GAME_HZ);assert(g.crew[2].health==150);
 g.crew[2].room=g.crew[2].destination=g.player.systems[SYS_MEDBAY].room;g.crew[2].health=147;g.player.systems[SYS_MEDBAY].power=1;tick(GAME_HZ);assert(g.crew[2].health==150);
 init();game_set_crew_species(&g,0,2,SPEC_CRYSTAL);room(&g.player,g.crew[2].room)->oxygen=0;breach_add(&g.player,g.crew[2].room,1);tick(2*GAME_HZ);assert(g.crew[2].health==120);
 /* Engi and Mantis hit with different amounts against the same faction-aware opponent. */
 init();game_set_crew_species(&g,0,2,SPEC_MANTIS);game_init_crew(&g,1,0,g.crew[2].room);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;game_set_crew_species(&g,1,0,SPEC_ENGI);
 for(i=0;i<GAME_HZ;i++){g.ticks++;boarding_step(&g,0);}assert(g.crew[2].health==97&&g.enemy_crew[0].health==91);
 /* Slug sight survives broken sensors, but is lost when the Slug dies. */
 init();game_begin_encounter(&g,ENEMY_REBEL,0,0);g.subsystems[0][SUB_SENSORS].health=0;target=g.enemy_crew[0].room;assert(!game_room_visible(&g,1,target));game_set_crew_species(&g,0,0,SPEC_SLUG);assert(game_room_visible(&g,1,target));g.crew[0].health=0;assert(!game_room_visible(&g,1,target));
 /* Zoltan power belongs to its current friendly room and never consumes reactor. */
 init();g.player.systems[SYS_WEAPONS].power=0;used=game_used_power(&g);game_set_crew_species(&g,0,2,SPEC_ZOLTAN);assert(g.player.systems[SYS_WEAPONS].bonus_power==1&&game_system_power(&g.player,SYS_WEAPONS)==1&&game_used_power(&g)==used);
 g.crew[2].room=g.player.systems[SYS_OXYGEN].room;species_refresh_power(&g);assert(!g.player.systems[SYS_WEAPONS].bonus_power&&g.player.systems[SYS_OXYGEN].bonus_power==1);
 g.crew[2].health=0;species_refresh_power(&g);assert(!g.player.systems[SYS_OXYGEN].bonus_power);
 /* The one-time death burst can chain, never repeats or resurrects. */
 init();game_set_crew_species(&g,0,2,SPEC_ZOLTAN);target=g.crew[2].room;game_init_crew(&g,1,0,target);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;game_set_crew_species(&g,1,0,SPEC_ZOLTAN);g.enemy_crew[0].health=10;
 g.crew[1].room=g.crew[1].destination=target;g.crew[1].health=10;g.crew[2].health=0;species_step(&g,0);assert(g.crew[2].death_processed&&g.enemy_crew[0].death_processed&&g.crew[1].health==0);snapshot=g;species_step(&g,0);assert(!memcmp(g.crew,snapshot.crew,sizeof g.crew));
 /* Lockdown closes doors, blocks both factions and remote controls, pauses and expires. */
 init();game_set_crew_species(&g,0,0,SPEC_CRYSTAL);g.crew[0].room=g.crew[0].destination=a;assert(game_crystal_lockdown(&g,0));assert(!game_crystal_lockdown(&g,0));assert(!game_toggle_door(&g,d));assert(!subsystem_door_pass(&g,0,0,d)&&!subsystem_door_pass(&g,1,0,d));
 g.paused=1;snapshot=g;species_step(&g,0);assert(!memcmp(&g,&snapshot,sizeof g));g.paused=0;for(i=0;i<CRYSTAL_LOCKDOWN_TICKS;i++)species_step(&g,1);assert(!room(&g.player,a)->lockdown_ticks&&subsystem_door_pass(&g,0,0,d));assert(g.crew[0].ability_ticks==CRYSTAL_COOLDOWN_TICKS-CRYSTAL_LOCKDOWN_TICKS);
 puts("PASS: eight crew/group orders, species health/speed/work/combat/hazards, Slug sight, local Zoltan power/death chains and Crystal lockdown.");return 0;
}
