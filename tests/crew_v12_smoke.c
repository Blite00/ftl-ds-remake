#include "game.h"
#include "fire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,copy;
static void steps(int n){while(n--)game_step(&g);}
static void safe_enemy(void){g.enemy_weapon.cooldown_ticks=1000000;g.enemy_weapon.shots=0;}
int main(void){int i,j,room;unsigned id;
 game_init(&g);
 {int shield=g.player.systems[SYS_SHIELDS].room;g.player.shields=1;g.player.shield_ticks=12;
  fire_ignite(&g.player,shield,1);g.ticks=3*GAME_HZ;fire_step(&g,0);
  assert(g.player.shields==0&&g.player.shield_ticks==0);}
 game_init(&g);assert(g.crew[0].id>0&&g.next_crew_id>g.crew[2].id);
 assert(game_rename_crew(&g,0,"  Jane-Doe  ")&&!strcmp(g.crew[0].name,"Jane-Doe"));
 assert(!game_rename_crew(&g,0,"   ")&&!game_rename_crew(&g,0,"bad!"));
 assert(!game_rename_crew(&g,0,"1234567890123"));
 id=g.crew[0].id;assert(game_save_stations(&g)==3);room=g.crew[0].room;
 assert(game_order_crew(&g,0,g.player.systems[SYS_WEAPONS].room));assert(!strcmp(game_crew_action(&g,0),"WALK"));
 assert(game_return_stations(&g)==3&&g.crew[0].destination==room);
 game_begin_encounter(&g,ENEMY_REBEL,0,0);safe_enemy();
 assert(g.crew[0].id==id&&!strcmp(g.crew[0].name,"Jane-Doe"));
 assert(g.enemy_crew_count==3&&!g.enemy.automated&&g.enemy_door_count>0);
 for(i=0;i<g.enemy_crew_count;i++){assert(g.enemy_crew[i].ship==1&&g.enemy_crew[i].id>id);for(j=0;j<g.enemy.room_count;j++)assert(game_next_room(&g,1,g.enemy_crew[i].room,g.enemy.rooms[j].id)>=0);}
 g.paused=1;copy=g;game_step(&g);assert(!memcmp(&g,&copy,sizeof g));g.paused=0;
 /* Enemy medical AI finds imported route and heals below 25% health. */
 g.enemy_crew[0].health=20;steps(15*GAME_HZ);assert(g.enemy_crew[0].health>=65);
 {int power=0;for(i=0;i<GAME_SYSTEM_COUNT;i++)power+=g.enemy.systems[i].power;assert(power<=g.enemy.reactor);}
 /* Enemy damage repair is performed by living crew, not automated repair. */
 g.enemy.systems[SYS_OXYGEN].health=g.enemy.systems[SYS_OXYGEN].power=0;
 steps(20*GAME_HZ);assert(g.enemy.systems[SYS_OXYGEN].health==1);
 copy=g;game_service_step(&g);assert(!memcmp(&g.enemy,&copy.enemy,sizeof g.enemy));assert(!memcmp(g.enemy_crew,copy.enemy_crew,sizeof g.enemy_crew));
 /* Enemy fire injury and friendly firefighting use ship/faction, not room ID alone. */
 game_begin_encounter(&g,ENEMY_REBEL,0,0);safe_enemy();
 room=g.enemy_crew[0].room;fire_ignite(&g.enemy,room,1);
 for(i=0;i<3*GAME_HZ;i++){g.ticks++;fire_step(&g,0);}
 assert(g.enemy_crew[0].health<100);for(i=0;i<g.crew_count;i++)assert(g.crew[i].health>0);
 /* Damage on enemy ship hits located enemy crew, not player crew with same room ID. */
 g.enemy.shields=0;g.enemy.systems[SYS_SHIELDS].power=0;g.enemy.systems[SYS_ENGINES].power=0;
 g.effects[0].active=1;g.effects[0].duration=1;g.effects[0].side=0;g.effects[0].room=g.enemy_crew[1].room;g.effects[0].damage=1;g.effects[0].system_damage=1;g.effects[0].crew_damage=5;g.effects[0].piercing=9;
 i=g.enemy_crew[1].health;game_step(&g);assert(g.enemy_crew[1].health<=i-5);
 for(i=0;i<g.enemy_crew_count;i++)g.enemy_crew[i].health=0;
 game_step(&g);assert(g.outcome==GAME_VICTORY&&g.result_reason==RESULT_ENEMY_CREW&&g.enemy.hull>0);
 game_begin_encounter(&g,ENEMY_SCOUT,0,0);assert(g.enemy_crew_count==0&&g.enemy.automated);game_step(&g);assert(g.outcome==GAME_PLAYING);
 /* Death by suffocation still applies to friendly crew located on another ship. */
 g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=g.enemy.rooms[0].id;g.crew[0].health=5;
 g.ticks=GAME_HZ-1;game_step(&g);assert(g.crew[0].health==0);
 puts("PASS: named crew identities, stations, side routing, living enemy AI, power, healing, repair, fires and crew-only outcome.");return 0;
}
