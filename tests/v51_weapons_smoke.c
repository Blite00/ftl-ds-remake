/* V51: the base-game weapon list and its new mechanics: hull-bust, lockdown,
 * healing and ion bombs, zero-damage beams blocked by shields, enemy beams and
 * bombs, store stock and enemy loadout picks. */
#include "campaign.h"
#include "species.h"
#include "weapon_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static int find(const char*id){for(int i=0;i<GAME_CATALOG_COUNT;i++)if(!strcmp(imported_weapon_catalog[i].id,id))return i;return -1;}
static void settle(int ticks){for(int t=0;t<ticks;t++)game_step(&g);}
static int empty_room(const Ship*s,int want_system){for(int i=0;i<s->room_count;i++){int sys=s->rooms[i].system>=0;if(sys==want_system&&s->rooms[i].id!=g.enemy_pilot_room&&s->rooms[i].id!=g.enemy_drone.room)return s->rooms[i].id;}return -1;}
int main(void){
 /* Catalog: 37 entries, stable historical ids, enemy-only Artemis at 7, AE weapons absent. */
 assert(GAME_CATALOG_COUNT==39&&find("MISSILES_2_PLAYER")==0&&find("BOMB_1")==6&&find("MISSILES_2")==GAME_ENEMY_ARTEMIS);
 assert(!game_weapon_player(GAME_ENEMY_ARTEMIS)&&game_weapon_player(find("BEAM_3"))&&find("SHOTGUN")<0&&find("LASER_CHAINGUN")<0);
 int players=0;for(int i=0;i<GAME_CATALOG_COUNT;i++){if(!game_weapon_player(i))continue;players++;game_init(&g);assert(game_equip(&g,0,i)&&g.weapons[0].cooldown_ticks>0&&*game_weapon_short(i)&&strlen(game_weapon_short(i))<=8&&game_weapon_price(i)>0);}
 assert(players==37); /* V58: + Heavy Pierce Laser (Rock B); the Artillery Beam is system-mounted. */
 /* Hull-bust: double hull damage in a room without a system, normal on a system room. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy.shields=0;g.enemy.systems[SYS_SHIELDS].power=0;g.enemy.systems[SYS_ENGINES].power=0;g.enemy_pilot_health=0;
 {int plain=empty_room(&g.enemy,0),sys=g.enemy.systems[SYS_WEAPONS].room,hull=g.enemy.hull;GameDamage d={1,1,0,0,0,1,0,0};
  assert(plain>=0&&game_launch_effect(&g,0,plain,EFFECT_LASER,&d,0));settle(14);assert(g.enemy.hull==hull-2);
  hull=g.enemy.hull;assert(game_launch_effect(&g,0,sys,EFFECT_LASER,&d,0));settle(14);assert(g.enemy.hull==hull-1);}
 /* Lockdown bomb seals the room; healing burst heals crew up to their maximum. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));
 {int room=g.crew[0].room;GameDamage lock={0,0,0,0,0,0,1,0},heal={0,0,-150,0,0,0,0,0};g.crew[0].health=20;
  assert(game_launch_bomb(&g,1,room,&heal));settle(14);assert(g.crew[0].health==species_max_health(g.crew[0].species));
  assert(game_launch_bomb(&g,1,room,&lock));settle(14);int r=-1;for(int i=0;i<g.player.room_count;i++)if(g.player.rooms[i].id==room)r=i;assert(r>=0&&g.player.rooms[r].lockdown_ticks>0);}
 /* Ion bomb bypasses shields and ionizes the target room's system. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy.shields=2;g.enemy.systems[SYS_ENGINES].power=0;g.enemy_pilot_health=0;
 {GameDamage ion={0,0,0,0,0,0,0,4};assert(game_launch_bomb(&g,0,g.enemy.systems[SYS_WEAPONS].room,&ion));settle(14);assert(g.enemy.systems[SYS_WEAPONS].ion_bars>0&&!g.enemy.systems[SYS_SHIELDS].ion_bars);}
 /* Zero-damage beams (fire, anti-bio) are stopped by any shield layer, land without shields. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));assert(game_equip(&g,0,find("BEAM_BIO")));
 {GameDamage bio={0,0,60,0,0,0,0,0};const GameRoom*r=&g.enemy.rooms[0];int x=(2*r->x+r->w)*GAME_BEAM_UNITS/2,y=(2*r->y+r->h)*GAME_BEAM_UNITS/2;
  g.enemy.shields=1;g.enemy.systems[SYS_ENGINES].power=0;g.enemy_pilot_health=0;assert(game_launch_beam(&g,0,x,y,x+1,y,&bio));settle(14);
  int found=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)found|=g.effects[i].kind==EFFECT_BEAM&&g.effects[i].result==EFFECT_SHIELD;assert(found);}
 /* Enemies may carry beams and bombs: both fire at the player. */
 {int pairs[2][2]={{find("BEAM_1"),2},{find("BOMB_FIRE"),2}};
  for(int k=0;k<2;k++){game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy.systems[SYS_WEAPONS].capacity=g.enemy.systems[SYS_WEAPONS].health=g.enemy.systems[SYS_WEAPONS].power=4;
   for(int i=0;i<g.weapon_count;i++){g.weapons[i].enabled=0;}g.player.hull=g.player.max_hull=1000;
   assert(game_enemy_set_loadout(&g,pairs[k],2,10));int seen=0;for(int t=0;t<40*GAME_HZ;t++){game_step(&g);for(int i=0;i<GAME_MAX_EFFECTS;i++)seen|=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].kind==(k?EFFECT_BOMB:EFFECT_BEAM);}assert(seen);}}
 /* Stores: offers 8-10 sell distinct base-game weapons at their original cost; the purchase lands in cargo. */
 {Campaign c;campaign_init_seed(&c,42);c.mode=CAM_STORE;c.current=3;c.scrap=500;int a=campaign_offer_weapon(&c,8),b=campaign_offer_weapon(&c,9),d=campaign_offer_weapon(&c,10);
  assert(game_weapon_player(a)&&game_weapon_player(b)&&game_weapon_player(d)&&a!=b&&b!=d&&a!=d);assert(campaign_offer_weapon(&c,4)==2&&campaign_offer_weapon(&c,7)<0);
  assert(campaign_offer_price(&c,9)==game_weapon_price(b)&&strstr(campaign_offer_name(&c,9),game_weapon_name(b)));
  for(int i=0;i<CAM_CARGO_COUNT;i++){c.cargo[i]=-1;}assert(campaign_purchase(&c,9)&&c.cargo[0]==b);
  int seen[GAME_CATALOG_COUNT]={0},kinds=0;for(int node=0;node<c.node_count;node++)for(int sector=0;sector<8;sector++){c.current=node;c.sector=sector;for(int n=8;n<=10;n++)seen[campaign_offer_weapon(&c,n)]=1;}
  for(int i=0;i<GAME_CATALOG_COUNT;i++){kinds+=seen[i];}assert(kinds>=20);}
 puts("PASS: base-game weapon catalog, hull-bust, lockdown/heal/ion bombs, shielded zero-damage beams, enemy beams/bombs and store stock.");
 return 0;
}
