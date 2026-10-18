/* Full ordinary-economy campaign with a real boarding deployment and recall. */
#define main original_campaign_main
#include "campaign_v10_smoke.c"
#undef main
#include "boarding.h"
static int deployments,recalls,pursuit_ticks;
static int distance(const Game*g,int from,int to){int n=0,next;while(from!=to&&n<GAME_MAX_ROOMS){next=game_next_room(g,1,from,to);if(next<0||next==from)return 99;from=next;n++;}return n;}
static void boarding_fight(Campaign*c){Game*g=&c->game;int i,t;
 if(!deployments&&g->teleporter[0].installed&&!g->enemy.automated){
  unsigned mask=6u;unsigned ids[2]={g->crew[1].id,g->crew[2].id};
  while(g->teleporter[0].power<1){if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_ENGINES,-1);if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_WEAPONS,-1);assert(boarding_power(g,0,1));}
  assert(game_order_crew(g,1,g->teleporter[0].room));assert(game_order_crew(g,2,g->teleporter[0].room));
  for(t=0;t<30*GAME_HZ&&!(g->crew[1].room==g->teleporter[0].room&&g->crew[2].room==g->teleporter[0].room);t++)campaign_step(c);
  assert(c->mode==CAM_COMBAT);assert(boarding_deploy(g,mask,1));deployments++;
  /* Keep the away team moving as enemy defenders pursue until recall recharges. */
  for(t=0;t<BOARDING_COOLDOWN+5*GAME_HZ&&game_away_crew(g);t++){
   if(!g->teleporter[0].cooldown_ticks&&g->crew[1].room==g->crew[2].room){
    game_order_crew(g,1,g->crew[1].room);game_order_crew(g,2,g->crew[2].room);
    if(boarding_recall(g,mask)){recalls++;break;}
   }
   if(g->crew[1].room==g->crew[1].destination&&g->crew[2].room==g->crew[2].destination){int target=-1,best=-999;
    for(i=0;i<g->enemy.room_count;i++){int j,score=99;int r=g->enemy.rooms[i].id;
     if(r==g->crew[1].room)continue;
     for(j=0;j<g->enemy_crew_count;j++)if(g->enemy_crew[j].health>0&&g->enemy_crew[j].ship==1){int d=distance(g,r,g->enemy_crew[j].room);if(d<score)score=d;}
     score=score*10-distance(g,g->crew[1].room,r);if(score>best){best=score;target=r;}
    }
    assert(target>=0);game_order_crew(g,1,target);game_order_crew(g,2,target);
   }
   campaign_step(c);
   for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==1&&g->enemy_crew[i].destination==g->crew[1].room)pursuit_ticks++;
  }
  if(!recalls)fprintf(stderr,"Boarding run recall failed hp%d/%d room%d/%d dest%d/%d cooldown%d mode%d\n",g->crew[1].health,g->crew[2].health,g->crew[1].room,g->crew[2].room,g->crew[1].destination,g->crew[2].destination,g->teleporter[0].cooldown_ticks,c->mode);
  assert(recalls==1&&!game_away_crew(g));assert(g->crew[1].health>0&&g->crew[2].health>0&&g->crew[1].id==ids[0]&&g->crew[2].id==ids[1]);
  assert(boarding_power(g,0,-1));
 }
 fight(c);
}
static void boarding_settle(Campaign *c) {
    while(c->mode!=CAM_MAP&&c->mode!=CAM_COMPLETE) {
        if(c->mode==CAM_COMBAT)boarding_fight(c);
        else if(c->mode==CAM_STORE){
            while(c->game.player.hull<=25&&campaign_purchase(c,2)){}
            if(c->game.weapon_count==2&&campaign_upgrade_price(c,SYS_WEAPONS)>=0&&c->scrap>=25+campaign_upgrade_price(c,SYS_WEAPONS)&&campaign_purchase(c,4)){
                assert(campaign_upgrade(c,SYS_WEAPONS));
                if(game_used_power(&c->game)==c->game.player.reactor)game_adjust_power(&c->game,SYS_ENGINES,-1);
                game_adjust_power(&c->game,SYS_WEAPONS,1);
                assert(campaign_swap(c,0,2));
            }
                        if(c->sector>=2&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==2&&c->scrap>=60+campaign_upgrade_price(c,SYS_WEAPONS)+campaign_upgrade_price(c,-1)) {
                assert(campaign_purchase(c,6));assert(campaign_upgrade(c,SYS_WEAPONS));assert(campaign_upgrade(c,-1));
                assert(campaign_swap(c,0,2));game_adjust_power(&c->game,SYS_WEAPONS,1);
            }
            while(c->fuel<8&&campaign_purchase(c,0)){}
            while(c->game.missiles<6&&campaign_purchase(c,1)){}
            if(!c->game.teleporter[0].installed&&c->scrap>=campaign_teleporter_price(c))assert(campaign_install_teleporter(c));
            campaign_continue(c);
        } else if(campaign_choice_count(c)) {
            int id=c->event_id,n=0;
            if(id==EVENT_HAZARD)n=2;
            if(id==EVENT_DISTRESS&&c->fuel<5)n=1;
            if(id==EVENT_REPAIR&&c->game.player.hull<20&&campaign_choice_enabled(c,1))n=1;
            assert(campaign_choice(c,n));
        } else {service(c);campaign_continue(c);}
    }
}
int main(void){Campaign c;const int route[]={1,3,5,7,9,11};int sector,i;campaign_init_legacy_seed(&c,1);
 for(sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++)for(i=0;i<6;i++){assert(campaign_jump(&c,route[i]));boarding_settle(&c);if(c.mode==CAM_COMPLETE)break;}
 assert(c.mode==CAM_COMPLETE&&deployments==1&&recalls==1&&pursuit_ticks>0);assert(game_alive_crew(&c.game)==3&&!game_away_crew(&c.game));
 for(i=0;i<c.game.crew_count;i++){int j;for(j=i+1;j<c.game.crew_count;j++)assert(c.game.crew[i].id!=c.game.crew[j].id);}
 printf("PASS: full campaign with store-bought teleporter, walking/deployment/defender pursuit/recall, preserved identities and finale (hull%d scrap%d).\n",c.game.player.hull,c.scrap);return 0;}
