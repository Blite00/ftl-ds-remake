/* V59: the base-game ship achievements (each gated to its cruiser), run progress
 * in save30, the profile merge and Type B unlocks (2 of 3). */
#include "campaign.h"
#include "save.h"
#include "profile.h"
#include "achievements.h"
#include "drones.h"
#include "species.h"
#include "boarding.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static Game g;
static int has(const Game*x,int id){return !!(x->ach.earned&(1u<<id));}
static void fight(int layout){assert(game_init_ship(&g,layout)&&game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy_weapon.cooldown_ticks=g.enemy_weapon_secondary.cooldown_ticks=999999;}
static void finish(void){g.enemy.hull=0;achievements_step(&g);}
static void advance(Campaign*x,int sectors){for(int s=0;s<sectors;s++){x->mode=CAM_EVENT;x->event_id=EVENT_EXIT;x->event_resolved=0;assert(campaign_choice(x,0));}}
int main(void){
 assert(ACH_COUNT==27&&*achievement_name(ACH_KESTREL_TOUGH)&&!strcmp(achievement_name(ACH_KESTREL_TOUGH),"Tough Little Ship"));
 /* Achievements only count on their own cruiser. */
 fight(2);achievements_earn(&g,ACH_KESTREL_TOUGH);assert(!g.ach.earned);
 /* Kestrel: six species, eleven systems, back to full hull from 1 HP. */
 fight(0);g.crew_count=6;for(int i=0;i<6;i++){game_init_crew(&g,0,i,g.player.rooms[i].id);game_set_crew_species(&g,0,i,i);}achievements_step(&g);assert(has(&g,ACH_KESTREL_FEDERATION));
 fight(0);assert(boarding_install(&g,0)&&drones_install(&g)&&game_cloak_install(&g));achievements_step(&g);assert(has(&g,ACH_KESTREL_ARSENAL));
 fight(0);g.player.hull=1;achievements_step(&g);assert(g.tough_low&&!has(&g,ACH_KESTREL_TOUGH));g.player.hull=g.player.max_hull;achievements_step(&g);assert(has(&g,ACH_KESTREL_TOUGH));
 /* Stealth: kill during one cloak from full health; dodge 9 damage while cloaked. */
 fight(2);g.player.reactor=20;game_cloak_power(&g,1);assert(game_cloak_activate(&g)&&g.ach.cloak_full);finish();assert(has(&g,ACH_STEALTH_DESTROY));
 fight(2);g.player.reactor=20;game_cloak_power(&g,1);assert(game_cloak_activate(&g));
 {GameEffect e;memset(&e,0,sizeof e);e.side=1;e.result=EFFECT_MISS;e.damage=5;achievements_effect_resolved(&g,&e,10,0);assert(!has(&g,ACH_STEALTH_AVOID));achievements_effect_resolved(&g,&e,10,0);assert(has(&g,ACH_STEALTH_AVOID));}
 /* Mantis: five kills without damage; last crewmember kills the last enemy aboard. */
 fight(4);g.ach.enemy_kills=5;achievements_step(&g);assert(has(&g,ACH_MANTIS_SLAUGHTER));
 fight(4);for(int i=1;i<g.crew_count;i++){g.crew[i].health=0;}g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=g.enemy.rooms[0].id;
 for(int i=1;i<g.enemy_crew_count;i++){g.enemy_crew[i].health=0;}achievements_step(&g);assert(!has(&g,ACH_MANTIS_SURVIVOR));g.enemy_crew[0].health=0;achievements_step(&g);assert(has(&g,ACH_MANTIS_SURVIVOR));
 /* Engi: three drones out; four enemy systems ioned; a kill by drones only. */
 fight(6);g.player.reactor=30;g.drones.capacity=g.drones.health=8;g.drone_parts=5;for(int i=0;i<3;i++){assert(drones_equip(&g,i,0));}while(g.drones.power<6){assert(drones_adjust_power(&g,1));}
 for(int i=0;i<3;i++){assert(drones_toggle(&g,i));}achievements_step(&g);assert(has(&g,ACH_ENGI_ROBOTIC));
 fight(6);{int n=0;for(int i=0;i<GAME_SYSTEM_COUNT&&n<4;i++)if(g.enemy.systems[i].capacity>0){g.enemy.systems[i].ion_bars=1;n++;}assert(n==4);}achievements_step(&g);assert(has(&g,ACH_ENGI_IONED));
 fight(6);g.ach.used=USED_DRONE;finish();assert(has(&g,ACH_ENGI_ONLY_DRONES));
 fight(6);g.ach.used=USED_DRONE|USED_WEAPON;finish();assert(!has(&g,ACH_ENGI_ONLY_DRONES));
 /* Federation: artillery-only kill without hull damage. */
 fight(8);g.ach.used=USED_ARTILLERY;finish();assert(has(&g,ACH_FED_PATIENCE));
 fight(8);g.ach.used=USED_ARTILLERY;g.ach.hull_hit=1;finish();assert(!has(&g,ACH_FED_PATIENCE));
 /* Slug: full enemy vision without sensors (Advanced Scanners); three kills with one Anti-Bio beam. */
 fight(10);assert(game_sensor_level(&g)<=0);game_add_augment(&g,AUG_ADV_SCANNERS);achievements_step(&g);assert(has(&g,ACH_SLUG_VISION));
 fight(10);{GameEffect e;memset(&e,0,sizeof e);e.side=0;e.origin=ORIGIN_BIO;int alive=achievements_enemy_alive(&g);assert(alive>=3);for(int i=0;i<3;i++){g.enemy_crew[i].health=0;}achievements_effect_resolved(&g,&e,g.enemy.hull,alive);assert(has(&g,ACH_SLUG_BIO));}
 /* Rock: crew kill a burning enemy aboard; missile-only kill past a defense drone. */
 fight(12);{int room=g.enemy_crew[0].room;GameRoom*r=0;for(int i=0;i<g.enemy.room_count;i++)if(g.enemy.rooms[i].id==room)r=&g.enemy.rooms[i];assert(r);r->fires=1;
  g.crew[0].ship=1;g.crew[0].room=g.crew[0].destination=room;g.enemy_crew[0].health=0;achievements_step(&g);assert(has(&g,ACH_ROCK_FIRE));}
 fight(12);g.ach.defense_drone=1;g.ach.used=USED_MISSILE;finish();assert(has(&g,ACH_ROCK_MISSILES));
 /* Zoltan: kill before the Zoltan shield breaks; 29 power in systems. */
 fight(14);assert(g.ach.start_shield==5);finish();assert(has(&g,ACH_ZOLTAN_SHIELDS));
 fight(14);g.player_super_shield=0;achievements_step(&g);g.enemy.hull=0;achievements_step(&g);assert(!has(&g,ACH_ZOLTAN_SHIELDS));
 fight(14);for(int i=0;i<GAME_SYSTEM_COUNT;i++){g.player.systems[i].power=6;}achievements_step(&g);assert(has(&g,ACH_ZOLTAN_POWER));
 /* Crystal: shard kill; four enemies locked down in one room. */
 fight(16);g.ach.killing_origin=ORIGIN_SHARD;finish();assert(has(&g,ACH_CRYSTAL_SHARD));
 fight(16);{int room=g.enemy.rooms[0].id;g.enemy.rooms[0].lockdown_ticks=100;g.enemy_crew_count=4;for(int i=0;i<4;i++){game_init_crew(&g,1,i,room);}achievements_step(&g);assert(has(&g,ACH_CRYSTAL_LOCKDOWN));}
 /* Run progress: Federation/Zoltan reach sector 5 unupgraded; Stealth sector 8 without hazards. */
 assert(campaign_init_ship(&c,5901,8));advance(&c,4);assert(has(&c.game,ACH_FED_UPGRADE));
 assert(campaign_init_ship(&c,5902,8));c.mode=CAM_MAP;c.scrap=500;assert(campaign_upgrade(&c,SYS_WEAPONS)&&(c.ach_flags&ACH_FLAG_WEAPONS));advance(&c,4);assert(!has(&c.game,ACH_FED_UPGRADE));
 assert(campaign_init_ship(&c,5903,14));advance(&c,4);assert(has(&c.game,ACH_ZOLTAN_MANPOWER));
 assert(campaign_init_ship(&c,5904,2));advance(&c,7);assert(c.sector==7&&has(&c.game,ACH_STEALTH_TACTICAL));
 assert(campaign_init_ship(&c,5905,2));c.ach_flags|=ACH_FLAG_HAZARD;advance(&c,7);assert(!has(&c.game,ACH_STEALTH_TACTICAL));
 /* Mantis: 20 crews killed by sector 6 (the 20th win here). */
 assert(campaign_init_ship(&c,5906,4));c.ach_counter[ACH_RUN_CREW_SHIPS]=19;assert(game_begin_encounter(&c.game,ENEMY_REBEL,1,0));c.mode=CAM_COMBAT;
 c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_CREW;campaign_step(&c);assert(c.ach_counter[ACH_RUN_CREW_SHIPS]==20&&has(&c.game,ACH_MANTIS_CREW_DEAD));
 /* Save30 keeps run progress and the 1-HP latch. */
 assert(campaign_init_ship(&c,5907,0));c.ach_counter[ACH_RUN_NEBULA]=12;c.ach_counter[ACH_RUN_ROCK_SHIPS]=3;c.ach_flags=ACH_FLAG_HAZARD|ACH_FLAG_REACTOR;c.game.tough_low=1;
 assert(campaign_save(&c,"tests/v59.sav")&&campaign_load(&d,"tests/v59.sav")&&d.ach_counter[ACH_RUN_NEBULA]==12&&d.ach_counter[ACH_RUN_ROCK_SHIPS]==3&&d.ach_flags==(ACH_FLAG_HAZARD|ACH_FLAG_REACTOR)&&d.game.tough_low==1);
 remove("tests/v59.sav");remove("tests/v59.sav.bak");
 /* Profile: 2 of 3 unlock the Type B layout; duplicates are not new. */
 Profile p;profile_defaults(&p);assert(profile_add_achievements(&p,1u<<ACH_KESTREL_TOUGH)==(1u<<ACH_KESTREL_TOUGH)&&!profile_layout_unlocked(&p,1));
 assert(!profile_add_achievements(&p,1u<<ACH_KESTREL_TOUGH)&&profile_add_achievements(&p,1u<<ACH_KESTREL_ARSENAL)&&profile_layout_unlocked(&p,1)&&profile_class_achievements(&p,0)==2);
 assert(profile_add_achievements(&p,(1u<<ACH_ENGI_ROBOTIC)|(1u<<ACH_ENGI_IONED))&&profile_layout_unlocked(&p,7)&&!profile_layout_unlocked(&p,6));
 assert(profile_save(&p,"tests/v59-profile.dat")&&profile_load(&p,"tests/v59-profile.dat")==1&&profile_class_achievements(&p,3)==2);
 p.achievements[0]|=1u<<30;assert(!profile_save(&p,"tests/v59-profile.dat"));
 remove("tests/v59-profile.dat");remove("tests/v59-profile.dat.bak");
 printf("PASS: ship achievements (Kestrel, Stealth, Mantis, Engi, Federation, Slug, Rock, Zoltan, Crystal), run progress in save30, profile Type B unlocks.\n");
 return 0;
}
