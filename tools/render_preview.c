/* Exact game framebuffer previews, generated on host; not emulator screenshots. */
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "render.h"
#include "audio.h"
#include "setup.h"
#include "boarding.h"
#include "subsystems.h"
#include "species.h"
#include "drones.h"
#include "hangar.h"
#include "tutorial.h"
#include "profile.h"
#include "achievements.h"
#include "event_catalog.h"
static uint16_t top[256*192],bottom[256*192];
static void write_preview(const char*name){char path[160];snprintf(path,sizeof path,"build/%s-v71.ppm",name);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");for(int s=0;s<2;s++)for(int i=0;i<256*192;i++){unsigned v=(s?bottom:top)[i];unsigned char rgb[3]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31};fwrite(rgb,1,3,f);}assert(!fclose(f));}
static void save(Campaign*c,Ui*u,const char*name){render_campaign(c,u,top,bottom);write_preview(name);}
static void save_menu(MainMenu*m,const char*name){render_main_menu(m,top,bottom);write_preview(name);}
int main(void){CrewSetup setup;crew_setup_begin(&setup);render_crew_setup(&setup,top,bottom);write_preview("crew-setup");crew_setup_key(&setup,SETUP_ACCEPT);render_crew_setup(&setup,top,bottom);write_preview("crew-setup-keyboard");Campaign c;Ui u={.focus_room=16,.weapon=1};audio_init();MainMenu menu={0};main_menu_init(&menu,0);save_menu(&menu,"main-menu-new");main_menu_init(&menu,1);save_menu(&menu,"main-menu-continue");menu.selected=1;main_menu_key(&menu,2);save_menu(&menu,"main-menu-confirm");main_menu_key(&menu,3);menu.selected=3;main_menu_key(&menu,2);save_menu(&menu,"main-menu-controls");main_menu_init(&menu,1);menu.selected=4;save_menu(&menu,"main-menu-sound");main_menu_init(&menu,1);menu.selected=2;main_menu_key(&menu,2);save_menu(&menu,"main-menu-finale-original");main_menu_key(&menu,2);main_menu_open(&menu,1);save_menu(&menu,"main-menu-resume");campaign_init_seed(&c,42);save(&c,&u,"map");
 save(&c,&u,"map23");
 c.selected=10;save(&c,&u,"map-middle");c.selected=22;save(&c,&u,"map-exit");u.map_overview=1;save(&c,&u,"map-overview");c.current=10;c.selected=13;c.sector_jumps=12;u.map_overview=0;save(&c,&u,"map-pursuit-zoom");
 /* Low-hull UI fixtures; ordinary combat journeys are validated separately. */
 for(unsigned seed=1;seed<1024;seed++){campaign_init_seed(&c,seed);assert(campaign_imported_begin(&c,11));assert(campaign_choice(&c,0));if(c.imported_surrender_threshold>=0){c.game.enemy.hull=c.imported_surrender_threshold;campaign_step(&c);if(campaign_surrender_pending(&c))break;}}
 assert(campaign_surrender_pending(&c));u=(Ui){0};save(&c,&u,"surrender-offer");Campaign offer=c;ui_event_key(&c,&u,1);save(&c,&u,"surrender-refuse-selected");ui_event_key(&c,&u,2);save(&c,&u,"surrender-refused");c=offer;u=(Ui){0};ui_event_key(&c,&u,2);save(&c,&u,"surrender-accepted");
 for(unsigned seed=1;seed<1024;seed++){campaign_init_seed(&c,seed);assert(campaign_imported_begin(&c,11));assert(campaign_choice(&c,0));if(c.game.enemy_escape_total&&c.imported_surrender_threshold<0){c.game.enemy.hull=c.game.enemy_escape_threshold;campaign_step(&c);if(c.imported_battle==4)break;}}
 assert(c.imported_battle==4);u=(Ui){0};save(&c,&u,"rebel-escape-warning");assert(campaign_choice(&c,0));save(&c,&u,"rebel-escape-charging");


 campaign_init_seed(&c,42);assert(campaign_jump(&c,1));u=(Ui){0};ui_combat_key(&c,&u,UI_COMBAT_R);ui_combat_key(&c,&u,UI_COMBAT_RIGHT);save(&c,&u,"controller-crew");ui_combat_key(&c,&u,UI_COMBAT_START);save(&c,&u,"controller-actions");for(int i=0;i<7;i++)ui_combat_key(&c,&u,UI_COMBAT_DOWN);save(&c,&u,"controller-actions-page2");ui_combat_key(&c,&u,UI_COMBAT_A);save(&c,&u,"controller-doors");ui_combat_key(&c,&u,UI_COMBAT_START);ui_combat_key(&c,&u,UI_COMBAT_START);ui_combat_key(&c,&u,UI_COMBAT_Y);save(&c,&u,"controller-aim");campaign_init_seed(&c,42);u=(Ui){0};

 /* Reward menu fixtures exercise all overflow states without changing old scenes. */
 campaign_init_seed(&c,42);u=(Ui){0};assert(campaign_imported_begin(&c,9));save(&c,&u,"reward-weapon");
 for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=i;
 save(&c,&u,"reward-full-cargo");campaign_touch(&c,&u,80,99);ui_reward_key(&c,&u,2);save(&c,&u,"reward-weapon-replacement");
 ui_reward_key(&c,&u,3);ui_reward_key(&c,&u,3);save(&c,&u,"reward-decline");
 campaign_init_seed(&c,42);u=(Ui){0};c.mode=CAM_EVENT;c.event_resolved=1;assert(rewards_offer_crew(&c.reward,&c.game,SPEC_ZOLTAN));save(&c,&u,"reward-recruit");
 for(int i=c.game.crew_count;i<GAME_MAX_CREW;i++)game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);
 c.game.crew_count=GAME_MAX_CREW;save(&c,&u,"reward-full-crew");campaign_touch(&c,&u,80,149);ui_reward_key(&c,&u,2);save(&c,&u,"reward-crew-replacement");
 campaign_init_seed(&c,42);u=(Ui){.focus_room=16,.weapon=1};

 campaign_init_seed(&c,2828);assert(campaign_jump(&c,1));u=(Ui){0};save(&c,&u,"retreat-combat");ui_retreat_open(&c,&u);save(&c,&u,"retreat-charge");
 c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;save(&c,&u,"retreat-ready");c.sector_jumps=30;save(&c,&u,"retreat-captured");c.sector_jumps=2;
 c.game.crew_count=GAME_MAX_CREW;for(int i=0;i<GAME_MAX_CREW;i++){if(i>=3)game_init_crew(&c.game,0,i,c.game.player.rooms[0].id);c.game.crew[i].ship=1;c.game.crew[i].room=c.game.crew[i].destination=c.game.enemy.rooms[0].id;}
 u.retreat_confirm=1;save(&c,&u,"retreat-eight-away");campaign_init_seed(&c,42);u=(Ui){.weapon=1};

 c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL_FAT,2,0);c.game.paused=1;c.game.subsystems[0][SUB_SENSORS].health=c.game.subsystems[0][SUB_SENSORS].capacity=3;u=(Ui){.weapon=1};save(&c,&u,"rebel-rigger");u.aiming=1;save(&c,&u,"rebel-rigger-aim");u.aiming=0;assert(boarding_install(&c.game,0));u.boarding=1;u.board_side=1;u.board_room=c.game.enemy_pilot_room;save(&c,&u,"rebel-rigger-board");u=(Ui){.weapon=1};

 for(int root=6;root<8;root++)for(int intact=0;intact<=(root==7);intact++){
  char name[80];campaign_init_seed(&c,42);c.sector=1;u=(Ui){0};assert(campaign_imported_begin(&c,root));
  snprintf(name,sizeof name,"station-%d-introduction",root);save(&c,&u,name);
  assert(campaign_choice(&c,0));snprintf(name,sizeof name,"station-%d-engage",root);save(&c,&u,name);
  assert(campaign_choice(&c,0));assert(c.mode==CAM_COMBAT);
  c.game.outcome=GAME_VICTORY;c.game.result_reason=intact?RESULT_ENEMY_CREW:RESULT_ENEMY_HULL;
  if(!intact)c.game.enemy.hull=0;
  for(int tick=0;tick<240&&c.mode==CAM_COMBAT;tick++)campaign_step(&c);
  assert(c.mode==CAM_EVENT);snprintf(name,sizeof name,"station-%d-%s-outcome",root,intact?"crew":"hull");save(&c,&u,name);
  assert(campaign_choice(&c,0));snprintf(name,sizeof name,"station-%d-%s-reward",root,intact?"crew":"hull");save(&c,&u,name);
 }

 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;assert(game_begin_encounter(&c.game,ENEMY_SCOUT,1,0));u=(Ui){.aiming=1,.weapon=0};ui_select_weapon(&c.game,&u,0);game_set_weapon_target(&c.game,0,c.game.enemy.systems[SYS_WEAPONS].room);c.game.target_room=c.game.enemy.systems[SYS_WEAPONS].room;
 save(&c,&u,"target-hidden");c.game.subsystems[0][SUB_SENSORS].health=c.game.subsystems[0][SUB_SENSORS].capacity=3;c.game.enemy.systems[SYS_WEAPONS].capacity=3;c.game.enemy.systems[SYS_WEAPONS].health=1;save(&c,&u,"target-damaged");c.game.enemy.systems[SYS_WEAPONS].health=0;save(&c,&u,"target-disabled");
 c.game.effects[0]=(GameEffect){.active=1,.side=0,.room=c.game.weapons[0].aim_room,.kind=EFFECT_MISSILE,.age=12,.duration=12,.result=EFFECT_HIT};save(&c,&u,"target-impact-hit");c.game.effects[0].result=EFFECT_MISS;save(&c,&u,"target-impact-miss");c.game.effects[0].active=0;
 campaign_init_seed(&c,42);u=(Ui){0};assert(campaign_imported_begin(&c,8));save(&c,&u,"escape-introduction");
 assert(campaign_choice(&c,0));assert(c.imported_battle==4);save(&c,&u,"escape-warning");
 assert(campaign_choice(&c,0));c.game.paused=1;save(&c,&u,"escape-charging");
 c.game.enemy.systems[SYS_ENGINES].power=0;save(&c,&u,"escape-stopped");
 c.game.enemy.systems[SYS_ENGINES].power=1;c.game.paused=0;c.game.enemy_escape_ticks=1;campaign_step(&c);assert(c.game.outcome==GAME_ESCAPE);save(&c,&u,"escape-gotaway");
 {Campaign report=c;c.mode=CAM_COMBAT;u.aiming=1;save(&c,&u,"escape-departed");c.game.outcome=GAME_PLAYING;u.aiming=0;u.boarding=1;u.board_side=1;save(&c,&u,"escape-departed-board");assert(game_equip(&c.game,0,6));assert(game_set_bomb_target(&c.game,0,1,c.game.player.rooms[0].id));u=(Ui){.weapon=0,.aiming=1};save(&c,&u,"escape-home-bomb");c=report;u=(Ui){0};}
 campaign_continue(&c);save(&c,&u,"escape-fleet-map");
 int long_event=0,four_event=0,blue_event=0;
 for(int root=0;root<campaign_imported_count();root++){
  campaign_init_seed(&c,42);c.fuel=0;u=(Ui){0};assert(campaign_imported_begin(&c,root));
  if(!long_event&&ui_event_text_pages(&c)>1){save(&c,&u,"original-event-page1");ui_event_text_key(&c,&u,1);save(&c,&u,"original-event-page2");long_event=1;u.event_text_page=0;}
  if(!strcmp(campaign_imported_id(root),"REPAIR_STATION")){c.scrap=50;c.game.player.hull=10;save(&c,&u,"original-event-repair");Campaign repair=c;ui_event_key(&c,&u,2);u.feedback_ticks=0;save(&c,&u,"original-event-repair-outcome");c=repair;}
  if(!strcmp(campaign_imported_id(root),"FUEL_FOR_DRONE")){c.fuel=10;save(&c,&u,"original-event-trade-range");}
  if(!four_event&&campaign_choice_count(&c)==4){u.event_selected=3;save(&c,&u,"original-event-four");four_event=1;}
  for(int i=0;i<campaign_choice_count(&c);i++)if(!blue_event&&campaign_choice_blue(&c,i)){
   for(int crew=0;crew<c.game.crew_count;crew++)c.game.crew[crew].species=SPEC_ENGI;
   u.event_selected=i==0?1:0;save(&c,&u,"original-event-blue");
   for(int crew=0;crew<c.game.crew_count;crew++)c.game.crew[crew].species=SPEC_HUMAN;
   u.event_selected=i;save(&c,&u,"original-event-disabled");blue_event=1;
  }
 }
 campaign_init_seed(&c,42);c.fuel=0;u=(Ui){0};assert(campaign_imported_begin(&c,0));c.text[0]=0;
 for(int line=0;line<23;line++){char part[64];snprintf(part,sizeof part,"Synthetic test line %02d is readable.\n",line+1);strcat(c.text,part);}
 save(&c,&u,"event-synthetic-page1");ui_event_text_key(&c,&u,1);save(&c,&u,"event-synthetic-page2");ui_event_text_key(&c,&u,1);save(&c,&u,"event-synthetic-page3");
 campaign_init_seed(&c,42);u=(Ui){0};
 c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;c.current=c.node_count-1;snprintf(c.text,sizeof c.text,"Choose your next sector. Both routes lead onward; their encounter pools differ. The fleet follows whichever route you take.");save(&c,&u,"sector-routes");u.event_selected=1;save(&c,&u,"sector-routes-second");c.game.crew[0].ship=1;ui_event_key(&c,&u,2);save(&c,&u,"sector-routes-blocked");c.game.crew[0].ship=0;u.feedback_ticks=0;
 ui_event_key(&c,&u,2);u.feedback_ticks=0;save(&c,&u,"sector-identity");
 campaign_init_seed(&c,42);u=(Ui){0};
 c.selected=6;while(c.sector_jumps<40&&!campaign_node_captured(&c,6,1))c.sector_jumps++;save(&c,&u,"fleet-threat");
 c.sector_jumps++;c.node_kind[6]=NODE_STORE;save(&c,&u,"fleet-captured-store");
 campaign_init_legacy_seed(&c,42);c.node_count=12;c.sector_count=6;c.current=0;c.selected=5;
 for(int i=0;i<CAM_NODE_COUNT;i++){c.links[i]=0;c.visited[i]=i==0;c.node_kind[i]=NODE_FIGHT;c.node_y[i]=(i&1)?67:123;}
 c.node_kind[0]=NODE_START;c.node_kind[5]=NODE_STORE;c.node_kind[11]=NODE_EXIT;c.node_y[0]=c.node_y[11]=94;
 c.links[0]=(1<<1)|(1<<2);c.links[1]|=1;c.links[2]|=1;
 for(int i=1;i<9;i++){c.links[i]|=1<<(i+2);c.links[i+2]|=1<<i;}
 c.links[9]|=1<<11;c.links[10]|=1<<11;c.links[11]=(1<<9)|(1<<10);
 snprintf(c.text,sizeof c.text,"Migrated legacy run: 12 beacons and six sectors preserved.");save(&c,&u,"fleet-legacy12");
 campaign_init_seed(&c,42);
 c.scrap=120;u.page=1;save(&c,&u,"crew");u.page=2;save(&c,&u,"upgrades");u.page=3;c.cargo[0]=3;c.cargo[1]=2;save(&c,&u,"cargo");u.page=4;save(&c,&u,"help");u.page=0;
 c.mode=CAM_STORE;c.current=5;save(&c,&u,"store");u.shop_page=1;save(&c,&u,"store2");u.shop_page=2;save(&c,&u,"store-ion");save(&c,&u,"store-beam");save(&c,&u,"store-bomb");u.shop_page=0;
 c.mode=CAM_EVENT;c.event_id=EVENT_DISTRESS;c.event_resolved=0;snprintf(c.text,sizeof c.text,"A civilian transport needs fuel. Deliver 2 fuel for 24 scrap, salvage its abandoned cargo, or escort it through a hostile patrol.");save(&c,&u,"event");
 c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_SCOUT,0,0);c.game.paused=1;save(&c,&u,"combat");u.aiming=1;save(&c,&u,"aim");
 game_equip(&c.game,2,2);game_equip(&c.game,3,1);game_begin_encounter(&c.game,ENEMY_FLAGSHIP,5,2);c.game.paused=1;save(&c,&u,"boss-aim");u.aiming=0;save(&c,&u,"boss");
 game_begin_encounter(&c.game,ENEMY_ASSAULT,2,0);c.game.paused=1;u.aiming=1;save(&c,&u,"assault-aim");u.aiming=0;c.game.enemy.shields=0;c.game.enemy.shield_ticks=35;c.game.player.rooms[10].fires=1;ui_feedback(&u,"Weapon needs power.",1);save(&c,&u,"assault-feedback");u.feedback_ticks=0;
 game_begin_encounter(&c.game,ENEMY_REBEL,2,0);c.game.paused=1;u.aiming=1;save(&c,&u,"rebel-aim");u.aiming=0;u.crew_details=1;save(&c,&u,"crew-details");u.renaming=1;snprintf(u.rename_text,sizeof u.rename_text,"JACOB");save(&c,&u,"crew-rename");u.renaming=0;u.crew_details=0;
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.scrap=200;assert(campaign_install_teleporter(&c));game_adjust_power(&c.game,SYS_ENGINES,-1);assert(boarding_power(&c.game,0,1));
 c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL_BOARDER,2,0);c.game.paused=1;u=(Ui){.weapon=1,.boarding=1,.board_party=3,.board_room=c.game.teleporter[0].room};save(&c,&u,"boarding-home");
 for(int i=0;i<2;i++){c.game.crew[i].room=c.game.crew[i].destination=c.game.teleporter[0].room;}
 u.board_side=1;u.board_room=c.game.enemy.systems[SYS_WEAPONS].room;assert(boarding_deploy(&c.game,3,u.board_room));save(&c,&u,"boarding-enemy");
 c.mode=CAM_EVENT;c.event_resolved=1;c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_CREW;u.page=1;u.abandon_confirm=1;save(&c,&u,"boarding-abandon");u=(Ui){.weapon=1};
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);u=(Ui){0};c.game.enemy.systems[SYS_WEAPONS].capacity=c.game.enemy.systems[SYS_WEAPONS].health=c.game.enemy.systems[SYS_WEAPONS].power=3;{int pair[]={2,GAME_ENEMY_ARTEMIS};assert(game_enemy_set_loadout(&c.game,pair,2,10));}save(&c,&u,"combat-clear");ui_deselect_crew(&u);save(&c,&u,"crew-deselected");u=(Ui){0};c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;save(&c,&u,"combat-clear-sensors");c.game.enemy.hull=0;c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_HULL;c.result_ticks=22;save(&c,&u,"explosion-enemy");u.aiming=1;u.drone_panel=1;u.actions=1;u.feedback_ticks=30;save(&c,&u,"explosion-enemy-modal");
 c.game.enemy.hull=c.game.enemy.max_hull;c.game.outcome=GAME_DEFEAT;c.game.result_reason=RESULT_PLAYER_HULL;c.game.player.hull=0;c.result_ticks=38;save(&c,&u,"explosion-player");u=(Ui){0};
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);u=(Ui){.weapon=1};
 c.game.player.rooms[0].breaches=1;c.game.player.rooms[0].breach_repair_ticks=75;c.game.player.rooms[0].oxygen=550;c.game.player.rooms[10].fires=1;c.game.player.rooms[10].breaches=2;save(&c,&u,"breach-home");
 u.crew_details=1;save(&c,&u,"breach-sealing");u.crew_details=0;u.aiming=1;c.game.enemy.rooms[1].breaches=2;c.game.enemy.rooms[1].fires=1;c.game.target_room=c.game.enemy.rooms[1].id;c.game.weapons[1].aim_room=c.game.target_room;save(&c,&u,"breach-target");
 u.aiming=0;u.boarding=1;u.board_side=1;u.board_room=c.game.target_room;save(&c,&u,"breach-boarding");u=(Ui){.weapon=1};
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;
 c.game.crew_count=6;for(int i=3;i<6;i++)c.game.crew[i]=c.game.crew[0];u.crew_page=1;save(&c,&u,"crew6");u.doors=1;save(&c,&u,"doors");
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL_BOARDER,2,0);u=(Ui){.weapon=1,.aiming=1};
 c.game.enemy.rooms[0].fires=1;c.game.enemy.rooms[0].breaches=1;save(&c,&u,"sensors-level1");
 c.game.subsystems[0][SUB_SENSORS].health=c.game.subsystems[0][SUB_SENSORS].capacity=2;save(&c,&u,"sensors-level2");
 c.game.subsystems[0][SUB_SENSORS].health=c.game.subsystems[0][SUB_SENSORS].capacity=3;c.game.enemy_fire_ticks=c.game.enemy_weapon.cooldown_ticks/2;save(&c,&u,"sensors-level3");
 c.game.subsystems[0][SUB_SENSORS].health=0;u.aiming=0;c.game.player.rooms[10].fires=1;c.game.player.rooms[4].breaches=1;save(&c,&u,"sensors-broken");
 c.game.crew[0].ship=1;c.game.crew[0].room=c.game.crew[0].destination=c.game.enemy.rooms[0].id;u.aiming=1;save(&c,&u,"sensors-away-sight");
 campaign_init_seed(&c,42);u=(Ui){.page=5};c.scrap=120;save(&c,&u,"subsystem-upgrades");
 c.mode=CAM_COMBAT;u=(Ui){.doors=1,.focus_room=16};c.game.subsystems[0][SUB_DOORS].health=c.game.subsystems[0][SUB_DOORS].capacity=3;c.game.doors[0].force_ticks=60;save(&c,&u,"doors-reinforced");c.game.subsystems[0][SUB_DOORS].health=0;ui_feedback(&u,"Door control damaged. Repair it first.",1);save(&c,&u,"doors-broken");u=(Ui){0};
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;
 for(int i=3;i<8;i++){game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);}c.game.crew_count=8;
 for(int i=0;i<8;i++)game_set_crew_species(&c.game,0,i,i%SPEC_COUNT);
 u=(Ui){.crew_page=2,.selected_crew=7};save(&c,&u,"crew-eight");u.group_mode=1;u.crew_group=3;u.crew_page=0;save(&c,&u,"crew-group");
 u=(Ui){.boarding=1,.board_page=1,.board_party=192};save(&c,&u,"boarding-eight");
 u=(Ui){.crew_details=1,.selected_crew=3};save(&c,&u,"rock-details");u.selected_crew=6;save(&c,&u,"crystal-details");c.mode=CAM_MAP;u.dismiss_confirm=1;save(&c,&u,"crew-dismiss");u=(Ui){0};
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,4));u=(Ui){.weapon=2,.aiming=1};c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
 c.game.enemy.systems[SYS_WEAPONS].ion_bars=1;c.game.enemy.systems[SYS_WEAPONS].ion_ticks=4*GAME_HZ;c.game.target_room=c.game.weapons[2].aim_room=c.game.enemy.systems[SYS_WEAPONS].room;save(&c,&u,"ion-enemy");
 u.aiming=0;c.game.player.systems[SYS_SHIELDS].ion_bars=1;c.game.player.systems[SYS_SHIELDS].ion_ticks=5*GAME_HZ;c.game.player.shields=0;c.game.player.systems[SYS_WEAPONS].ion_bars=1;c.game.player.systems[SYS_WEAPONS].ion_ticks=8*GAME_HZ;save(&c,&u,"ion-player");
 assert(game_launch_ion(&c.game,0,c.game.target_room,1));c.game.effects[0].age=6;save(&c,&u,"ion-bolt");
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,5));u=(Ui){.weapon=2,.aiming=1};c.game.weapons[0].enabled=c.game.weapons[1].enabled=0;
 const GameRoom*br=&c.game.enemy.rooms[0];int bx=(br->x*2+br->w)*GAME_BEAM_UNITS/2,by=(br->y*2+br->h)*GAME_BEAM_UNITS/2;
 assert(game_set_beam_target(&c.game,2,bx,by,bx,by+2*GAME_BEAM_UNITS));save(&c,&u,"beam-target");
 c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;assert(game_fire(&c.game,2,br->id));game_step(&c.game);for(int tick=0;tick<8;tick++)game_step(&c.game);u.aiming=0;save(&c,&u,"beam-sweep");
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,2,0);assert(game_equip(&c.game,2,6));u=(Ui){.weapon=2,.aiming=1};c.game.weapons[0].enabled=c.game.weapons[1].enabled=0;
 c.game.target_room=c.game.weapons[2].aim_room=c.game.enemy.systems[SYS_WEAPONS].room;save(&c,&u,"bomb-target");ui_bomb_toggle_side(&c.game,&u);u.feedback_ticks=0;save(&c,&u,"bomb-home");ui_bomb_toggle_side(&c.game,&u);u.feedback_ticks=0;c.game.weapons[2].charge_ticks=c.game.weapons[2].cooldown_ticks;assert(game_fire(&c.game,2,c.game.target_room));game_step(&c.game);
 c.game.effects[0].age=c.game.effects[0].duration;c.game.effects[0].result=EFFECT_HIT;u.aiming=0;save(&c,&u,"bomb-impact");
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.cargo[0]=6;c.cargo[2]=5;u=(Ui){.page=3,.equipment_installed=1};save(&c,&u,"equipment-installed");u.equipment_installed=0;u.cargo_selected=2;save(&c,&u,"equipment-cargo");u.sale_confirm=1;save(&c,&u,"equipment-sale");u.sale_confirm=0;
 assert(campaign_sell_weapon(&c,1,1));assert(campaign_sell_weapon(&c,1,0));c.mode=CAM_COMBAT;u=(Ui){.aiming=1};save(&c,&u,"equipment-zero");
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.scrap=300;assert(campaign_purchase(&c,11));c.drone_cargo[2]=0;u=(Ui){.drone_panel=1,.drone_cargo=1,.drone_selected=2};save(&c,&u,"drone-management");u.drone_sale=1;save(&c,&u,"drone-sale");u.drone_sale=0;u.drone_cargo=0;u.drone_selected=0;c.mode=CAM_COMBAT;game_adjust_power(&c.game,SYS_ENGINES,-1);game_adjust_power(&c.game,SYS_OXYGEN,-1);assert(drones_adjust_power(&c.game,1));assert(drones_adjust_power(&c.game,1));assert(drones_toggle(&c.game,0));c.game.drones.slots[0].charge_ticks=170;save(&c,&u,"drone-active");drones_reset(&c.game);c.game.drone_parts=0;ui_drone_key(&c,&u,6);u.drone_focus=7;ui_drone_key(&c,&u,2);save(&c,&u,"drone-no-parts");u=(Ui){0};
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.current=5;c.scrap=300;u=(Ui){.shop_page=3,.shop_selected=2};save(&c,&u,"defense-store");assert(campaign_purchase(&c,11));assert(campaign_purchase(&c,14));assert(campaign_drone_swap(&c,0,0));u=(Ui){.drone_panel=1};c.mode=CAM_COMBAT;game_adjust_power(&c.game,SYS_ENGINES,-1);game_adjust_power(&c.game,SYS_OXYGEN,-1);assert(drones_adjust_power(&c.game,1));assert(drones_adjust_power(&c.game,1));assert(drones_toggle(&c.game,0));c.game.drones.slots[0].charge_ticks=drones_cooldown(1);save(&c,&u,"defense-ready");c.game.drones.slots[0].charge_ticks=0;c.game.effects[0]=(GameEffect){.active=1,.side=1,.kind=EFFECT_MISSILE,.result=EFFECT_INTERCEPT,.duration=18,.age=18,.room=0};save(&c,&u,"defense-intercept");u=(Ui){0};
 Campaign preview_before=c;Ui preview_ui=u;
 u=(Ui){0};
 /* Sector fixture using the real guaranteed-fight campaign selector. */
 for(unsigned seed=1;seed<=128;seed++){campaign_init_seed(&c,seed);c.sector=1;if(campaign_jump(&c,1)&&c.game.enemy_weapon_count==2&&c.game.enemy_weapon_secondary.missiles)break;}
 assert(c.game.enemy_weapon_count==2&&c.game.enemy_weapon_secondary.missiles);save(&c,&u,"enemy-missile-contact");
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;c.game.enemy_weapon.charge_ticks=120;c.game.enemy_weapon_secondary.charge_ticks=240;save(&c,&u,"enemy-dual-charge");c.game.enemy_missiles=0;save(&c,&u,"enemy-missiles-empty");c.game.enemy_missiles=10;c.game.enemy.systems[SYS_WEAPONS].power=1;save(&c,&u,"enemy-secondary-off");
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_REBEL,0,0));c.mode=CAM_COMBAT;u=(Ui){0};
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
 c.game.enemy_crew[0].health=20;c.game.enemy.systems[SYS_SHIELDS].health=0;c.game.enemy.systems[SYS_SHIELDS].power=0;c.game.enemy.shields=0;
 for(int step=0;step<2*GAME_HZ;step++)game_step(&c.game);
 save(&c,&u,"enemy-ai-triage");
 /* V38: original target selection; a Burst II volley in flight to distinct rooms. */
 campaign_init_seed(&c,7);assert(game_begin_encounter(&c.game,ENEMY_ARMORED,0,0));c.mode=CAM_COMBAT;u=(Ui){0};
 {int ids[]={1,2};assert(game_enemy_set_loadout(&c.game,ids,2,10));}
 c.game.player.systems[SYS_ENGINES].power=0;c.game.enemy_weapon.charge_ticks=c.game.enemy_weapon.cooldown_ticks-1;
 for(int step=0;step<14;step++)game_step(&c.game);
 assert(c.game.enemy_weapon.queued_target_count==3);save(&c,&u,"enemy-volley");
 /* V38: enemy beam aimed from one chosen room toward a second, full length. */
 campaign_init_seed(&c,9);assert(game_begin_encounter(&c.game,ENEMY_SCOUT,0,0));c.mode=CAM_COMBAT;u=(Ui){0};
 c.game.enemy_weapon.beam_length=(80*GAME_BEAM_UNITS+17)/35;c.game.enemy_weapon.shots=1;c.game.enemy_weapon.damage=2;c.game.enemy_weapon.ion_damage=0;c.game.enemy_weapon.bomb=0;c.game.enemy_weapon.missiles=0;
 c.game.enemy_fire_ticks=c.game.enemy_weapon.cooldown_ticks-1;
 for(int step=0;step<14;step++)game_step(&c.game);
 assert(c.game.enemy_weapon.queued_beam_length);save(&c,&u,"enemy-beam");
 /* V39: unscripted Rebel boarder encounter; original party size and a random landing room. */
 campaign_init_seed(&c,11);assert(game_begin_encounter(&c.game,ENEMY_REBEL_BOARDER,0,0));c.mode=CAM_COMBAT;u=(Ui){0};
 {int landed=0;for(int step=0;step<120*GAME_HZ&&!landed;step++){game_step(&c.game);for(int i=0;i<c.game.enemy_crew_count;i++)landed|=c.game.enemy_crew[i].health>0&&c.game.enemy_crew[i].ship==0;}
  assert(landed);for(int step=0;step<GAME_HZ;step++)game_step(&c.game);}
 save(&c,&u,"enemy-boarders");
 /* V40: calm ship; a 90% crewmember leaves its console for the medbay (bCalmShip, 99% threshold). */
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_REBEL,0,0));c.mode=CAM_COMBAT;u=(Ui){0};
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;c.game.enemy_crew[1].health=90;
 for(int step=0;step<GAME_HZ+2;step++)game_step(&c.game);
 assert(c.game.enemy_crew[1].destination==c.game.enemy.systems[SYS_MEDBAY].room);save(&c,&u,"enemy-calm-heal");
 c=preview_before;u=preview_ui;
 /* V41: original final sector; flagship route, rebel-held beacons, base and countdown. */
 campaign_init_seed(&c,42);for(int i=1;i<c.sector_count-1;i++)c.sector_types[i]=SECTOR_REBEL;c.sector=c.sector_count-2;c.next_sector[0]=SECTOR_FINAL;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;
 assert(campaign_choice(&c,0)&&campaign_finale_active(&c));u=(Ui){0};c.fuel=20;for(int i=0;i<3;i++)assert(campaign_wait(&c));c.selected=campaign_boss_node(&c);
 save(&c,&u,"final-sector-map");u.map_overview=1;save(&c,&u,"final-sector-overview");u.map_overview=0;
 while(campaign_base_turns_left(&c)<0&&c.mode==CAM_MAP){campaign_wait(&c);}
 assert(c.mode==CAM_MAP);c.selected=c.node_count-1;save(&c,&u,"final-sector-base-attack");
 /* V42: authentic flagship stages (imported BOSS_N_NORMAL) with artillery and cloak. */
 for(int stage=0;stage<3;stage++){campaign_init_seed(&c,42);c.game.interim_flagship=0;assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,stage));c.mode=CAM_COMBAT;u=(Ui){0};
  c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;c.game.enemy_cloak.capacity=0;
  for(int step=0;step<8*GAME_HZ;step++)game_step(&c.game);
  char name[32];snprintf(name,sizeof name,"flagship-stage%d",stage+1);save(&c,&u,name);}
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,0));c.mode=CAM_COMBAT;u=(Ui){0};game_step(&c.game);assert(game_enemy_cloaked(&c.game));save(&c,&u,"flagship-cloaked");
 /* V43: stage 2 drones deployed and a power-surge warning. */
 /* V44: stage 3 with its super shield and the surge warning before a laser barrage. */
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,2));c.mode=CAM_COMBAT;u=(Ui){0};c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
 c.game.surge_ticks=5*GAME_HZ;for(int step=0;step<2*GAME_HZ;step++){game_step(&c.game);}
 save(&c,&u,"flagship-super-shield");
 for(int step=0;step<3*GAME_HZ+2;step++){game_step(&c.game);}
 save(&c,&u,"flagship-laser-surge");
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,1));c.mode=CAM_COMBAT;u=(Ui){0};c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
 c.game.surge_ticks=8*GAME_HZ;for(int step=0;step<4*GAME_HZ;step++)game_step(&c.game);assert(game_surge_warning(&c.game));save(&c,&u,"flagship-drones-surge");
 /* V46: room system icons, door-following crew, projectile markers, landing results, fires and UNMANNED. */
 {GameDamage laser={1,1,15,0,0,0,0,0},missile={2,2,30,0,0,0,0,0};
  campaign_init_seed(&c,42);c.mode=CAM_COMBAT;assert(game_begin_encounter(&c.game,ENEMY_REBEL,2,0));u=(Ui){0};Game*g=&c.game;
  g->subsystems[0][SUB_SENSORS].capacity=g->subsystems[0][SUB_SENSORS].health=3;
  assert(game_order_crew(g,1,g->player.systems[SYS_SHIELDS].room));for(int i=0;i<12;i++)game_step(g);
  memset(g->effects,0,sizeof g->effects);
  assert(game_launch_effect(g,1,g->player.systems[SYS_SHIELDS].room,EFFECT_LASER,&laser,0));g->effects[0].age=7;
  assert(game_launch_effect(g,1,g->player.systems[SYS_WEAPONS].room,EFFECT_MISSILE,&missile,5));g->effects[1].age=4;
  assert(game_launch_effect(g,0,g->enemy.systems[SYS_WEAPONS].room,EFFECT_LASER,&laser,0));g->effects[2].age=5;
  g->player.rooms[13].fires=2;g->player.rooms[13].oxygen=620;g->player.rooms[4].breaches=1;g->player.rooms[4].oxygen=180;
  g->player.systems[SYS_ENGINES].health=1;g->paused=1;save(&c,&u,"v46-incoming");
  u.aiming=1;memset(g->effects,0,sizeof g->effects);
  assert(game_launch_effect(g,0,g->enemy.systems[SYS_SHIELDS].room,EFFECT_LASER,&laser,0));g->effects[0].age=g->effects[0].duration+1;g->effects[0].result=EFFECT_HIT;
  assert(game_launch_effect(g,0,g->enemy.systems[SYS_ENGINES].room,EFFECT_LASER,&laser,0));g->effects[1].age=g->effects[1].duration+2;g->effects[1].result=EFFECT_MISS;
  assert(game_launch_effect(g,0,g->enemy.systems[SYS_WEAPONS].room,EFFECT_MISSILE,&missile,5));g->effects[2].age=6;
  g->enemy.systems[SYS_SHIELDS].health=1;g->enemy.systems[SYS_WEAPONS].health=0;g->target_room=g->enemy.systems[SYS_WEAPONS].room;save(&c,&u,"v46-aim-results");
  g->effects[0].result=EFFECT_SHIELD;g->effects[1].active=g->effects[2].active=0;save(&c,&u,"v46-aim-shield");
  u.aiming=0;memset(g->effects,0,sizeof g->effects);
  for(int i=0;i<g->crew_count;i++)if(g->crew[i].room==g->player.systems[SYS_WEAPONS].room)g->crew[i].room=g->crew[i].destination=g->pilot_room==0?1:0;
  g->weapons[0].charge_ticks=g->weapons[0].cooldown_ticks;g->ticks=10;save(&c,&u,"v46-unmanned");
  campaign_init_seed(&c,42);c.mode=CAM_COMBAT;assert(game_begin_encounter(&c.game,ENEMY_SCOUT,0,0));u=(Ui){0};g=&c.game;
  g->player.systems[SYS_SHIELDS].health=1;g->player.systems[SYS_ENGINES].health=0;g->player.systems[SYS_OXYGEN].ion_ticks=90;g->player.systems[SYS_OXYGEN].ion_bars=1;g->paused=1;save(&c,&u,"v46-icon-states");
  campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,0));c.mode=CAM_COMBAT;u=(Ui){.aiming=1};g=&c.game;g->enemy_cloak.capacity=0;
  g->subsystems[0][SUB_SENSORS].capacity=g->subsystems[0][SUB_SENSORS].health=3;g->artillery[1].health=1;g->paused=1;save(&c,&u,"v46-flagship-icons");}
 /* V47: enemy damage colours without sensors. */
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;assert(game_begin_encounter(&c.game,ENEMY_REBEL,2,0));u=(Ui){.aiming=1};
 c.game.enemy.systems[SYS_SHIELDS].health=1;c.game.enemy.systems[SYS_WEAPONS].health=0;c.game.paused=1;save(&c,&u,"v47-enemy-damage-no-sensors");
 /* V49: a sealed artillery room whose gunner died stays silent (UNMAN). */
 campaign_init_seed(&c,42);assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,0));c.mode=CAM_COMBAT;u=(Ui){0};c.game.enemy_cloak.capacity=0;
 c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;
 for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(c.game.enemy_crew[i].room==c.game.artillery[0].room)c.game.enemy_crew[i].health=0;
 for(int i=0;i<3*GAME_HZ;i++){game_step(&c.game);}assert(!game_artillery_manned(&c.game,0));c.game.paused=1;save(&c,&u,"v49-artillery-silent");
 /* V51: base-game weapons in a store and in combat on both sides. */
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.current=3;c.scrap=300;u=(Ui){.shop_page=2};save(&c,&u,"v51-store-weapons");
 campaign_init_seed(&c,42);c.mode=CAM_COMBAT;assert(game_begin_encounter(&c.game,ENEMY_REBEL,4,0));u=(Ui){0};
 {Game*g=&c.game;g->weapon_count=0;assert(game_equip(g,0,23)&&game_equip(g,1,18)&&game_equip(g,2,12));g->player.systems[SYS_WEAPONS].capacity=g->player.systems[SYS_WEAPONS].health=8;g->player.reactor=16;
  for(int i=0;i<3;i++){game_adjust_power(g,SYS_WEAPONS,1);}g->subsystems[0][SUB_SENSORS].capacity=g->subsystems[0][SUB_SENSORS].health=3;
  g->enemy.systems[SYS_WEAPONS].capacity=g->enemy.systems[SYS_WEAPONS].health=g->enemy.systems[SYS_WEAPONS].power=4;{int pair[2]={22,29};assert(game_enemy_set_loadout(g,pair,2,10));}
  for(int i=0;i<12*GAME_HZ;i++){game_step(g);}g->paused=1;save(&c,&u,"v51-combat-weapons");}
 /* V52: original enemy ships of every people, with sensors to show crews. */
 {static const int ships[]={7,10,12,17,19,21,30,15};static const char*const names[]={"slug","rock","mantis","engi","zoltan","crystal","pirate","fed"};
  for(int k=0;k<8;k++){campaign_init_seed(&c,42);c.mode=CAM_COMBAT;c.game.enemy_ship=ships[k];assert(game_begin_encounter(&c.game,ENEMY_SHIP,4,0));
   c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=3;for(int t=0;t<3*GAME_HZ;t++){game_step(&c.game);}c.game.paused=1;
   char name[32];u=(Ui){.aiming=1};snprintf(name,sizeof name,"v52-ship-%s",names[k]);save(&c,&u,name);}}
 /* V53: original sector-list events (many choices, a blue option). */
 {int many=0,blue=0;for(int root=12;root<campaign_imported_count()&&!(many&&blue);root++){campaign_init_seed(&c,4242u+(unsigned)root);c.sector=2;c.sector_types[2]=SECTOR_CIVILIAN;c.fuel=20;c.scrap=300;c.mode=CAM_MAP;u=(Ui){0};
   if(!campaign_imported_begin(&c,root)){continue;}int n=campaign_choice_count(&c),b=0;for(int i=0;i<n;i++)b|=campaign_choice_blue(&c,i);
   if(!many&&n>=5){many=1;save(&c,&u,"v53-event-many");}else if(!blue&&b){blue=1;save(&c,&u,"v53-event-blue");}}}
 c=preview_before;u=preview_ui;
 /* V55: cloaking and augments in the store, ship management and combat. */
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.scrap=600;u=(Ui){.shop_page=3};save(&c,&u,"v55-store-cloak");
 u.shop_page=4;save(&c,&u,"v55-store-augment");
 assert(campaign_purchase(&c,15)&&campaign_purchase(&c,16));game_add_augment(&c.game,AUG_SCRAP_COLLECTOR);c.mode=CAM_MAP;u=(Ui){.page=5};save(&c,&u,"v55-manage-cloak-augments");
 {Game*g=&c.game;assert(game_begin_encounter(g,ENEMY_REBEL,2,0));c.mode=CAM_COMBAT;g->player.reactor=20;game_cloak_power(g,1);assert(game_cloak_activate(g));
  for(int i=0;i<2*GAME_HZ;i++){game_step(g);}g->paused=1;u=(Ui){.actions=1,.action_selected=13};save(&c,&u,"v55-cloaked-combat");}
 /* V56: interior and boarding drones in combat, the drone panel and the store's drone offer. */
 campaign_init_seed(&c,42);c.mode=CAM_STORE;c.scrap=600;u=(Ui){.shop_page=3};save(&c,&u,"v56-store-drone");
 {Game*g=&c.game;campaign_init_seed(&c,42);assert(game_begin_encounter(g,ENEMY_REBEL,2,0));c.mode=CAM_COMBAT;g->player.reactor=30;assert(drones_install(g));g->drones.capacity=g->drones.health=8;g->drones.slot_count=3;g->drone_parts=6;
  assert(drones_equip(g,0,8)&&drones_equip(g,1,7)&&drones_equip(g,2,6));while(g->drones.power<6)assert(drones_adjust_power(g,1));for(int i=0;i<3;i++)assert(drones_toggle(g,i));
  g->enemy_weapon.cooldown_ticks=g->enemy_weapon_secondary.cooldown_ticks=999999;g->player.systems[SYS_ENGINES].health=0;
  {int slot=g->enemy_crew_count;game_init_crew(g,1,slot,g->player.systems[SYS_OXYGEN].room);g->enemy_crew[slot].ship=0;g->enemy_crew[slot].station=g->enemy_crew[slot].room;g->enemy_crew_count=slot+1;}
  for(int t=0;t<5*GAME_HZ;t++){drones_step(g);}g->paused=1;u=(Ui){0};save(&c,&u,"v56-interior-drones");
  u=(Ui){.aiming=1};save(&c,&u,"v56-boarding-drone");
  u=(Ui){0};u.drone_panel=1;u.drone_selected=0;save(&c,&u,"v56-drone-panel");}
 /* V57: hangar (default, locked silhouette, unlocked Engi, unsupported Federation), 4-crew setup, other ships in combat. */
 {Hangar h;Profile p;profile_defaults(&p);hangar_open(&h,&p);render_hangar(&h,top,bottom);write_preview("v57-hangar-kestrel");
  hangar_key(&h,HANGAR_RIGHT);render_hangar(&h,top,bottom);write_preview("v57-hangar-locked");
  profile_unlock_layout(&p,6);profile_unlock_layout(&p,8);hangar_open(&h,&p);h.cls=3;render_hangar(&h,top,bottom);write_preview("v57-hangar-engi");
  h.cls=4;render_hangar(&h,top,bottom);write_preview("v57-hangar-federation");
  CrewSetup cs;crew_setup_begin_ship(&cs,16,4);render_crew_setup(&cs,top,bottom);write_preview("v57-crew-setup-crystal");}
 {static const int layouts[]={2,4,6,10,12,14,16,3,5,7,11,13,15,17};static const char*const names[]={"stealth","mantis","engi","slug","rock","zoltan","crystal","stealth-b","mantis-b","engi-b","slug-b","rock-b","zoltan-b","crystal-b"};
  for(int k=0;k<14;k++){char name[40];assert(campaign_init_ship(&c,42,layouts[k]));assert(game_begin_encounter(&c.game,ENEMY_REBEL,1,0));c.mode=CAM_COMBAT;
   for(int t=0;t<3*GAME_HZ;t++){game_step(&c.game);}c.game.paused=1;u=(Ui){.focus_room=16,.weapon=1};snprintf(name,sizeof name,"v57-ship-%s",names[k]);save(&c,&u,name);}
  assert(campaign_init_ship(&c,42,6));u=(Ui){0};save(&c,&u,"v57-map-engi");}
 /* V58: Federation artillery in combat and on the Upgrade page; Stealth store selling shields. */
 {assert(campaign_init_ship(&c,42,8));Game*g=&c.game;assert(game_begin_encounter(g,ENEMY_REBEL,1,0));c.mode=CAM_COMBAT;g->enemy_weapon.cooldown_ticks=g->enemy_weapon_secondary.cooldown_ticks=999999;
  for(int t=0;t<40*GAME_HZ+4;t++){game_step(g);}g->paused=1;u=(Ui){.focus_room=16,.weapon=1};save(&c,&u,"v58-federation-artillery");
  u=(Ui){.actions=1,.action_selected=12};save(&c,&u,"v58-artillery-power-row");
  assert(campaign_init_ship(&c,42,8));c.mode=CAM_MAP;c.scrap=200;u=(Ui){.page=5};save(&c,&u,"v58-upgrade-artillery");
  for(unsigned seed=1;seed<300;seed++){assert(campaign_init_ship(&c,seed,2));c.mode=CAM_STORE;c.scrap=300;if(campaign_offer_system(&c)==SYS_SHIELDS)break;}
  u=(Ui){.shop_page=3};save(&c,&u,"v58-store-shields");}
 /* V59: a locked Type B with one of its three achievements earned, and an unlocked one. */
 {Hangar h;Profile p;profile_defaults(&p);profile_add_achievements(&p,1u<<ACH_KESTREL_TOUGH);hangar_open(&h,&p);h.variant=1;render_hangar(&h,top,bottom);write_preview("v59-hangar-kestrel-b-locked");
  profile_add_achievements(&p,1u<<ACH_KESTREL_FEDERATION);hangar_open(&h,&p);h.variant=1;render_hangar(&h,top,bottom);write_preview("v59-hangar-kestrel-b-unlocked");}
 /* V60: fights in each beacon hazard. */
 {static const char*const names[]={"","asteroid","sun","nebula","storm"};
  for(int env=1;env<=4;env++){char name[40];campaign_init_seed(&c,42);Game*g=&c.game;assert(game_begin_encounter(g,ENEMY_REBEL,1,0));c.mode=CAM_COMBAT;
   g->enemy_weapon.cooldown_ticks=g->enemy_weapon_secondary.cooldown_ticks=999999;game_set_environment(g,env);
   for(int t=0;t<(env==2?21:6)*GAME_HZ;t++){game_step(g);}g->paused=1;u=(Ui){.focus_room=16,.weapon=1};snprintf(name,sizeof name,"v60-%s",names[env]);save(&c,&u,name);}}
 /* V61: a quest beacon marked on the map. */
 {int target=-1;for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT&&target<0;i++)if(imported_event_effects[i].kind==IE_QUEST)target=imported_event_effects[i].min;
  campaign_init_seed(&c,42);c.mode=CAM_MAP;assert(target>=0&&campaign_quest_add(&c,target)==1);snprintf(c.text,sizeof c.text,"A quest beacon is marked on the map.");u=(Ui){0};save(&c,&u,"v61-quest-map");}
 /* V62: an event with long choices (taller boxes) and intruders aboard after a boarder event. */
 {int best=-1;for(int i=0;i<IMPORTED_EVENT_NODE_COUNT;i++){const ImportedEventNode*n=&imported_event_nodes[i];if(n->choice_count>=2&&n->choice_count<=3&&n->ship<0&&strlen(imported_event_choices[n->first_choice].text)>80){best=i;break;}}
  if(best>=0){campaign_init_seed(&c,42);c.mode=CAM_EVENT;c.event_id=EVENT_IMPORTED;c.event_resolved=0;c.imported_node=best;snprintf(c.text,sizeof c.text,"%s",imported_event_nodes[best].text);u=(Ui){0};save(&c,&u,"v62-long-choices");}
  campaign_init_seed(&c,42);c.mode=CAM_MAP;c.game.enemy.hull=0;assert(game_spawn_intruders(&c.game,3,SPEC_MANTIS)==3);u=(Ui){.page=1};save(&c,&u,"v62-intruders");}
 /* V64: tutorial steps (intro, power, crew, fire, map, combat). */
 {static Tutorial t;tutorial_begin(&t);render_tutorial(&t,top,bottom);write_preview("v64-tutorial-intro");
  for(int i=0;i<5;i++){tutorial_key(&t,TUT_A);}render_tutorial(&t,top,bottom);write_preview("v64-tutorial-power");}
 /* V65: the Crystal secret sector on the map. */
 {campaign_init_seed(&c,42);c.sector=3;c.sector_types[3]=SECTOR_SECRET;c.mode=CAM_MAP;u=(Ui){0};save(&c,&u,"v65-crystal-sector");}
 /* V68: fire (pulsing room + icon), O2 LOW!, HULL CRITICAL, breach and intruder warnings; then a fresh hull hit. */
 {campaign_init_seed(&c,42);Game*g=&c.game;assert(game_begin_encounter(g,ENEMY_REBEL,1,0));c.mode=CAM_COMBAT;
  g->enemy_weapon.cooldown_ticks=g->enemy_weapon_secondary.cooldown_ticks=999999;
  for(int i=0;i<g->player.room_count;i++){GameRoom*r=&g->player.rooms[i];if(r->id==g->player.systems[SYS_WEAPONS].room)r->fires=2;if(r->id==g->player.systems[SYS_ENGINES].room)r->breaches=1;}
  g->player.oxygen=180;for(int i=0;i<g->player.room_count;i++)g->player.rooms[i].oxygen=i%2?120:260;g->player.hull=g->player.max_hull/4;
  assert(game_spawn_intruders(g,2,SPEC_MANTIS)==2);g->ticks=8;g->paused=1;
  assert(audio_alarm_mask(g)==((1u<<ALARM_FIRE)|(1u<<ALARM_BREACH)|(1u<<ALARM_OXYGEN)|(1u<<ALARM_HULL)|(1u<<ALARM_INTRUDER)));
  u=(Ui){.focus_room=16,.weapon=1};save(&c,&u,"v68-alarms");
  g->effects[0]=(GameEffect){.active=1,.side=1,.kind=EFFECT_LASER,.room=g->player.systems[SYS_SHIELDS].room,.duration=12,.age=13,.result=EFFECT_HIT,.damage=1};
  save(&c,&u,"v68-hull-hit");}
 /* V70: impact flash and damage number on the hit rooms of your ship (both screens). */
 {campaign_init_seed(&c,42);assert(campaign_jump(&c,1));Game*g=&c.game;c.mode=CAM_COMBAT;g->paused=1;u=(Ui){0};
  for(int i=0;i<g->player.room_count;i++){GameRoom*r=&g->player.rooms[i];if(r->id==g->player.systems[SYS_SHIELDS].room){r->hit_ticks=20;r->hit_damage=2;}if(r->id==g->player.systems[SYS_WEAPONS].room){r->hit_ticks=12;r->hit_damage=1;}}
  save(&c,&u,"v70-hit-flash");}
 /* V69: each weapon's own numbered target on the aim screen; weapon 3 holds fire (cleared with B). */
 {campaign_init_seed(&c,42);assert(campaign_jump(&c,1));Game*g=&c.game;c.mode=CAM_COMBAT;g->paused=1;g->subsystems[0][SUB_SENSORS].health=g->subsystems[0][SUB_SENSORS].capacity=3;
  u=(Ui){.aiming=1,.weapon=1};for(int i=0;i<g->weapon_count&&i<2;i++){assert(game_set_weapon_target(g,i,g->enemy.rooms[i?g->enemy.room_count-1:0].id));}
  g->target_room=g->enemy.rooms[g->enemy.room_count-1].id;save(&c,&u,"v69-weapon-targets");u.aiming=0;save(&c,&u,"v69-weapon-targets-fight");}
 /* V68 shot paths: volleys leave from each weapon's hull mount (slot i uses mount i) toward the target;
  * hits land in the targeted room, shield blocks stop on the bubble, misses fly past. */
 {static const int layouts[3]={0,0,1},enemies[3]={12,12,4};static const char*const names[3]={"v68-shots-kestrel","v68-shots-enemy","v68-shots-typeb"};
  for(int k=0;k<3;k++){campaign_init_seed(&c,42);Game*g=&c.game;assert(game_init_ship(g,layouts[k]));g->enemy_ship=enemies[k];assert(game_begin_encounter(g,ENEMY_SHIP,1,0));c.mode=CAM_COMBAT;
   g->subsystems[0][SUB_SENSORS].health=g->subsystems[0][SUB_SENSORS].capacity=3;g->paused=1;g->ticks=8;g->enemy.shields=2;
   int er=g->enemy.systems[SYS_WEAPONS].room,es=g->enemy.systems[SYS_SHIELDS].room,pr=g->player.systems[SYS_SHIELDS].room,po=g->player.systems[SYS_OXYGEN].room;
   memset(g->effects,0,sizeof g->effects);
   g->effects[0]=(GameEffect){.active=1,.side=0,.kind=EFFECT_LASER,.room=er,.duration=12,.age=3,.mount=1};
   g->effects[1]=(GameEffect){.active=1,.side=0,.kind=EFFECT_MISSILE,.room=es,.duration=12,.age=7,.mount=2};
   g->effects[2]=(GameEffect){.active=1,.side=0,.kind=EFFECT_LASER,.room=es,.duration=12,.age=12,.result=EFFECT_HIT,.mount=1};
   g->effects[3]=(GameEffect){.active=1,.side=1,.kind=EFFECT_LASER,.room=pr,.duration=12,.age=6,.mount=1};
   g->effects[4]=(GameEffect){.active=1,.side=1,.kind=EFFECT_LASER,.room=po,.duration=12,.age=12,.result=EFFECT_SHIELD,.mount=2};
   g->effects[5]=(GameEffect){.active=1,.side=1,.kind=EFFECT_ION,.room=po,.duration=12,.age=15,.result=EFFECT_MISS,.mount=2};
   u=(Ui){.focus_room=16,.weapon=0,.aiming=k==1};save(&c,&u,names[k]);}}
 c.mode=CAM_COMPLETE;snprintf(c.text,sizeof c.text,"Victory! The flagship is defeated. Completed %d sectors in 64 jumps with 16 hull. Seed 42.",c.sector_count);save(&c,&u,"victory");return 0;}

