#include "game.h"
#include "beam.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,snapshot;
static void init(int layers){int i;game_init(&g);memset(&g.enemy,0,sizeof g.enemy);g.enemy.hull=g.enemy.max_hull=100;g.enemy.automated=1;g.enemy.room_count=4;
 for(i=0;i<4;i++){GameRoom*r=&g.enemy.rooms[i];r->id=i;r->x=i%2;r->y=i/2;r->w=r->h=1;r->system=-1;}
 for(i=0;i<GAME_SYSTEM_COUNT;i++)g.enemy.systems[i].room=-1;
 g.enemy.systems[SYS_SHIELDS].health=g.enemy.systems[SYS_SHIELDS].power=layers*2;g.enemy.shields=layers;
 g.enemy.systems[SYS_ENGINES].health=g.enemy.systems[SYS_ENGINES].power=20;g.enemy_weapon.cooldown_ticks=999999;g.enemy_weapon.shots=0;g.enemy_crew_count=0;
}
static void tick(int n){while(n--)game_step(&g);}
int main(void){int i,x,y;GameDamage damage={2,2,10,0,0,0,0,0};
 init(0);assert(beam_intersects_room(&g.enemy.rooms[0],0,16,64,16));assert(beam_intersects_room(&g.enemy.rooms[1],0,16,64,16));
 assert(!beam_intersects_room(&g.enemy.rooms[2],0,16,64,16));assert(!beam_intersects_room(&g.enemy.rooms[1],0,0,64,64));assert(!beam_intersects_room(&g.enemy.rooms[2],0,0,64,64));
 assert(beam_intersects_room(&g.enemy.rooms[0],0,0,64,64)&&beam_intersects_room(&g.enemy.rooms[3],0,0,64,64));
 assert(!beam_valid_line(&g.enemy,0,32,64,32)&&!beam_valid_line(&g.enemy,-1,16,64,16)&&!beam_valid_line(&g.enemy,0,0,0,0));
 x=1000;y=1000;assert(beam_clamp_line(0,0,&x,&y,73)&&x*x+y*y<=73*73);
 /* Beam crosses two rooms once each, ignores 100% engine evasion. */
 assert(game_launch_beam(&g,0,0,16,64,16,&damage));tick(12);assert(g.enemy.hull==96&&g.effects[0].result==EFFECT_HIT);
 init(1);assert(game_launch_beam(&g,0,0,16,64,16,&damage));tick(12);assert(g.enemy.hull==98&&g.enemy.shields==1);
 init(2);assert(game_launch_beam(&g,0,0,16,64,16,&damage));tick(12);assert(g.enemy.hull==100&&g.enemy.shields==2&&g.effects[0].result==EFFECT_SHIELD);
 /* Shield attenuation is fixed for the whole impact, even if its first room
  * contains shield control and suffers system damage. */
 init(1);g.enemy.rooms[0].system=SYS_SHIELDS;g.enemy.systems[SYS_SHIELDS].room=0;
 assert(game_launch_beam(&g,0,0,16,64,16,&damage));tick(12);assert(g.enemy.hull==98&&g.enemy.systems[SYS_SHIELDS].health==1);
 /* Exact source weapon values, explicit point target, queued endpoint snapshot. */
 init(0);assert(game_equip(&g,0,5));assert(g.weapons[0].power==3&&g.weapons[0].cooldown_ticks==17*GAME_HZ&&g.weapons[0].beam_length==73);
 g.weapons[1].enabled=0;g.player.systems[SYS_WEAPONS].power=3;g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
 assert(!game_fire(&g,0,0));assert(game_set_beam_target(&g,0,0,16,64,16));assert(game_fire(&g,0,-1));assert(!game_fire(&g,0,-1));
 assert(game_set_beam_target(&g,0,16,0,16,64));g.weapons[0].damage=9;tick(1);
 assert(g.effects[0].kind==EFFECT_BEAM&&g.effects[0].beam_x0==0&&g.effects[0].beam_y0==16&&g.effects[0].beam_x1==64&&g.effects[0].damage==2);
 g.paused=1;snapshot=g;game_step(&g);assert(!memcmp(&g,&snapshot,sizeof g));g.paused=0;tick(12);assert(g.enemy.hull==96);
 /* A saturated effect pool leaves one pending beam and retries without extra cost. */
 init(0);game_equip(&g,0,5);g.weapons[1].enabled=0;g.player.systems[SYS_WEAPONS].power=3;g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;assert(game_set_beam_target(&g,0,0,16,64,16));
 for(i=0;i<GAME_MAX_EFFECTS;i++){g.effects[i].active=1;g.effects[i].duration=9999;}
 assert(game_fire(&g,0,0));tick(2);assert(g.weapons[0].pending_shots==1);g.effects[3].active=0;tick(1);assert(g.weapons[0].pending_shots==0&&g.effects[3].kind==EFFECT_BEAM);tick(12);assert(g.enemy.hull==96);
 /* Enemy beam scheduling uses its bounded source length and the same effect type. */
 init(0);g.enemy.systems[SYS_WEAPONS].power=g.enemy.systems[SYS_WEAPONS].health=3;g.enemy_weapon.beam_length=73;g.enemy_weapon.power=3;g.enemy_weapon.cooldown_ticks=1;g.enemy_weapon.damage=g.enemy_weapon.system_damage=2;g.enemy_weapon.crew_damage=10;
 tick(1);assert(g.effects[0].active&&g.effects[0].kind==EFFECT_BEAM&&g.effects[0].side==1);
 puts("PASS: bounded point beams, strict interior intersections, no evasion, per-room/shield attenuation, queued snapshots, pause and effect-pool retry.");return 0;
}
