/* Focused state fixtures exercise campaign integration; not a complete-run claim. */
#include "campaign.h"
#include "boarding.h"
#include "outcome.h"
#include <assert.h>
#include <stdio.h>
static Campaign c;
static void init(void){campaign_init_legacy_seed(&c,919);c.scrap=300;}
static void board_fixture(void){
 init();c.mode=CAM_STORE;assert(campaign_install_teleporter(&c));
 game_begin_encounter(&c.game,ENEMY_REBEL,0,0);c.mode=CAM_COMBAT;c.game.player.reactor=20;
 assert(boarding_power(&c.game,0,1));
 for(int i=0;i<2;i++)c.game.crew[i].room=c.game.crew[i].destination=c.game.teleporter[0].room;
 /* One last defender, injured but alive, is a controlled melee fixture. */
 for(int i=0;i<c.game.enemy_crew_count;i++)c.game.enemy_crew[i].health=i?0:6;
 c.game.enemy_crew[0].room=c.game.enemy_crew[0].destination=c.game.enemy.systems[SYS_WEAPONS].room;
 c.game.enemy.systems[SYS_WEAPONS].power=0;
 assert(boarding_deploy(&c.game,3u,c.game.enemy.systems[SYS_WEAPONS].room));
}
int main(void){int scrap,age,fuel,missiles;
 init();assert(campaign_teleporter_price(&c)==120);assert(!campaign_install_teleporter(&c)&&c.scrap==300);
 c.mode=CAM_STORE;c.scrap=119;assert(!campaign_install_teleporter(&c)&&c.scrap==119);
 c.scrap=300;assert(campaign_install_teleporter(&c)&&c.scrap==180);assert(!campaign_install_teleporter(&c)&&c.scrap==180);
 assert(campaign_teleporter_price(&c)==40);c.mode=CAM_COMBAT;assert(!campaign_upgrade_teleporter(&c)&&c.scrap==180);
 c.mode=CAM_EVENT;c.event_resolved=0;assert(!campaign_upgrade_teleporter(&c));c.event_resolved=1;
 assert(campaign_upgrade_teleporter(&c)&&c.scrap==140);c.mode=CAM_MAP;assert(campaign_upgrade_teleporter(&c)&&c.scrap==100);
 assert(c.game.teleporter[0].capacity==3&&campaign_teleporter_price(&c)==-1);assert(!campaign_upgrade_teleporter(&c)&&c.scrap==100);
 /* Terminal hull fixture: no salvage until the complete presentation interval. */
 init();c.mode=CAM_COMBAT;c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_HULL;
 scrap=c.scrap;fuel=c.fuel;missiles=c.game.missiles;c.game.paused=1;
 for(age=0;age<100;age++)campaign_step(&c);
 assert(c.result_ticks==0&&c.scrap==scrap&&c.mode==CAM_COMBAT);c.game.paused=0;
 for(age=0;age<outcome_duration(&c.game);age++){campaign_step(&c);assert(c.mode==CAM_COMBAT&&c.scrap==scrap&&c.fuel==fuel&&c.game.missiles==missiles);}
 campaign_step(&c);assert(c.mode==CAM_EVENT&&c.scrap==scrap+18&&c.fuel==fuel+2&&c.game.missiles==missiles+2);
 for(age=0;age<120;age++){campaign_step(&c);}assert(c.scrap==scrap+18&&c.fuel==fuel+2&&c.game.missiles==missiles+2);
 /* Real melee resolves the controlled last-defender encounter. */
 board_fixture();scrap=c.scrap;
 for(age=0;age<5*GAME_HZ&&c.mode==CAM_COMBAT;age++)campaign_step(&c);
 assert(c.mode==CAM_EVENT&&c.game.result_reason==RESULT_ENEMY_CREW&&c.result_ticks==0&&c.scrap==scrap+18);
 assert(game_away_crew(&c.game)==2);campaign_continue(&c);assert(c.mode==CAM_EVENT);
 for(age=0;age<BOARDING_COOLDOWN;age++)campaign_step(&c);
 assert(boarding_recall(&c.game,3u)&&!game_away_crew(&c.game));campaign_continue(&c);assert(c.mode==CAM_MAP);
 /* Abandon is explicit and cannot happen during an unresolved battle. */
 board_fixture();assert(!campaign_abandon_boarders(&c)&&game_away_crew(&c.game)==2);
 c.game.crew[2].health=0;c.game.outcome=GAME_VICTORY;c.game.result_reason=RESULT_ENEMY_CREW;c.mode=CAM_EVENT;c.event_resolved=1;
 assert(campaign_abandon_boarders(&c)==2&&c.mode==CAM_DEFEAT&&c.game.result_reason==RESULT_PLAYER_CREW);
 assert(!campaign_abandon_boarders(&c));
 puts("Boarding campaign: install/upgrade economy, presentation pause/one-time salvage, melee victory, away guard, recall and explicit last-crew abandonment passed");return 0;
}

