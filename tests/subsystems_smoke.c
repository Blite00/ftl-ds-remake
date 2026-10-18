#include "subsystems.h"
#include "boarding.h"
#include "fire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void init(void){game_init(&g);g.enemy_weapon.cooldown_ticks=999999;g.enemy_weapon.shots=0;}
static void tick(int n){while(n--)game_step(&g);}
int main(void){int i,d=-1,a,b,room,used;
 init();assert(g.subsystems[0][SUB_DOORS].room==2&&g.subsystems[0][SUB_SENSORS].room==3);
 used=game_used_power(&g);g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=3;assert(game_used_power(&g)==used);
 assert(game_room_visible(&g,0,16));assert(!game_room_visible(&g,1,g.enemy.rooms[0].id));
 g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=2;assert(game_room_visible(&g,1,g.enemy.rooms[0].id)&&!game_enemy_details_visible(&g));
 g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;assert(game_enemy_details_visible(&g));
 g.subsystems[0][SUB_SENSORS].health=0;assert(game_room_visible(&g,0,g.crew[0].room)&&!game_room_visible(&g,0,16));
 g.crew[0].ship=1;g.crew[0].room=g.enemy.rooms[0].id;assert(game_room_visible(&g,1,g.crew[0].room));g.crew[0].health=0;assert(!game_room_visible(&g,1,g.crew[0].room));
 assert(!game_room_visible(&g,0,99));
 /* Unshielded independent system damage disables sensors. */
 init();g.player.shields=0;g.player.systems[SYS_SHIELDS].power=0;g.pilot_health=0;room=g.subsystems[0][SUB_SENSORS].room;
 {GameDamage hit={0,1,0,0,0,0,0,0};assert(game_launch_effect(&g,1,room,EFFECT_LASER,&hit,0));}tick(12);assert(!subsystem_level(&g,0,SUB_SENSORS));
 g.crew[0].room=g.crew[0].destination=room;for(i=0;i<5*GAME_HZ;i++)game_service_step(&g);assert(subsystem_level(&g,0,SUB_SENSORS)==1);
 /* Doors lose remote control, but friendly automatic traversal remains. */
 init();for(i=0;i<g.door_count;i++)if(g.doors[i].room_a>=0&&g.doors[i].room_b>=0){d=i;break;}assert(d>=0);a=g.doors[d].room_a;b=g.doors[d].room_b;
 g.subsystems[0][SUB_DOORS].health=0;snapshot=g;assert(!game_toggle_door(&g,d));assert(!memcmp(g.doors,snapshot.doors,sizeof g.doors));
 assert(subsystem_door_pass(&g,0,0,d));
 /* Reinforcement delays hostile movement, then leaves a temporarily broken door. */
 g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=2;
 game_init_crew(&g,1,0,a);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;g.enemy_crew[0].destination=b;
 tick(GAME_HZ+2*GAME_HZ-2);assert(g.enemy_crew[0].room==a&&!g.doors[d].open);
 tick(2);assert(g.enemy_crew[0].room==b&&g.doors[d].jam_ticks>0&&g.doors[d].open);
 assert(!game_toggle_door(&g,d));game_close_all_doors(&g);assert(g.doors[d].open);
 g.paused=1;snapshot=g;subsystem_step(&g,0);
 assert(!memcmp(&snapshot,&g,sizeof g));g.paused=0;
 for(i=0;i<5*GAME_HZ;i++)subsystem_step(&g,0);
 assert(!g.doors[d].open&&!g.doors[d].jam_ticks);
 /* Interrupted door attacks recover; higher levels require more force. */
 g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=3;
 for(i=0;i<2*GAME_HZ;i++)assert(!subsystem_door_pass(&g,1,0,d));
 assert(g.doors[d].force_ticks==2*GAME_HZ);
 for(i=0;i<GAME_HZ;i++)subsystem_step(&g,0);
 assert(!g.doors[d].force_ticks&&!g.doors[d].force_idle_ticks);
 /* Repeated remote sealing cannot reset damage from active attackers. */
 for(i=0;i<4*GAME_HZ-1;i++){game_close_all_doors(&g);assert(!subsystem_door_pass(&g,1,0,d));}
 assert(subsystem_door_pass(&g,1,0,d)&&g.doors[d].jam_ticks>0);
 g.doors[d].jam_ticks=0;g.doors[d].open=0;g.doors[d].force_ticks=15;g.doors[d].force_idle_ticks=0;
 assert(game_toggle_door(&g,d)&&game_toggle_door(&g,d));assert(g.doors[d].force_ticks==15);
 snapshot=g;subsystem_damage(&g,-1,room,1);subsystem_damage(&g,2,room,1);assert(!memcmp(&g,&snapshot,sizeof g));
 /* Fire and boarder sabotage damage subsystem health as well as projectiles. */
 init();room=g.subsystems[0][SUB_DOORS].room;fire_ignite(&g.player,room,1);g.ticks=3*GAME_HZ;fire_step(&g,0);assert(!g.subsystems[0][SUB_DOORS].health);
 init();room=g.subsystems[0][SUB_SENSORS].room;game_init_crew(&g,1,0,room);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;
 for(i=0;i<5*GAME_HZ;i++){g.ticks=1;boarding_step(&g,0);}assert(!g.subsystems[0][SUB_SENSORS].health);
 puts("PASS: imported subsystem rooms, visibility fallbacks, independent damage/repair, reinforced traversal, break/recovery, pause and sabotage.");return 0;
}
