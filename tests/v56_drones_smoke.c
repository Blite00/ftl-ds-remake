/* V56: the remaining base-game drones (Combat II, Defense II, Beam, Hull Repair,
 * System Repair, Anti-Personnel, Boarding), store offer and event rewards. */
#include "campaign.h"
#include "drones.h"
#include "species.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
static Game g;
static void setup(int id){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));assert(drones_install(&g));g.drones.capacity=g.drones.health=4;g.player.reactor=30;g.drone_parts=5;
 assert(drones_equip(&g,0,id));while(g.drones.power<drones_power(id))assert(drones_adjust_power(&g,1));assert(drones_toggle(&g,0)&&g.drones.slots[0].deployed&&g.drone_parts==4);
 g.enemy_weapon.cooldown_ticks=g.enemy_weapon_secondary.cooldown_ticks=999999;}
static int count_effects(int side,int kind){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==side&&g.effects[i].kind==kind;return n;}
static int add_intruder(int ship,int room){int slot=g.enemy_crew_count;assert(slot<GAME_MAX_ENEMY_CREW);game_init_crew(&g,1,slot,room);game_set_crew_species(&g,1,slot,SPEC_HUMAN);g.enemy_crew[slot].ship=ship;g.enemy_crew[slot].station=room;g.enemy_crew_count=slot+1;return slot;}
int main(void){
 assert(GAME_DRONE_CATALOG_COUNT==9&&!strcmp(drones_name(0),"Combat Drone Mark I")&&!strcmp(drones_name(8),"Boarding Drone"));
 for(int id=0;id<GAME_DRONE_CATALOG_COUNT;id++)assert(drones_kind(id)>=0&&drones_rarity(id)>=1&&drones_price(id)>0&&drones_power(id)>0);
 /* Combat II fires every 6 s. */
 setup(2);for(int t=0;t<6*GAME_HZ;t++)drones_step(&g);assert(count_effects(0,EFFECT_LASER)==1);
 /* Defense II shoots down an incoming laser; Defense I ignores lasers. */
 setup(3);{GameDamage hit={1,1,10,0,0,0,0,0};assert(game_launch_effect(&g,1,g.player.rooms[0].id,EFFECT_LASER,&hit,0));
  for(int t=0;t<GAME_HZ;t++){drones_intercept(&g);}
  int caught=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)caught+=g.effects[i].active&&g.effects[i].result==EFFECT_INTERCEPT;assert(caught==1);}
 setup(1);{GameDamage hit={1,1,10,0,0,0,0,0};assert(game_launch_effect(&g,1,g.player.rooms[0].id,EFFECT_LASER,&hit,0));
  for(int t=0;t<2*GAME_HZ;t++){drones_intercept(&g);}
  for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g.effects[i].active||g.effects[i].result!=EFFECT_INTERCEPT);}
 /* Beam drone sweeps the enemy every 10 s. */
 setup(4);for(int t=0;t<10*GAME_HZ;t++)drones_step(&g);assert(count_effects(0,EFFECT_BEAM)==1);
 /* Hull repair: 3-5 hull, then the part is spent. */
 setup(5);g.player.hull=10;for(int t=0;t<20*GAME_HZ;t++)drones_step(&g);assert(g.player.hull>=13&&g.player.hull<=15&&!g.drones.slots[0].deployed);
 /* System repair walks to a damaged system and repairs it at the crew rate. */
 setup(6);g.player.systems[SYS_SHIELDS].health=0;g.player.systems[SYS_SHIELDS].power=0;
 for(int t=0;t<12*GAME_HZ&&g.player.systems[SYS_SHIELDS].health<g.player.systems[SYS_SHIELDS].capacity;t++)drones_step(&g);
 assert(g.drones.slots[0].unit_room==g.player.systems[SYS_SHIELDS].room&&g.player.systems[SYS_SHIELDS].health==g.player.systems[SYS_SHIELDS].capacity);
 /* Anti-personnel drone hunts and kills an intruder. */
 setup(7);{int room=g.player.systems[SYS_OXYGEN].room,k=add_intruder(0,room);for(int t=0;t<40*GAME_HZ&&g.enemy_crew[k].health>0;t++)drones_step(&g);
  assert(g.enemy_crew[k].health<=0&&g.drones.slots[0].deployed&&g.drones.slots[0].unit_room==room);}
 /* Boarding drone: 3 s flight, breach, then sabotage of enemy systems. */
 setup(8);for(int t=0;t<3*GAME_HZ;t++){drones_step(&g);}
 {int room=g.drones.slots[0].unit_room,before=0,after=0;assert(room>=0);int breaches=0;for(int i=0;i<g.enemy.room_count;i++){breaches+=g.enemy.rooms[i].breaches;}assert(breaches>=1);
  for(int i=0;i<g.enemy_crew_count;i++)g.enemy_crew[i].health=0; /* No defenders: pure sabotage. */
  for(int k=0;k<GAME_SYSTEM_COUNT;k++){before+=g.enemy.systems[k].health;}
  for(int t=0;t<12*GAME_HZ;t++){drones_step(&g);}
  for(int k=0;k<GAME_SYSTEM_COUNT;k++){after+=g.enemy.systems[k].health;}assert(after<before);}
 /* Suspending a boarding drone loses it; reset clears units. */
 assert(drones_toggle(&g,0)&&!g.drones.slots[0].deployed);drones_reset(&g);assert(g.drones.slots[0].unit_room==-1);
 /* Store offer 14 sells a drone schematic of store rarity. */
 campaign_init_seed(&c,5601);c.mode=CAM_STORE;c.scrap=500;{int id=campaign_offer_drone(&c);assert(id>=1&&id<GAME_DRONE_CATALOG_COUNT&&campaign_offer_price(&c,14)==drones_price(id));
  assert(campaign_purchase(&c,14));int found=0;for(int i=0;i<CAM_CARGO_COUNT;i++)found|=c.drone_cargo[i]==id;assert(found);}
 /* Events: drone rewards are compiled. */
 int rewards=0;for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT;i++){rewards+=imported_event_effects[i].kind==IE_DRONE;}assert(rewards>0);
 printf("PASS: drones (Combat II, Defense II, Beam, Hull Repair, System Repair, Anti-Personnel, Boarding), store drone offer, %d event drone rewards.\n",rewards);
 return 0;
}
