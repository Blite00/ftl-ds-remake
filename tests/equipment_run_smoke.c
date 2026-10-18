/* Public-API inventory management and earned-economy campaign, no synthetic loot. */
#define main original_campaign_main
#include "campaign_v10_smoke.c"
#undef main
#include "save.h"
static int cargo_sale,installed_sale,checkpoints;
static const char*path="tests/equipment-run.sav";
static void checkpoint(Campaign*c){static Campaign loaded;
 assert(c->mode==CAM_MAP&&campaign_save(c,path));campaign_init_legacy_seed(&loaded,999);assert(campaign_load(&loaded,path));
 assert(loaded.scrap==c->scrap&&loaded.fuel==c->fuel&&loaded.jumps==c->jumps);
 assert(!memcmp(loaded.stock,c->stock,sizeof c->stock)&&!memcmp(loaded.cargo,c->cargo,sizeof c->cargo));
 assert(loaded.game.weapon_count==c->game.weapon_count);
 for(int i=0;i<c->game.weapon_count;i++)assert(loaded.game.weapons[i].catalog_id==c->game.weapons[i].catalog_id&&loaded.game.weapons[i].enabled==c->game.weapons[i].enabled);
 *c=loaded;checkpoints++;
}
static void equipment_settle(Campaign *c) {
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
            if(c->sector>=3&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==1&&c->scrap>=80){
                if(!cargo_sale&&campaign_purchase(c,4)){
                    int before=c->scrap,price=campaign_weapon_sell_price(c,0,0);
                    assert(c->cargo[0]==2&&price==game_weapon_price(2)/2);
                    assert(campaign_sell_weapon(c,0,0)&&c->scrap==before+price&&c->cargo[0]==-1);
                    assert(c->game.weapon_count==3);cargo_sale++;
                }
                if(!installed_sale&&campaign_purchase(c,5)){
                    int id=c->cargo[0],before=c->scrap;
                    assert(id==3&&campaign_swap(c,0,3));
                    int price=campaign_weapon_sell_price(c,1,3);
                    assert(price==game_weapon_price(id)/2&&campaign_sell_weapon(c,1,3));
                    assert(c->scrap==before+price&&c->game.weapon_count==3&&c->cargo[0]==-1);
                    assert(c->game.weapons[0].catalog_id==0&&c->game.weapons[1].catalog_id==1&&c->game.weapons[2].catalog_id==1);installed_sale++;
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

int main(void){static Campaign c;const int route[]={1,3,5,7,9,11};
 const char*files[]={"tests/equipment-run.sav","tests/equipment-run.sav.bak","tests/equipment-run.sav.tmp","tests/equipment-run.sav.bad"};
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");assert(!f);}
 campaign_init_legacy_seed(&c,1);
 /* Existing starting weapons move through cargo without creating or losing items. */
 assert(campaign_unequip(&c,0)&&c.game.weapon_count==1&&c.cargo[0]==0);
 assert(campaign_unequip(&c,0)&&c.game.weapon_count==0&&c.cargo[1]==1);checkpoint(&c);
 assert(c.game.weapon_count==0&&c.cargo[0]==0&&c.cargo[1]==1);
 assert(campaign_swap(&c,0,0)&&campaign_swap(&c,1,1));
 assert(campaign_reorder_weapon(&c,1,0));checkpoint(&c);
 assert(c.game.weapons[0].catalog_id==1&&c.game.weapons[1].catalog_id==0);
 assert(campaign_reorder_weapon(&c,0,1));checkpoint(&c);
 assert(c.game.weapons[0].catalog_id==0&&c.game.weapons[1].catalog_id==1);
 for(int sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++)for(int i=0;i<6;i++){
  assert(campaign_jump(&c,route[i]));equipment_settle(&c);if(c.mode==CAM_COMPLETE)break;checkpoint(&c);
 }
 assert(c.mode==CAM_COMPLETE&&c.jumps==36&&cargo_sale==1&&installed_sale==1&&checkpoints==38);
 for(int i=0;i<4;i++){FILE*f=fopen(files[i],"rb");if(f){fclose(f);assert(!remove(files[i]));}}
 printf("PASS: unequip to zero, reinstall/reorder, exact cargo/installed sales from earned purchases, six-sector victory and %d cold loads (hull%d scrap%d).\n",checkpoints,c.game.player.hull,c.scrap);return 0;
}
