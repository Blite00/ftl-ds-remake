/* V55: player cloaking (store install, power, activation, enemy charge pause,
 * lockout, firing cost, upgrades) and augments (slots, stacking, store offer,
 * effects, event hooks) with save29 persistence. */
#include "campaign.h"
#include "save.h"
#include "event_catalog.h"
#include "subsystems.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
int main(void){
 /* Store install and save29 round trip of cloak + augments. */
 campaign_init_seed(&c,5501);c.mode=CAM_STORE;c.scrap=600;
 assert(!strcmp(campaign_offer_name(&c,15),"Cloaking system (install)")&&campaign_offer_price(&c,15)==150);
 assert(campaign_purchase(&c,15)&&c.game.player_cloak.capacity==1&&!campaign_purchase(&c,15));
 int aug=campaign_offer_augment(&c);assert(aug>=0&&game_augment_rarity(aug)>=1&&campaign_offer_price(&c,16)==game_augment_cost(aug));
 assert(campaign_purchase(&c,16)&&game_augment_count(&c.game,aug)==1);
 c.mode=CAM_MAP;assert(campaign_upgrade_cloak(&c)&&c.game.player_cloak.capacity==2&&campaign_upgrade_cloak(&c)&&campaign_cloak_upgrade_price(&c)<0);
 assert(campaign_save(&c,"tests/v55.sav")&&campaign_load(&d,"tests/v55.sav"));
 assert(d.game.player_cloak.capacity==3&&d.game.player_cloak.health==3&&d.game.augments[0]==aug&&d.game.augments[1]==-1);
 remove("tests/v55.sav");remove("tests/v55.sav.bak");
 /* Cloak in combat: power, activation, enemy weapons stop charging, ion lockout. */
 Game*g=&c.game;assert(game_begin_encounter(g,ENEMY_REBEL,2,0));g->player.reactor=20;
 assert(!game_cloak_activate(g));assert(game_cloak_power(g,1)&&game_cloak_power(g,1));
 assert(game_cloak_activate(g)&&game_player_cloaked(g)&&g->player_cloak.active_ticks==10*GAME_HZ);
 {int before=g->enemy_fire_ticks+g->enemy_weapon.charge_ticks+g->enemy_weapon_secondary.charge_ticks;for(int t=0;t<GAME_HZ;t++)game_step(g);
  assert(g->enemy_fire_ticks+g->enemy_weapon.charge_ticks+g->enemy_weapon_secondary.charge_ticks==before);}
 /* Firing while cloaked shortens the cloak by 2 s unless Stealth Weapons is fitted. */
 {int left=g->player_cloak.active_ticks;g->weapons[0].charge_ticks=g->weapons[0].cooldown_ticks;assert(game_fire(g,0,g->enemy.rooms[0].id));assert(g->player_cloak.active_ticks==left-2*GAME_HZ);}
 for(int t=0;t<12*GAME_HZ&&game_player_cloaked(g);t++){game_step(g);}
 assert(!game_player_cloaked(g)&&g->player_cloak.ion_bars==4&&!game_cloak_activate(g));
 /* Augments: three slots, non-stackable duplicates refused, stackable ones allowed. */
 game_init(g);assert(game_add_augment(g,AUG_WEAPON_PREIGNITE)==0&&game_add_augment(g,AUG_WEAPON_PREIGNITE)<0);
 assert(game_add_augment(g,AUG_SCRAP_COLLECTOR)==1&&game_add_augment(g,AUG_SCRAP_COLLECTOR)==2&&game_add_augment(g,AUG_FTL_BOOSTER)<0);
 assert(game_remove_augment(g,2)&&game_augment_count(g,AUG_SCRAP_COLLECTOR)==1);
 /* Weapon Pre-igniter: weapons start charged. */
 assert(game_begin_encounter(g,ENEMY_REBEL,1,0));for(int i=0;i<g->weapon_count;i++)assert(g->weapons[i].charge_ticks==g->weapons[i].cooldown_ticks);
 /* Scrap Recovery Arm +10%, Repair Arm repairs 1 hull for 15% of the gain. */
 campaign_init_seed(&c,5502);c.scrap=0;game_add_augment(&c.game,AUG_SCRAP_COLLECTOR);campaign_gain_scrap(&c,100);assert(c.scrap==110);
 game_add_augment(&c.game,AUG_REPAIR_ARM);c.game.player.hull=20;c.scrap=0;campaign_gain_scrap(&c,100);assert(c.scrap==94&&c.game.player.hull==21);
 /* Zoltan Shield: 5 points absorb the first shots of each encounter. */
 game_init(g);game_add_augment(g,AUG_ENERGY_SHIELD);assert(game_begin_encounter(g,ENEMY_REBEL,1,0)&&g->player_super_shield==5);
 {GameDamage hit={2,1,10,0,0,0,0,0};int hull=g->player.hull;g->enemy.systems[SYS_WEAPONS].power=0;game_launch_effect(g,1,g->player.rooms[0].id,EFFECT_LASER,&hit,0);for(int t=0;t<14;t++){game_step(g);}
  assert(g->player.hull==hull&&(g->player_super_shield==3||g->player_super_shield==5));}
 /* Advanced Scanners: enemy rooms visible with sensors destroyed. */
 game_init(g);assert(game_begin_encounter(g,ENEMY_REBEL,1,0));g->subsystems[0][SUB_SENSORS].health=0;
 {int room=g->enemy.rooms[g->enemy.room_count-1].id;int seen=game_room_visible(g,1,room);game_add_augment(g,AUG_ADV_SCANNERS);assert(game_room_visible(g,1,room)>=seen&&game_room_visible(g,1,room));}
 /* Adv. FTL Navigation: explored beacons become reachable. */
 campaign_init_seed(&c,5503);c.mode=CAM_MAP;int far=-1;for(int n=1;n<c.node_count;n++)if(!campaign_linked(&c,c.current,n)){far=n;break;}
 assert(far>0);c.visited[far]=1;assert(!campaign_can_jump(&c,far));game_add_augment(&c.game,AUG_FTL_JUMPER);assert(campaign_can_jump(&c,far));
 /* Events now use augment requirements and rewards. */
 int req=0,reward=0;for(int i=0;i<IMPORTED_EVENT_CHOICE_COUNT;i++)req+=imported_event_choices[i].req_kind==IE_REQ_AUGMENT;
 for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT;i++){reward+=imported_event_effects[i].kind==IE_AUGMENT;}assert(req+reward>0);
 printf("PASS: cloaking install/power/activation/pause/lockout/fire cost/upgrades, augments (slots, stacking, preignite, scrap, repair arm, zoltan shield, navigation), events (%d reqs, %d rewards), save29.\n",req,reward);
 return 0;
}
