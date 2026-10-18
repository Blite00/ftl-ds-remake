#include "campaign.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){Campaign c;campaign_init_legacy(&c);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_REBEL,1,0);
 int hull=c.game.enemy.hull,scrap=c.scrap,fuel=c.fuel;for(int i=0;i<c.game.enemy_crew_count;i++)c.game.enemy_crew[i].health=0;
 campaign_step(&c);assert(c.mode==CAM_EVENT&&c.game.outcome==GAME_VICTORY&&c.game.result_reason==RESULT_ENEMY_CREW&&c.game.enemy.hull==hull);
 assert(strstr(c.text,"intact")&&c.scrap>scrap&&c.fuel>fuel);scrap=c.scrap;fuel=c.fuel;for(int i=0;i<120;i++)campaign_step(&c);assert(c.scrap==scrap&&c.fuel==fuel);
 campaign_continue(&c);assert(c.mode==CAM_MAP);campaign_continue(&c);assert(c.scrap==scrap&&c.fuel==fuel);
 /* Automated ship is not defeated for having no crew. */
 game_begin_encounter(&c.game,ENEMY_ASSAULT,1,0);game_step(&c.game);assert(c.game.outcome==GAME_PLAYING&&c.game.result_reason==RESULT_NONE);
 game_begin_encounter(&c.game,ENEMY_REBEL,1,0);c.game.enemy.hull=0;game_step(&c.game);assert(c.game.outcome==GAME_VICTORY&&c.game.result_reason==RESULT_ENEMY_HULL);
 game_begin_encounter(&c.game,ENEMY_REBEL,1,0);for(int i=0;i<c.game.crew_count;i++)c.game.crew[i].health=0;game_step(&c.game);assert(c.game.outcome==GAME_DEFEAT&&c.game.result_reason==RESULT_PLAYER_CREW);
 puts("Crew outcomes: intact-ship victory, one-time salvage, automated exception and distinct hull/crew results passed");return 0;}
