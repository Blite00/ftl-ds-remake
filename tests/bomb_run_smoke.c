/* Ordinary earned-economy campaign. No injected resources, damage or victories. */
#include "campaign.h"
#include "save.h"
static int bomb_system_hits,bomb_effect_ticks,bomb_bought,checkpoints;
static void observed_step(Campaign*c){
    int health[GAME_SYSTEM_COUNT],pending[GAME_MAX_EFFECTS];
    for(int i=0;i<GAME_SYSTEM_COUNT;i++)health[i]=c->game.enemy.systems[i].health;
    for(int i=0;i<GAME_MAX_EFFECTS;i++)pending[i]=c->game.effects[i].active&&c->game.effects[i].kind==EFFECT_BOMB&&c->game.effects[i].result==EFFECT_PENDING;
    campaign_step(c);
    for(int i=0;i<GAME_MAX_EFFECTS;i++)if(c->game.effects[i].active&&c->game.effects[i].kind==EFFECT_BOMB){
        bomb_effect_ticks++;
        if(pending[i]&&c->game.effects[i].result==EFFECT_HIT)for(int j=0;j<GAME_SYSTEM_COUNT;j++)
            if(c->game.enemy.systems[j].room==c->game.effects[i].room&&c->game.enemy.systems[j].health<health[j])bomb_system_hits++;
    }
}
static int observed_fire(Game*g,int slot,int target){
 if(g->weapons[slot].catalog_id==6){int best=0;
  for(int i=0;i<GAME_SYSTEM_COUNT;i++)if(g->enemy.systems[i].room>=0&&g->enemy.systems[i].health>best){best=g->enemy.systems[i].health;target=g->enemy.systems[i].room;}
 }
 return game_fire(g,slot,target);
}
#define game_fire observed_fire
#define campaign_step observed_step
#define main original_campaign_main
#include "campaign_v10_smoke.c"
#undef main
#undef campaign_step
#undef game_fire
static void bomb_settle(Campaign *c) {
    while(c->mode!=CAM_MAP&&c->mode!=CAM_COMPLETE) {
        if(c->mode==CAM_COMBAT)fight(c);
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
            if(!bomb_bought&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==1){
                int cost=campaign_offer_price(c,10)+campaign_upgrade_price(c,SYS_WEAPONS)+campaign_upgrade_price(c,-1);
                if(c->scrap>=cost){
                    int paid=campaign_offer_price(c,10),before=c->scrap,stock=c->stock[c->current][10];
                    assert(campaign_purchase(c,10));assert(c->scrap==before-paid&&c->stock[c->current][10]==stock-1);
                    assert(campaign_upgrade(c,SYS_WEAPONS));assert(campaign_upgrade(c,-1));
                    int cargo=-1;for(int j=0;j<CAM_CARGO_COUNT;j++)if(c->cargo[j]==6){cargo=j;break;}
                    assert(cargo>=0&&campaign_swap(c,cargo,3));
                    assert(game_adjust_power(&c->game,SYS_WEAPONS,1));bomb_bought=1;
                }
            }
            while(c->fuel<8&&campaign_purchase(c,0)){}
            while(c->game.missiles<6&&campaign_purchase(c,1)){}
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

static const char*path="tests/bomb-run.sav";
static void checkpoint(Campaign*c){static Campaign loaded;
 assert(campaign_save(c,path));campaign_init_legacy_seed(&loaded,999);assert(campaign_load(&loaded,path));
 assert(loaded.scrap==c->scrap&&loaded.fuel==c->fuel&&loaded.jumps==c->jumps);
 assert(!memcmp(loaded.stock,c->stock,sizeof c->stock)&&!memcmp(loaded.cargo,c->cargo,sizeof c->cargo));
 assert(loaded.game.weapon_count==c->game.weapon_count);
 for(int i=0;i<c->game.weapon_count;i++)assert(loaded.game.weapons[i].catalog_id==c->game.weapons[i].catalog_id);
 if(bomb_bought)assert(loaded.game.weapons[3].catalog_id==6&&loaded.game.weapons[3].system_damage==2);
 *c=loaded;checkpoints++;
}
int main(void){static Campaign c;const int route[]={1,3,5,7,9,11};
 const char*files[]={"tests/bomb-run.sav","tests/bomb-run.sav.bak","tests/bomb-run.sav.tmp","tests/bomb-run.sav.bad"};
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");assert(!f);}
 campaign_init_legacy_seed(&c,1);checkpoint(&c);
 for(int sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++)for(int i=0;i<6;i++){
  assert(campaign_jump(&c,route[i]));bomb_settle(&c);if(c.mode==CAM_COMPLETE)break;checkpoint(&c);
 }
 assert(c.mode==CAM_COMPLETE&&c.jumps==36&&bomb_bought&&bomb_effect_ticks>0&&bomb_system_hits>0&&checkpoints==36);
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");if(f){fclose(f);assert(!remove(files[i]));}}
 printf("PASS: earned store Small Bomb, actual bomb impacts/system damage, complete six-sector run and %d cold loads (hull%d scrap%d).\n",checkpoints,c.game.player.hull,c.scrap);return 0;
}
