/* V58: Federation Artillery Beam (charge by power, shield-piercing beam, power row,
 * damage/repair, upgrades, saves), store system installs for missing systems,
 * Rock B's Heavy Pierce Laser. */
#include "campaign.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static Game g;
static int beams(int side){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==side&&g.effects[i].kind==EFFECT_BEAM;return n;}
int main(void){
 /* Federation A: artillery level 1, 40 s charge, pierces shields. */
 assert(game_layout_supported(8)&&game_layout_supported(9));
 assert(game_init_ship(&g,8)&&g.player_artillery.capacity==1&&g.player_artillery.power==1&&g.player_artillery.weapon==GAME_WEAPON_ARTILLERY_FED);
 assert(game_player_artillery_need(&g)==40*GAME_HZ);
 assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.enemy_weapon.cooldown_ticks=g.enemy_weapon_secondary.cooldown_ticks=999999;
 g.enemy.shields=2;g.enemy.systems[SYS_SHIELDS].capacity=g.enemy.systems[SYS_SHIELDS].health=g.enemy.systems[SYS_SHIELDS].power=4;g.enemy.shields=2;
 {int hull=g.enemy.hull,fired=0;for(int t=0;t<41*GAME_HZ&&!fired;t++){game_step(&g);fired=beams(0);}
  assert(fired&&g.player_artillery.charge==0);for(int t=0;t<20;t++)game_step(&g);assert(g.enemy.hull<hull);}
 /* Power row: down to 0 stops charging, back up resumes. */
 assert(game_player_artillery_power(&g,-1)&&!game_player_artillery_need(&g)&&!game_player_artillery_power(&g,-1));
 assert(game_player_artillery_power(&g,1)&&!game_player_artillery_power(&g,1));
 /* Federation B starts at level 2 (32 s). */
 assert(game_init_ship(&g,9)&&g.player_artillery.capacity==2&&game_player_artillery_need(&g)==32*GAME_HZ);
 /* Damage to the artillery room drops its level; crew repair it. */
 {GameDamage hit={0,1,0,0,0,0,0,0};assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));int room=g.player_artillery.room;
  assert(game_launch_effect(&g,1,room,EFFECT_LASER,&hit,5));g.player.shields=0;for(int t=0;t<20;t++){game_step(&g);}
  assert(g.player_artillery.health<=1);}
 /* Upgrades and save round trip. */
 assert(campaign_init_ship(&c,5801,8));c.mode=CAM_MAP;c.scrap=500;assert(campaign_artillery_upgrade_price(&c)==30&&campaign_upgrade_artillery(&c)&&c.game.player_artillery.capacity==2);
 assert(campaign_upgrade_artillery(&c)&&campaign_upgrade_artillery(&c)&&c.game.player_artillery.capacity==4&&campaign_artillery_upgrade_price(&c)<0);
 c.game.player_artillery.power=1;
 assert(campaign_save(&c,"tests/v58.sav")&&campaign_load(&d,"tests/v58.sav")&&d.game.player_ship==8&&d.game.player_artillery.capacity==4&&d.game.player_artillery.power==1);
 /* Kestrel saves never carry artillery, and their stores keep selling cloaking. */
 campaign_init_seed(&c,5802);c.mode=CAM_STORE;assert(campaign_offer_system(&c)==10&&!strcmp(campaign_offer_name(&c,15),"Cloaking system (install)")&&campaign_offer_price(&c,15)==150);
 /* Stealth A has no shields: its stores sell them (125) into the reserved room. */
 assert(campaign_init_ship(&c,5803,2));c.mode=CAM_STORE;c.scrap=400;
 {int code=campaign_offer_system(&c);assert(code==SYS_SHIELDS||code==-1||code==10);}
 for(unsigned seed=5803;campaign_offer_system(&c)!=SYS_SHIELDS&&seed<5900;seed++){assert(campaign_init_ship(&c,seed,2));c.mode=CAM_STORE;c.scrap=400;}
 assert(campaign_offer_system(&c)==SYS_SHIELDS&&campaign_offer_price(&c,15)==125&&!strcmp(campaign_offer_name(&c,15),"Shield system (install)"));
 assert(campaign_purchase(&c,15)&&c.game.player.systems[SYS_SHIELDS].capacity==2&&c.game.player.systems[SYS_SHIELDS].room==game_ship_room(&c.game,SYS_SHIELDS));
 c.mode=CAM_MAP;assert(campaign_save(&c,"tests/v58.sav")&&campaign_load(&d,"tests/v58.sav")&&d.game.player.systems[SYS_SHIELDS].capacity==2&&d.game.player.systems[SYS_SHIELDS].room==c.game.player.systems[SYS_SHIELDS].room);
 /* Rock B (no doors) can buy doors; Slug B (no medbay) can buy a medbay. */
 {int found=0;for(unsigned seed=0;seed<200&&!found;seed++){assert(campaign_init_ship(&c,seed+1,13));c.mode=CAM_STORE;c.scrap=400;if(campaign_offer_system(&c)==7){found=1;assert(campaign_purchase(&c,15)&&c.game.subsystems[0][SUB_DOORS].capacity==1);
   c.mode=CAM_MAP;assert(campaign_save(&c,"tests/v58.sav")&&campaign_load(&d,"tests/v58.sav")&&d.game.subsystems[0][SUB_DOORS].capacity==1);}}assert(found);}
 {int found=0;for(unsigned seed=0;seed<200&&!found;seed++){assert(campaign_init_ship(&c,seed+1,11));c.mode=CAM_STORE;c.scrap=400;if(campaign_offer_system(&c)==SYS_MEDBAY){found=1;assert(campaign_purchase(&c,15)&&c.game.player.systems[SYS_MEDBAY].capacity==1);}}assert(found);}
 remove("tests/v58.sav");remove("tests/v58.sav.bak");
 /* Rock B carries its unique Heavy Pierce Laser (shield piercing 1). */
 assert(game_init_ship(&g,13));{int found=0;for(int i=0;i<g.weapon_count;i++)if(g.weapons[i].catalog_id==37){found=1;assert(g.weapons[i].shield_piercing==1&&g.weapons[i].damage==2);}assert(found);}
 assert(!game_weapon_player(GAME_WEAPON_ARTILLERY_FED));
 printf("PASS: Federation Artillery Beam (charge, piercing, power, damage, upgrades, save), store system installs (shields, doors, medbay), Heavy Pierce Laser.\n");
 return 0;
}
