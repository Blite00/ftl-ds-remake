/* Earned Defense I campaign: no resource, damage or victory injection.
   Count only pending enemy missile transitions to EFFECT_INTERCEPT, never misses or shield hits. */
#include "campaign.h"
#include "save.h"
#include "drones.h"
#include <assert.h>
static int drone_bought,parts_bought,schematic_bought,deployments,interceptions,checkpoints;
static void observed_step(Campaign*c){
 Game*g=&c->game;int pending[GAME_MAX_EFFECTS];
 if(c->mode==CAM_COMBAT&&g->outcome==GAME_PLAYING&&schematic_bought&&g->drone_parts>0&&!g->drones.slots[0].deployed){
  while(g->drones.power<2&&g->drones.health>=2){
   if(game_used_power(g)>=g->player.reactor){
    if(!game_adjust_power(g,SYS_ENGINES,-1)&&!game_adjust_power(g,SYS_WEAPONS,-1))break;
   }
   if(!drones_adjust_power(g,1))break;
  }
  int before=g->drone_parts;
  if(drones_toggle(g,0)){assert(g->drone_parts==before-1);deployments++;}
 }
 for(int i=0;i<GAME_MAX_EFFECTS;i++){
  GameEffect*e=&g->effects[i];
  pending[i]=e->active&&e->side==1&&e->kind==EFFECT_MISSILE&&e->result==EFFECT_PENDING;
 }
 campaign_step(c);
 for(int i=0;i<GAME_MAX_EFFECTS;i++){
  GameEffect*e=&g->effects[i];
  if(pending[i]&&e->active&&e->side==1&&e->kind==EFFECT_MISSILE&&e->result==EFFECT_INTERCEPT)interceptions++;
 }
}
static int observed_fire(Game*g,int slot,int target){
 /* Let a real missile-capable enemy fire before suppressing its weapons. This
    waits for evidence of interception without generating a synthetic attack. */
 if(schematic_bought&&!interceptions&&g->enemy_weapon.missiles&&!g->enemy_weapon.bomb&&g->drone_parts+g->drones.slots[0].deployed>0)return 0;
 return game_fire(g,slot,target);
}
#define game_fire observed_fire
#define campaign_step observed_step
#define main original_campaign_main
#include "campaign_v10_smoke.c"
#undef main
#undef campaign_step
#undef game_fire
static void defense_settle(Campaign *c) {
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
            if(!drone_bought&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==1&&c->scrap>=85){
                int before=c->scrap,stock=c->stock[c->current][11],parts=c->game.drone_parts;
                assert(campaign_purchase(c,11));
                assert(c->scrap==before-85&&c->stock[c->current][11]==stock-1);
                assert(c->game.drones.installed&&c->game.drones.slots[0].catalog_id==0&&c->game.drone_parts==parts+2);
                drone_bought=1;
            }
            if(drone_bought&&c->game.drone_parts<6&&c->scrap>=24){
                int before=c->scrap,parts=c->game.drone_parts,stock=c->stock[c->current][13];
                if(campaign_purchase(c,13)){
                    assert(c->scrap==before-24&&c->game.drone_parts==parts+3&&c->stock[c->current][13]==stock-1);parts_bought++;
                }
            }
            if(drone_bought&&!schematic_bought&&c->scrap>=drones_price(1)){
                int before=c->scrap,stock=c->stock[c->current][14];
                assert(campaign_purchase(c,14));
                assert(c->scrap==before-drones_price(1)&&c->stock[c->current][14]==stock-1&&c->drone_cargo[0]==1);
                assert(campaign_drone_swap(c,0,0)&&c->drone_cargo[0]==0&&c->game.drones.slots[0].catalog_id==1);
                schematic_bought=1;
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
        } else {
            while(drones_adjust_power(&c->game,-1)){}
            service(c);
            if(c->game.drones.installed&&c->game.drones.health<c->game.drones.capacity){
                int crew=0;while(crew<c->game.crew_count&&c->game.crew[crew].health<=0)crew++;
                assert(crew<c->game.crew_count&&game_order_crew(&c->game,crew,c->game.drones.room));
                for(int t=0;t<60*GAME_HZ;t++)campaign_step(c);
            }
            campaign_continue(c);
        }
    }
}

static const char*path="tests/defense-run.sav";
static void checkpoint(Campaign*c){static Campaign loaded;
 assert(campaign_save(c,path));
 {unsigned char header[12];FILE*f=fopen(path,"rb");assert(f&&fread(header,1,sizeof header,f)==sizeof header);fclose(f);assert(header[8]==32&&header[9]==0&&header[10]==0&&header[11]==0);}
 campaign_init_legacy_seed(&loaded,999);assert(campaign_load(&loaded,path));
 assert(loaded.scrap==c->scrap&&loaded.fuel==c->fuel&&loaded.jumps==c->jumps);
 assert(!memcmp(loaded.stock,c->stock,sizeof c->stock)&&!memcmp(loaded.cargo,c->cargo,sizeof c->cargo));
 assert(!memcmp(loaded.drone_cargo,c->drone_cargo,sizeof c->drone_cargo));
 assert(loaded.game.drone_parts==c->game.drone_parts&&loaded.game.drones.installed==c->game.drones.installed);
 assert(loaded.game.drones.capacity==c->game.drones.capacity&&loaded.game.drones.health==c->game.drones.health);
 for(int i=0;i<GAME_MAX_DRONES;i++){
  assert(loaded.game.drones.slots[i].catalog_id==c->game.drones.slots[i].catalog_id);
  assert(!loaded.game.drones.slots[i].deployed&&!loaded.game.drones.slots[i].enabled);
 }
 *c=loaded;checkpoints++;
}
int main(void){static Campaign c;const int route[]={1,3,5,7,9,11};
 const char*files[]={"tests/defense-run.sav","tests/defense-run.sav.bak","tests/defense-run.sav.tmp","tests/defense-run.sav.bad"};
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");assert(!f);}
 campaign_init_legacy_seed(&c,1);checkpoint(&c);
 for(int sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++)for(int i=0;i<6;i++){
  assert(campaign_jump(&c,route[i]));defense_settle(&c);if(c.mode==CAM_COMPLETE)break;checkpoint(&c);
 }
 assert(c.mode==CAM_COMPLETE&&c.boss_phase==2&&c.jumps==36&&drone_bought&&parts_bought&&schematic_bought&&deployments>0&&interceptions>0&&checkpoints==36);
 assert(c.game.drone_parts==4+3*parts_bought-deployments);
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");if(f){fclose(f);assert(!remove(files[i]));}}
 printf("PASS: earned Defense I package, schematic and parts; %d deployments, %d actual incoming missile interceptions; six-sector victory and %d cold loads (hull%d scrap%d).\n",deployments,interceptions,checkpoints,c.game.player.hull,c.scrap);return 0;
}
