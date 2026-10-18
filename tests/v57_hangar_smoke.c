/* V57: SD profile (unlocks, backup, validation), the 18 base-game layouts,
 * multi-ship saves, original unlock rules, hangar input, 2-4 crew setup and
 * repair drones working between fights. */
#include "campaign.h"
#include "save.h"
#include "profile.h"
#include "hangar.h"
#include "setup.h"
#include "drones.h"
#include "species.h"
#include "subsystems.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static Game g;
static void corrupt(const char*path){FILE*f=fopen(path,"r+b");assert(f);fseek(f,14,SEEK_SET);fputc(0x5a,f);fclose(f);}
int main(void){
 const char*path="tests/v57-profile.dat";
 remove(path);remove("tests/v57-profile.dat.bak");remove("tests/v57-profile.dat.tmp");
 /* Profile: defaults, round trip, backup restore, rejection of bad files. */
 Profile p,q;assert(profile_load(&p,path)==0&&p.unlocked_a==1u&&!p.unlocked_b);
 assert(profile_layout_unlocked(&p,0)&&!profile_layout_unlocked(&p,1)&&!profile_layout_unlocked(&p,6));
 assert(profile_unlock_layout(&p,6)&&!profile_unlock_layout(&p,6)&&profile_save(&p,path));
 assert(profile_load(&q,path)==1&&q.unlocked_a==((1u<<3)|1u));
 assert(profile_unlock_layout(&p,1)&&profile_save(&p,path));            /* Previous good file becomes the .bak. */
 corrupt(path);assert(profile_load(&q,path)==2&&q.unlocked_a==((1u<<3)|1u)&&!q.unlocked_b);
 corrupt("tests/v57-profile.dat.bak");assert(profile_load(&q,path)==0&&q.unlocked_a==1u);
 p.unlocked_a=0;assert(!profile_save(&p,path)); /* The Kestrel can never be locked. */
 remove(path);remove("tests/v57-profile.dat.bak");
 /* All 18 layouts start with their original loadout. */
 assert(game_layout_count()==18);
 for(int k=0;k<18;k++){GameLayoutInfo info;assert(game_init_ship(&g,k)&&game_layout_info(k,&info));
  assert(g.player_ship==k&&g.player.hull==30&&g.player.reactor==info.reactor&&g.crew_count==info.crew_count&&g.drone_parts==info.drone_parts);
  assert(g.weapon_count==(info.weapon_count<g.weapon_slots?info.weapon_count:g.weapon_slots)&&g.pilot_room>=0&&g.player.systems[SYS_WEAPONS].capacity>0);
  for(int i=0;i<g.crew_count;i++)assert(g.crew[i].species==info.crew[i]&&g.crew[i].health==species_max_health(info.crew[i]));
  assert(game_used_power(&g)<=g.player.reactor&&*game_layout_name(k)&&(k==0||*game_layout_unlock(k)));
  assert(game_layout_supported(k));}
 game_init_ship(&g,2);assert(!g.player.systems[SYS_SHIELDS].capacity&&g.player_cloak.capacity==1);       /* Stealth A: cloak, no shields. */
 game_init_ship(&g,4);assert(g.teleporter[0].installed&&game_augment_count(&g,AUG_CREW_STIMS));             /* Mantis A: teleporter. */
 game_init_ship(&g,6);assert(g.drones.installed&&g.drones.capacity==3&&g.drones.slot_count==3&&g.drones.slots[0].catalog_id==0&&g.drone_parts==15); /* Engi A. */
 game_init_ship(&g,13);assert(!g.subsystems[0][SUB_DOORS].capacity);                                     /* Rock B: no doors. */
 game_init_ship(&g,11);assert(!g.player.systems[SYS_MEDBAY].capacity&&g.teleporter[0].installed);          /* Slug B: no medbay. */
 /* Every playable layout fights 40 s against a Rebel Fighter without breaking invariants. */
 for(int k=0;k<18;k++){if(!game_layout_supported(k))continue;
  assert(game_init_ship(&g,k)&&game_begin_encounter(&g,ENEMY_REBEL,2,0));
  for(int w=0;w<g.weapon_count;w++){g.weapons[w].aim_room=g.enemy.rooms[0].id;}
  for(int t=0;t<40*GAME_HZ&&g.outcome==GAME_PLAYING;t++){game_step(&g);if(t%GAME_HZ==0){for(int w=0;w<g.weapon_count;w++){game_fire(&g,w,g.enemy.rooms[t%g.enemy.room_count].id);}}}
  assert(g.player.hull>=0&&g.player.hull<=g.player.max_hull&&game_used_power(&g)<=g.player.reactor+8);
  for(int i=0;i<g.crew_count;i++)assert(g.crew[i].health<=species_max_health(g.crew[i].species));}
 /* Multi-ship saves keep the layout; Kestrel saves keep their id. */
 assert(campaign_init_ship(&c,5701,6)&&c.game.player_ship==6&&!campaign_init_ship(&c,5701,18));
 assert(campaign_init_ship(&c,5701,6));
 assert(campaign_save(&c,"tests/v57.sav")&&campaign_load(&d,"tests/v57.sav")&&d.game.player_ship==6&&d.game.crew_count==3&&d.game.crew[1].species==SPEC_ENGI&&d.game.drones.installed);
 assert(campaign_init_ship(&c,5702,15));c.game.player.systems[SYS_MEDBAY].power=0;
 assert(campaign_save(&c,"tests/v57.sav")&&campaign_load(&d,"tests/v57.sav")&&d.game.player_ship==15&&d.game.player.reactor==5);
 campaign_init_seed(&c,5703);assert(campaign_save(&c,"tests/v57.sav")&&campaign_load(&d,"tests/v57.sav")&&d.game.player_ship==0);
 for(int k=0;k<18;k++){if(!game_layout_supported(k))continue;
  assert(campaign_init_ship(&c,5710+(unsigned)k,k));
  assert(campaign_save(&c,"tests/v57.sav")&&campaign_load(&d,"tests/v57.sav")&&d.game.player_ship==k&&d.game.crew_count==c.game.crew_count&&d.game.weapon_count==c.game.weapon_count);
  assert(d.game.drones.installed==c.game.drones.installed&&d.game.teleporter[0].installed==c.game.teleporter[0].installed&&d.game.player_cloak.capacity==c.game.player_cloak.capacity);
  for(int y=0;y<GAME_SYSTEM_COUNT;y++)assert(d.game.player.systems[y].capacity==c.game.player.systems[y].capacity&&d.game.player.systems[y].room==c.game.player.systems[y].room);}
 remove("tests/v57.sav");remove("tests/v57.sav.bak");
 /* Original unlock: reaching sector 5 in a Kestrel unlocks the Engi A (layout 6). */
 campaign_init_seed(&c,5704);
 for(int s=0;s<4;s++){assert(!c.unlock_pending);c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0)&&c.sector==s+1);}
 assert(c.unlock_pending==(1u<<6));
 assert(campaign_init_ship(&c,5705,6));for(int s=0;s<4;s++){c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0));}assert(!c.unlock_pending);
 /* Hangar: grid navigation, locked layouts refuse to start, unlocked ones start. */
 Hangar h;profile_defaults(&p);hangar_open(&h,&p);assert(hangar_layout(&h)==0&&hangar_can_start(&h));
 assert(hangar_key(&h,HANGAR_RIGHT)==HANGAR_NONE&&h.cls==1&&hangar_key(&h,HANGAR_A)==HANGAR_NONE&&h.active&&h.note[0]);
 hangar_key(&h,HANGAR_DOWN);assert(h.cls==4);hangar_key(&h,HANGAR_LEFT);assert(h.cls==3);hangar_key(&h,HANGAR_UP);hangar_key(&h,HANGAR_UP);assert(h.cls==6);
 hangar_key(&h,HANGAR_TOGGLE);assert(h.variant==1&&hangar_layout(&h)==13);
 profile_unlock_layout(&p,6);hangar_open(&h,&p);assert(hangar_touch(&h,4+84+10,24+25+5)==HANGAR_NONE&&h.cls==4);assert(hangar_touch(&h,20,24+25+5)==HANGAR_NONE&&h.cls==3);
 assert(hangar_can_start(&h)&&hangar_touch(&h,180,175)==HANGAR_START&&!h.active);
 hangar_open(&h,&p);assert(hangar_touch(&h,20,175)==HANGAR_BACK);
 /* Crew setup follows the layout's crew (four rows for four crew). */
 CrewSetup cs;crew_setup_begin_ship(&cs,8,4);assert(cs.count==4&&!strcmp(cs.names[3],"Morgan"));
 assert(crew_setup_touch(&cs,30,crew_setup_row_y(&cs,3)+5)==SETUP_NONE&&cs.editing&&cs.selected==3);
 /* Repair drones work between fights (service steps, as on the unpaused crew page). */
 game_init(&g);assert(drones_install(&g));g.drones.capacity=g.drones.health=4;g.player.reactor=20;assert(drones_equip(&g,0,5)&&drones_equip(&g,1,6));
 while(g.drones.power<3)assert(drones_adjust_power(&g,1));
 g.enemy.hull=0;g.outcome=GAME_VICTORY;g.player.hull=20;g.player.systems[SYS_ENGINES].health=0;g.player.systems[SYS_ENGINES].power=0;
 assert(drones_toggle(&g,0)&&drones_toggle(&g,1)&&g.drone_parts==0);
 {GameDamage none={0};(void)none;}
 for(int t=0;t<15*GAME_HZ;t++)game_service_step(&g);
 assert(g.player.hull>=23&&g.player.hull<=25&&g.player.systems[SYS_ENGINES].health>=1);
 drones_equip(&g,0,0);g.drone_parts=3;assert(!drones_toggle(&g,0)); /* Combat drones still need a fight. */
 printf("PASS: profile (backup, validation), 18 layouts, multi-ship saves, Engi unlock at sector 5, hangar, 4-crew setup, repair drones between fights.\n");
 return 0;
}
