/* Extended campaign pilot: all recruitment paid from earned scrap; actual disk reloads. */
#include "campaign.h"
#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void graph_checks(void) {
    Campaign c,d;int seed,i,j,k,seen[CAM_LEGACY_NODE_COUNT],changed;
    for(seed=1;seed<=256;seed++) {
        campaign_init_legacy_seed(&c,(unsigned)seed);campaign_init_legacy_seed(&d,(unsigned)seed);
        assert(!memcmp(c.links,d.links,sizeof c.links));assert(!memcmp(c.node_kind,d.node_kind,sizeof c.node_kind));
        memset(seen,0,sizeof seen);seen[0]=1;
        do {changed=0;for(i=0;i<CAM_LEGACY_NODE_COUNT;i++)for(j=0;j<CAM_LEGACY_NODE_COUNT;j++) {
            assert(campaign_linked(&c,i,j)==campaign_linked(&c,j,i));
            if(seen[i]&&!seen[j]&&campaign_linked(&c,i,j)){seen[j]=1;changed=1;}
        }}while(changed);
        for(k=0;k<CAM_LEGACY_NODE_COUNT;k++){assert(seen[k]);assert(campaign_node_x(&c,k)>=0&&campaign_node_x(&c,k)<256);assert(campaign_node_y(&c,k)>40&&campaign_node_y(&c,k)<160);}
        assert(!campaign_linked(&c,-1,0));assert(!campaign_can_jump(&c,11));
    }
}
static void rules_checks(void) {
    Campaign c;int scrap,stock;
    campaign_init_legacy(&c);assert(campaign_jump(&c,2));assert(campaign_choice_count(&c)==2);
    assert(campaign_choice(&c,0));assert(c.fuel==15);assert(!campaign_choice(&c,0));campaign_continue(&c);
    assert(campaign_jump(&c,0));campaign_continue(&c);assert(campaign_jump(&c,2));assert(campaign_choice_count(&c)==0);assert(c.fuel==13);
    campaign_continue(&c);c.fuel=0;scrap=c.scrap;assert(campaign_rescue(&c));assert(c.fuel==2&&c.scrap==scrap-10&&c.sector_jumps==5);assert(!campaign_rescue(&c));
    c.mode=CAM_STORE;c.current=5;c.scrap=500;stock=c.stock[5][4];assert(campaign_purchase(&c,4));assert(c.cargo[0]==2&&c.stock[5][4]==stock-1);assert(!campaign_purchase(&c,4));
    assert(campaign_swap(&c,0,2));assert(c.game.weapon_count==3&&c.cargo[0]==-1&&c.game.weapons[2].catalog_id==2);
    assert(campaign_purchase(&c,5));assert(campaign_swap(&c,0,0));assert(c.cargo[0]==0&&c.game.weapons[0].catalog_id==3);
    assert(campaign_sell_weapon(&c,0,0));assert(c.cargo[0]==-1);assert(!campaign_sell_weapon(&c,0,0));
    assert(campaign_purchase(&c,3));assert(c.game.crew_count==4);
    assert(campaign_upgrade(&c,-1)&&c.game.player.reactor==9);
    assert(campaign_upgrade(&c,SYS_SHIELDS)&&c.game.player.systems[SYS_SHIELDS].capacity==3);
    c.mode=CAM_COMBAT;assert(!campaign_upgrade(&c,-1)&&!campaign_swap(&c,0,0));
    c.mode=CAM_MAP;c.fuel=5;c.sector_jumps=20;c.current=0;assert(campaign_jump(&c,1));assert(c.pursued&&c.mode==CAM_COMBAT);
    /* Event predicates and once-only effects are checked without combat shortcuts. */
    c.mode=CAM_EVENT;c.event_resolved=0;c.event_id=EVENT_HAZARD;c.game.player.hull=3;
    assert(!campaign_choice_enabled(&c,0)&&!campaign_choice(&c,0));assert(!campaign_choice_enabled(&c,1));assert(campaign_choice(&c,2));
    c.mode=CAM_EVENT;c.event_resolved=0;c.event_id=EVENT_DISTRESS;c.fuel=1;assert(!campaign_choice(&c,0));assert(campaign_choice(&c,1));
    c.mode=CAM_EVENT;c.event_resolved=0;c.event_id=EVENT_QUEST;c.quest=0;assert(campaign_choice(&c,0)&&c.quest==1);
    c.event_resolved=0;c.event_id=EVENT_QUEST;scrap=c.scrap;assert(campaign_choice(&c,0)&&c.quest==2&&c.scrap==scrap+45);
}
static void service(Campaign *c) {
    int i,t,repair=0,med=c->game.player.systems[SYS_MEDBAY].room;
    while(repair<c->game.crew_count&&c->game.crew[repair].health<=0)repair++;
    assert(repair<c->game.crew_count);
    game_close_all_doors(&c->game);
    for(i=0;i<GAME_SYSTEM_COUNT;i++)while(game_adjust_power(&c->game,i,-1)){}
    game_adjust_power(&c->game,SYS_OXYGEN,1);
    for(i=0;i<GAME_SYSTEM_COUNT;i++) {
        if(c->game.player.systems[i].health<c->game.player.systems[i].capacity) {
            game_order_crew(&c->game,repair,c->game.player.systems[i].room);
            for(t=0;t<60*GAME_HZ;t++)campaign_step(c);
        }
    }
    if(!c->game.player.systems[SYS_MEDBAY].power) {
        game_adjust_power(&c->game,SYS_ENGINES,-1);
        game_adjust_power(&c->game,SYS_MEDBAY,1);
    }
    for(i=0;i<c->game.crew_count;i++)game_order_crew(&c->game,i,med);
    for(t=0;t<60*GAME_HZ;t++)campaign_step(c);
    game_adjust_power(&c->game,SYS_MEDBAY,-1);
    game_adjust_power(&c->game,SYS_OXYGEN,1);
    game_adjust_power(&c->game,SYS_SHIELDS,1);game_adjust_power(&c->game,SYS_SHIELDS,1);
    game_adjust_power(&c->game,SYS_ENGINES,1);
    game_order_crew(&c->game,0,c->game.pilot_room);
    game_order_crew(&c->game,1,c->game.player.systems[SYS_ENGINES].room);
    game_order_crew(&c->game,2,c->game.player.systems[SYS_WEAPONS].room);
    for(t=0;t<15*GAME_HZ;t++)campaign_step(c);
    while(game_adjust_power(&c->game,SYS_WEAPONS,1)){}
}
static int battles;
/* Scripted pilot uses ordinary crew orders and reactor controls to defend against
 * boarders. It never injects healing, repairs, damage or resources. */
static void fight(Campaign *c) {
    int t,w;battles++;
    for(t=0;t<GAME_HZ*360&&c->mode==CAM_COMBAT;t++) {
        Game*g=&c->game;int i,hostile=-1,repair=-1,healing=0;
        int med=g->player.systems[SYS_MEDBAY].room,weaponroom=g->player.systems[SYS_WEAPONS].room;
        for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==0){hostile=g->enemy_crew[i].room;break;}
        /* V46: weapons fire only while manned, so one healthy crewmember stays a gunner. */
        int gunner=-1;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>=40&&g->crew[i].ship==0&&g->crew[i].destination==weaponroom){gunner=i;break;}
        if(gunner<0)for(i=g->crew_count-1;i>=0;i--)if(g->crew[i].health>=40&&g->crew[i].ship==0){gunner=i;break;}
        for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0){
            Crew*crew=&g->crew[i];int target=-1;
            if(crew->health<40||(crew->destination==med&&crew->health<95)){target=med;healing=1;}
            else if(i==gunner)target=weaponroom;
            else if(hostile>=0)target=hostile;
            else if(repair<0){repair=i;}
            if(target>=0&&crew->destination!=target)game_order_crew(g,i,target);
        }
        if(healing&&!g->player.systems[SYS_MEDBAY].power){
            if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_ENGINES,-1);
            if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_WEAPONS,-1);
            game_adjust_power(g,SYS_MEDBAY,1);
        }
        if(!healing)game_adjust_power(g,SYS_MEDBAY,-1);
        if(hostile<0&&repair>=0){int target=-1;
            if(g->player.systems[SYS_OXYGEN].health<1)target=g->player.systems[SYS_OXYGEN].room;
            else if(g->player.systems[SYS_WEAPONS].health<g->player.systems[SYS_WEAPONS].capacity)target=weaponroom;
            if(target>=0&&g->crew[repair].destination!=target)game_order_crew(g,repair,target);
        }
        if(g->player.systems[SYS_OXYGEN].health>=1)game_adjust_power(g,SYS_OXYGEN,1);
        game_adjust_power(g,SYS_WEAPONS,1);
        for(w=0;w<g->weapon_count;w++)if(g->weapons[w].charge_ticks>=g->weapons[w].cooldown_ticks)game_fire(g,w,g->enemy.systems[SYS_WEAPONS].room);
        campaign_step(c);
    }
    if(c->mode!=CAM_EVENT&&c->mode!=CAM_COMPLETE)fprintf(stderr,"Fight failure sector%d phase%d mode%d hull%d enemy%d ammo%d ticks%d\n",c->sector,c->boss_phase,c->mode,c->game.player.hull,c->game.enemy.hull,c->game.missiles,t);
    assert(c->mode==CAM_EVENT||c->mode==CAM_COMPLETE);
}
static void settle(Campaign *c) {
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
            while(c->fuel<8&&campaign_purchase(c,0)){}
            while(c->game.missiles<6&&campaign_purchase(c,1)){}
            if(c->game.crew_count<GAME_MAX_CREW && c->scrap>=campaign_offer_price(c,3)+20){
                int price=campaign_offer_price(c,3),before=c->scrap,kind=campaign_species_offer(c),slot=c->game.crew_count;
                if(campaign_purchase(c,3)){assert(c->scrap==before-price);assert(c->game.crew[slot].species==kind);}
            }
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
static void species_run(unsigned int seed) {
    Campaign c;int sector,i;const int route[]={1,3,5,7,9,11};campaign_init_legacy_seed(&c,seed);
    for(sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++) {
        assert(c.sector==sector);
        for(i=0;i<6;i++) {
            assert(campaign_jump(&c,route[i]));settle(&c);
            if(c.mode==CAM_MAP){Campaign loaded;assert(campaign_save(&c,"tests/species-run.sav"));assert(campaign_load(&loaded,"tests/species-run.sav"));
                assert(loaded.game.crew_count==c.game.crew_count);for(int k=0;k<c.game.crew_count;k++){assert(loaded.game.crew[k].species==c.game.crew[k].species);assert(loaded.game.crew[k].id==c.game.crew[k].id);assert(loaded.game.crew[k].health==c.game.crew[k].health);}c=loaded;}

            if(c.mode==CAM_COMPLETE)break;
        }
    }
    assert(c.mode==CAM_COMPLETE&&c.boss_phase==2&&c.game.player.hull>0&&c.jumps==36);
    unsigned seen=0;for(int k=0;k<c.game.crew_count;k++)seen|=1u<<c.game.crew[k].species;
    assert(c.game.crew_count>=4 && (seen & (seen-1)));
    printf("Species campaign seed %u complete: crew %d, species mask %u, hull %d, scrap %d\n",seed,c.game.crew_count,seen,c.game.player.hull,c.scrap);
}
int main(void){graph_checks();rules_checks();species_run(1);species_run(42);remove("tests/species-run.sav");remove("tests/species-run.sav.bak");puts("PASS: earned species recruitment and mixed-roster cold-load campaign runs.");return 0;}
