#include "game.h"
#include "breach.h"
#include "fire.h"
#include "boarding.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static GameRoom*find(Ship*s,int id){int i;for(i=0;i<s->room_count;i++)if(s->rooms[i].id==id)return &s->rooms[i];return NULL;}
static void init(void){game_init(&g);g.player.systems[SYS_SHIELDS].power=0;g.player.shields=0;g.pilot_health=0;g.enemy_weapon.cooldown_ticks=999999;}
static void tick(int n){while(n--)game_step(&g);}
static void impact(GameDamage damage,int target){assert(game_launch_effect(&g,1,target,EFFECT_LASER,&damage,9));tick(12);}
int main(void){int room,health,crewhealth,i;GameRoom*r;
 init();{const int fire[]={10,10,10,30},breach[]={10,0,0,30};
  for(i=0;i<4;i++){assert(game_equip(&g,0,i));assert(g.weapons[0].fire_chance==fire[i]&&g.weapons[0].breach_chance==breach[i]);assert(g.weapons[0].system_damage==g.weapons[0].damage&&g.weapons[0].crew_damage==15*g.weapons[0].damage);}
  assert(g.weapons[0].power==1&&g.weapons[0].cooldown_ticks==9*GAME_HZ);}
 init();snapshot=g;{GameDamage d={1,1,5,0,0,0,0,0};
  assert(!game_launch_effect(&g,1,g.player.rooms[0].id,EFFECT_LASER,&d,-1));
  assert(!game_launch_effect(&g,1,g.player.rooms[0].id,-1,&d,0));
  assert(!game_launch_effect(&g,1,g.player.rooms[0].id,EFFECT_MISSILE+1,&d,0));
  assert(!memcmp(&g,&snapshot,sizeof g));}
 init();room=g.player.systems[SYS_WEAPONS].room;health=g.player.systems[SYS_WEAPONS].health;crewhealth=g.crew[2].health;
 impact((GameDamage){3,0,0,0,0,0,0,0},room);assert(g.player.hull==27&&g.player.systems[SYS_WEAPONS].health==health&&g.crew[2].health==crewhealth);
 init();room=g.player.systems[SYS_WEAPONS].room;health=g.player.systems[SYS_WEAPONS].health;
 impact((GameDamage){0,2,0,0,0,0,0,0},room);assert(g.player.hull==30&&g.player.systems[SYS_WEAPONS].health==health-2&&g.crew[2].health==100);
 init();room=g.player.systems[SYS_WEAPONS].room;
 impact((GameDamage){0,0,27,0,0,0,0,0},room);assert(g.player.hull==30&&g.player.systems[SYS_WEAPONS].health==3&&g.crew[2].health==73);
 /* Shields absorb all independent payload properties, not just hull damage. */
 init();room=g.player.systems[SYS_WEAPONS].room;g.player.systems[SYS_SHIELDS].power=2;g.player.shields=1;
 {GameDamage d={3,2,27,100,100,0,0,0};assert(game_launch_effect(&g,1,room,EFFECT_LASER,&d,0));}tick(12);
 r=find(&g.player,room);assert(g.player.hull==30&&g.player.systems[SYS_WEAPONS].health==3&&g.crew[2].health==100&&!r->fires&&!r->breaches);
 /* Real volleys snapshot their damage at the fire order, including follow-up shots. */
 init();g.player.systems[SYS_WEAPONS].power=3;g.weapons[1].charge_ticks=g.weapons[1].cooldown_ticks;
 assert(game_fire(&g,1,g.enemy.rooms[0].id));g.weapons[1].damage=9;g.weapons[1].system_damage=8;g.weapons[1].crew_damage=77;g.weapons[1].breach_chance=100;
 tick(8);{int shots=0;for(i=0;i<GAME_MAX_EFFECTS;i++)if(g.effects[i].active&&g.effects[i].side==0){assert(g.effects[i].damage==1&&g.effects[i].system_damage==1&&g.effects[i].crew_damage==15&&g.effects[i].breach_chance==0);shots++;}assert(shots==2);}
 init();room=g.player.systems[SYS_WEAPONS].room;impact((GameDamage){0,0,0,0,100,0,0,0},room);r=find(&g.player,room);assert(r->breaches==1);
 {int oxygen=r->oxygen;tick(1);assert(r->oxygen<=oxygen-12);}
 breach_add(&g.player,room,99);assert(r->breaches==4);breach_add(&g.player,room,-1);assert(r->breaches==4);
 /* A stationary friendly crew seals before repairing a damaged system. */
 init();room=g.player.systems[SYS_WEAPONS].room;r=find(&g.player,room);breach_add(&g.player,room,1);g.player.systems[SYS_WEAPONS].health=2;
 for(i=0;i<BREACH_SEAL_TICKS-1;i++)game_service_step(&g);
 assert(r->breaches==1&&g.player.systems[SYS_WEAPONS].health==2);game_service_step(&g);assert(!r->breaches&&!r->breach_repair_ticks&&g.player.systems[SYS_WEAPONS].health==2);
 /* Movement, fire and melee prevent sealing; pause is pure. */
 init();room=g.player.systems[SYS_WEAPONS].room;r=find(&g.player,room);breach_add(&g.player,room,1);g.crew[2].destination=0;breach_step(&g,0);assert(!r->breach_repair_ticks);
 g.crew[2].destination=room;r->fires=1;breach_step(&g,0);assert(!r->breach_repair_ticks);r->fires=0;
 game_init_crew(&g,1,0,room);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;breach_step(&g,0);assert(!r->breach_repair_ticks);
 r->fires=1;fire_step(&g,0);assert(!r->firefight_ticks);r->fires=0;
 g.enemy_crew[0].health=0;breach_step(&g,0);assert(r->breach_repair_ticks==1);g.paused=1;snapshot=g;breach_step(&g,0);assert(!memcmp(&g,&snapshot,sizeof g));
 /* Enemy crews seal their own hull; intruders cannot seal it, and service freezes it. */
 init();game_begin_encounter(&g,ENEMY_REBEL,0,0);room=g.enemy_pilot_room;r=find(&g.enemy,room);breach_add(&g.enemy,room,1);
 for(i=0;i<BREACH_SEAL_TICKS;i++)breach_step(&g,0);
 assert(!r->breaches);
 breach_add(&g.enemy,room,1);snapshot=g;breach_step(&g,1);assert(!memcmp(&g.enemy,&snapshot.enemy,sizeof g.enemy));
 g.enemy_crew[0].health=0;g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=room;breach_step(&g,0);assert(!r->breach_repair_ticks);
 /* Vacuum does not remove a breach. Sealing works in vacuum, with normal suffocation. */
 init();room=g.player.systems[SYS_WEAPONS].room;r=find(&g.player,room);r->oxygen=0;breach_add(&g.player,room,1);tick(GAME_HZ);assert(r->breaches==1&&r->oxygen==0&&g.crew[2].health==95);
 /* Connected open doors transmit exposure; closed doors protect neighbors. */
 init();room=g.player.systems[SYS_WEAPONS].room;r=find(&g.player,room);breach_add(&g.player,room,1);
 for(i=0;i<g.door_count;i++)if(g.doors[i].room_a==room||g.doors[i].room_b==room){int other=g.doors[i].room_a==room?g.doors[i].room_b:g.doors[i].room_a;if(other>=0){GameRoom*neighbor=find(&g.player,other);tick(1);assert(!neighbor->exposed);assert(game_toggle_door(&g,i));tick(1);assert(neighbor->exposed);break;}}
 assert(i<g.door_count);
 /* Powered medical healing is applied once while sealing, firefighting or fighting. */
 init();room=g.player.systems[SYS_MEDBAY].room;r=find(&g.player,room);g.player.systems[SYS_MEDBAY].power=1;
 g.crew[2].room=g.crew[2].destination=room;g.crew[2].health=50;breach_add(&g.player,room,1);
 tick(GAME_HZ);assert(g.crew[2].health==56&&r->breaches==1&&r->breach_repair_ticks==GAME_HZ);
 init();room=g.player.systems[SYS_MEDBAY].room;r=find(&g.player,room);g.player.systems[SYS_MEDBAY].power=1;
 g.crew[2].room=g.crew[2].destination=room;g.crew[2].health=50;r->fires=1;
 tick(GAME_HZ);assert(g.crew[2].health==54&&r->fires==1&&r->firefight_ticks==GAME_HZ);
 init();room=g.player.systems[SYS_MEDBAY].room;g.player.systems[SYS_MEDBAY].power=1;
 g.crew[2].room=g.crew[2].destination=room;g.crew[2].health=50;
 game_init_crew(&g,1,0,room);g.enemy_crew_count=1;g.enemy_crew[0].ship=0;
 tick(GAME_HZ);assert(g.crew[2].health==50); /* six healed, six melee damage */
 /* Captured-ship atmosphere and fire keep affecting survivors awaiting recall,
  * while enemy weapons, orders and AI remain frozen. */
 init();game_begin_encounter(&g,ENEMY_REBEL,0,0);g.outcome=GAME_VICTORY;g.result_reason=RESULT_ENEMY_CREW;
 for(i=0;i<g.enemy_crew_count;i++)g.enemy_crew[i].health=0;
 room=g.enemy_pilot_room;r=find(&g.enemy,room);g.crew[1].ship=1;g.crew[1].room=g.crew[1].destination=room;
 r->oxygen=0;snapshot=g;
 for(i=0;i<2*GAME_HZ;i++)game_service_step(&g);
 assert(r->oxygen>100&&!memcmp(&g.enemy_weapon,&snapshot.enemy_weapon,sizeof g.enemy_weapon)&&g.enemy_fire_ticks==snapshot.enemy_fire_ticks);
 for(i=0;i<g.enemy_crew_count;i++){assert(g.enemy_crew[i].death_processed);snapshot.enemy_crew[i].death_processed=1;} /* terminal bookkeeping only */
 assert(!memcmp(g.enemy_crew,snapshot.enemy_crew,sizeof g.enemy_crew));
 breach_add(&g.enemy,room,1);r->oxygen=0;crewhealth=g.crew[1].health;
 for(i=0;i<GAME_HZ;i++)game_service_step(&g);
 assert(r->oxygen==0&&r->breaches==1&&g.crew[1].health==crewhealth-5&&!r->breach_repair_ticks);
 r->breaches=0;r->oxygen=1000;r->fires=1;r->fire_ticks=0;crewhealth=g.crew[1].health;
 for(i=0;i<GAME_HZ;i++)game_service_step(&g);
 assert(g.crew[1].health==crewhealth-2&&r->fires==1&&!r->firefight_ticks);
 puts("PASS: independent queued damage, shield filtering, breaches, leak/exposure, sealing priority, faction, interruption, pause and vacuum.");return 0;
}
