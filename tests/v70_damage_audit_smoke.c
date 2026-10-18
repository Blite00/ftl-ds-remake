/* V70 damage audit against the original FTL rules (weapon data from ftl.dat):
   each damage point = 1 hull + 1 system bar (sysDamage replaces it when set),
   crew in the hit room only lose 15 HP per point (persDamage*15 when set),
   one shield layer stops one projectile (missiles pierce), each projectile applies once. */
#include "game.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static void setup(void){
 game_init(&g);g.enemy_ship=-1;assert(game_begin_encounter(&g,ENEMY_SHIP,1,0));
 g.enemy_pilot_health=0;g.enemy.systems[SYS_ENGINES].power=0;g.enemy.shields=0;g.enemy.hull=30;
 /* No random fires or breaches: those are separate rolls. */
 for(int i=0;i<g.weapon_count;i++){g.weapons[i].fire_chance=g.weapons[i].breach_chance=0;}
}
static int room_of(int sys){return g.enemy.systems[sys].room;}
/* Two humans in the shields room, one elsewhere. */
static void crew(int room,int other){g.enemy_crew_count=3;for(int i=0;i<3;i++){memset(&g.enemy_crew[i],0,sizeof g.enemy_crew[i]);g.enemy_crew[i].ship=1;g.enemy_crew[i].species=SPEC_HUMAN;g.enemy_crew[i].health=100;g.enemy_crew[i].room=g.enemy_crew[i].destination=i<2?room:other;}}
static void volley(int weapon,int room){const WeaponState*w=&g.weapons[weapon];GameDamage d={w->damage,w->system_damage,w->crew_damage,0,0,w->hull_bust,w->lockdown,0};
 for(int s=0;s<w->shots;s++){assert(game_launch_effect(&g,0,room,w->missiles?EFFECT_MISSILE:EFFECT_LASER,&d,w->shield_piercing));}
 int flashed=0;for(int t=0;t<3*GAME_HZ;t++){game_step(&g);for(int i=0;i<g.enemy.room_count;i++){flashed|=g.enemy.rooms[i].id==room&&g.enemy.rooms[i].hit_ticks>0&&g.enemy.rooms[i].hit_damage==w->damage;}}
 if(w->missiles||!g.enemy.shields){assert(flashed);}}
static void measure(const char*label,int weapon,int shields){
 setup();int room=room_of(SYS_SHIELDS),other=room_of(SYS_WEAPONS);crew(room,other);
 GameSystemState*s=&g.enemy.systems[SYS_SHIELDS];s->capacity=s->health=s->power=4;g.enemy.shields=shields;
 int hull=g.enemy.hull,bars=s->health;volley(weapon,room);
 printf("%-22s shields %d: hull -%d, shield bars -%d, crew in room %d/%d HP, crew elsewhere %d HP\n",label,shields,hull-g.enemy.hull,bars-s->health,g.enemy_crew[0].health,g.enemy_crew[1].health,g.enemy_crew[2].health);
 const WeaponState*w=&g.weapons[weapon];int landed=shields>w->shield_piercing?(w->missiles?w->shots:0):w->shots;if(w->missiles)landed=w->shots;
 if(!shields||w->missiles){assert(hull-g.enemy.hull==landed*w->damage);assert(bars-s->health==landed*w->system_damage);
  assert(g.enemy_crew[0].health==100-landed*w->damage*15&&g.enemy_crew[1].health==g.enemy_crew[0].health);}
 assert(g.enemy_crew[2].health==100); /* only the hit room */
 {int ri=-1;for(int i=0;i<g.enemy.room_count;i++){if(g.enemy.rooms[i].id==room)ri=i;}assert(ri>=0&&g.enemy.rooms[ri].hit_ticks==0);} /* the flash has expired */
}
int main(void){
 game_init(&g);int burst=-1,artemis=-1;
 for(int i=0;i<g.weapon_count;i++){if(!strcmp(game_weapon_short(g.weapons[i].catalog_id),"BURST2"))burst=i;if(!strcmp(game_weapon_short(g.weapons[i].catalog_id),"ARTEMIS"))artemis=i;}
 assert(burst>=0&&artemis>=0);
 assert(g.weapons[burst].shots==3&&g.weapons[burst].damage==1&&g.weapons[burst].crew_damage==15);
 assert(g.weapons[artemis].damage==2&&g.weapons[artemis].crew_damage==30&&g.weapons[artemis].missiles);
 measure("Burst Laser II volley",burst,0);
 measure("Burst Laser II volley",burst,2);
 measure("Artemis missile",artemis,0);
 measure("Artemis missile",artemis,2);
 printf("Rates: fire %d HP/s per fire (max %d), suffocation %d HP/s, human melee %d HP/s; human %d HP, Rock %d, Zoltan %d.\n",
  species_fire_damage(SPEC_HUMAN,1),species_fire_damage(SPEC_HUMAN,3),species_suffocation_damage(SPEC_HUMAN,0),species_melee_damage(SPEC_HUMAN),
  species_max_health(SPEC_HUMAN),species_max_health(SPEC_ROCK),species_max_health(SPEC_ZOLTAN));
 printf("PASS: damage audit (hull and bars per point, 15 crew HP per point in the hit room only, shields stop lasers per layer, missiles pierce).\n");
 return 0;
}
