#include "campaign.h"
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
/* Resolve actual encounter offers before any other campaign action. */
static int reward_offers[3],reward_accepts[3],reward_declines[3];
static void resolve_rewards(Campaign*c){
 if(c->reward.kind){int kind=c->reward.kind;assert(kind==REWARD_WEAPON||kind==REWARD_CREW);reward_offers[kind]++;
  if(rewards_accept(&c->reward,&c->game,c->cargo,CAM_CARGO_COUNT,-1,0))reward_accepts[kind]++;
  else {assert(rewards_decline(&c->reward));reward_declines[kind]++;}
  assert(!c->reward.kind);
 }
}
static void reward_evidence(void){
 printf("Earned rewards: weapons offered%d accepted%d declined%d; recruits offered%d accepted%d declined%d.\n",reward_offers[REWARD_WEAPON],reward_accepts[REWARD_WEAPON],reward_declines[REWARD_WEAPON],reward_offers[REWARD_CREW],reward_accepts[REWARD_CREW],reward_declines[REWARD_CREW]);
 for(int i=1;i<=2;i++)assert(reward_offers[i]==reward_accepts[i]+reward_declines[i]);
}
static int free_cargo(const Campaign*c){for(int i=0;i<CAM_CARGO_COUNT;i++)if(c->cargo[i]<0)return i;return -1;}
static int flagship_engagements,flagship_stage_wins,flagship_defeats,flagship_last_defeat;
static void service(Campaign *c) {
    resolve_rewards(c);
    int i,t,alive=0,med=c->game.player.systems[SYS_MEDBAY].room;
    for(i=0;i<c->game.crew_count;i++)alive+=c->game.crew[i].health>0;
    assert(alive);
    game_close_all_doors(&c->game);
    for(i=0;i<GAME_SYSTEM_COUNT;i++)while(game_adjust_power(&c->game,i,-1)){}
    game_adjust_power(&c->game,SYS_OXYGEN,1);game_adjust_power(&c->game,SYS_MEDBAY,1);
    /* Ordinary crew orders, re-evaluated every two seconds like a player: the
     * hurt heal, everyone else fights the worst fire or breach, then one crew
     * member repairs systems (oxygen first) while oxygen and medbay stay powered. */
    for(int step=0;step<180&&c->mode!=CAM_DEFEAT;step++) {
        Game*g=&c->game;GameRoom*hazard=NULL;int hurt=0,sys=-1,worker=-1;
        static const int order[GAME_SYSTEM_COUNT]={SYS_OXYGEN,SYS_SHIELDS,SYS_ENGINES,SYS_WEAPONS,SYS_MEDBAY};
        if(g->player.systems[SYS_OXYGEN].health>0&&!g->player.systems[SYS_OXYGEN].power)game_adjust_power(g,SYS_OXYGEN,1);
        if(g->player.systems[SYS_MEDBAY].health>0&&!g->player.systems[SYS_MEDBAY].power)game_adjust_power(g,SYS_MEDBAY,1);
        for(i=0;i<g->player.room_count;i++){GameRoom*q=&g->player.rooms[i];if((q->breaches||q->fires)&&(!hazard||q->fires>hazard->fires))hazard=q;}
        for(i=0;i<g->crew_count;i++){if(g->crew[i].health<=0)continue;if(g->crew[i].health<45)hurt=1;else if(worker<0)worker=i;}
        /* V48: boarders left aboard after a fight (flagship invasion) are fought first. */
        int intruder=-1;for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].ship==0&&g->enemy_crew[i].health>0){intruder=g->enemy_crew[i].room;break;}
        for(int k=0;k<GAME_SYSTEM_COUNT&&sys<0;k++)if(g->player.systems[order[k]].health<g->player.systems[order[k]].capacity)sys=order[k];
        if(!hazard&&sys<0&&!hurt&&intruder<0)break;
        /* Three or more burning rooms: gather in a quiet room (medbay first) and
         * vent the rest of the ship through open doors, then reseal it. */
        {int burning=0,safe=-1,o2fire=0;for(i=0;i<g->player.room_count;i++){burning+=g->player.rooms[i].fires>0;o2fire|=g->player.rooms[i].fires>0&&g->player.rooms[i].id==g->player.systems[SYS_OXYGEN].room;}
         /* V60: never vent while the oxygen room burns (open doors spread fire into it); fight that fire first. */
         if(burning>=4&&!o2fire){
            for(i=0;i<g->player.room_count;i++){const GameRoom*q=&g->player.rooms[i];if(!q->fires&&!q->breaches&&(safe<0||q->id==med))safe=q->id;}
            if(safe>=0){int n,k;GameDoor*doors=game_ship_doors(g,0,&n);
                for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0)game_order_crew(g,i,safe);
                for(t=0;t<4*GAME_HZ;t++)campaign_step(c);
                for(k=0;k<n;k++)if(doors[k].room_a!=safe&&doors[k].room_b!=safe&&!doors[k].open)game_toggle_door(g,k);
                for(t=0;t<40*GAME_HZ&&c->mode!=CAM_DEFEAT;t++){int left=0;campaign_step(c);for(i=0;i<g->player.room_count;i++)left+=g->player.rooms[i].fires>0;if(!left)break;}
                game_close_all_doors(g);continue;}}}
        /* Broken oxygen (or a broken medbay while someone is hurt) is critical work for
         * the first living crew member, whatever their health. */
        /* V53: the hurt shelter in the medbay unless it has no air; then in the room with the most. */
        int shelter=med;{int best=-1,air=-1;for(i=0;i<g->player.room_count;i++){const GameRoom*q=&g->player.rooms[i];if(q->id==med&&q->oxygen>=200){best=-1;break;}if(q->oxygen>air){air=q->oxygen;best=q->id;}}if(best>=0)shelter=best;}
        int critical=!g->player.systems[SYS_OXYGEN].health?SYS_OXYGEN:(hurt&&!g->player.systems[SYS_MEDBAY].health)?SYS_MEDBAY:-1,first=-1;
        /* V48: the healthiest repairs; V55: whoever is already assigned keeps the job. */
        if(critical>=0)for(i=0;i<g->crew_count&&first<0;i++)if(g->crew[i].health>0&&g->crew[i].destination==g->player.systems[critical].room)first=i;
        if(first<0)for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&(first<0||g->crew[i].health>g->crew[first].health))first=i;
        for(i=0;i<g->crew_count;i++){if(g->crew[i].health<=0)continue;
            int target=(critical>=0&&i==first)?g->player.systems[critical].room:g->crew[i].health<45?shelter:intruder>=0?intruder:hazard?hazard->id:(sys>=0&&i==worker)?g->player.systems[sys].room:med;
            if(g->crew[i].destination!=target)game_order_crew(g,i,target);}
        for(t=0;t<2*GAME_HZ&&c->mode!=CAM_DEFEAT;t++)campaign_step(c);
    }
    /* A crew lost to Last Stand damage right after a flagship stage ends the run in the finale. */
    if(c->mode==CAM_DEFEAT){if(c->sector==c->sector_count-1)flagship_last_defeat=1;return;}
    if(!c->game.player.systems[SYS_MEDBAY].power) {
        game_adjust_power(&c->game,SYS_ENGINES,-1);
        game_adjust_power(&c->game,SYS_MEDBAY,1);
    }
    for(i=0;i<c->game.crew_count;i++)game_order_crew(&c->game,i,med);
    for(t=0;t<60*GAME_HZ;t++)campaign_step(c);
    game_adjust_power(&c->game,SYS_MEDBAY,-1);
    game_adjust_power(&c->game,SYS_OXYGEN,1);
    game_adjust_power(&c->game,SYS_SHIELDS,1);game_adjust_power(&c->game,SYS_SHIELDS,1);
    if(c->finale_policy&&c->sector>=5)while(game_adjust_power(&c->game,SYS_SHIELDS,1)){}
    game_adjust_power(&c->game,SYS_ENGINES,1);
    game_order_crew(&c->game,0,c->game.pilot_room);
    game_order_crew(&c->game,1,c->game.player.systems[SYS_ENGINES].room);
    game_order_crew(&c->game,2,c->game.player.systems[SYS_WEAPONS].room);
    for(t=0;t<15*GAME_HZ;t++)campaign_step(c);
    while(game_adjust_power(&c->game,SYS_WEAPONS,1)){}
}
static int battles;
/* V42: authentic flagship outcomes are recorded; a loss there ends a current
 * eight-sector fixture run (finale_policy 1) instead of failing the harness. */
/* Scripted pilot uses ordinary crew orders and reactor controls to defend against
 * boarders. It never injects healing, repairs, damage or resources. */
static int burning_room(const Game*g){int best=-1,fires=0;
    for(int i=0;i<g->player.room_count;i++){const GameRoom*r=&g->player.rooms[i];if(r->fires>fires&&r->oxygen>=100){fires=r->fires;best=r->id;}}
    return best;
}
static int bot_retreats,tolerate_late_losses,late_losses;
static void fight(Campaign *c) {
    int t,w,flagship=game_is_flagship(&c->game);battles++;flagship_engagements+=flagship;
    for(t=0;t<GAME_HZ*(flagship?600:360)&&c->mode==CAM_COMBAT;t++) {
        Game*g=&c->game;int i,hostile=-1,repair=-1,healing=0;
        int med=g->player.systems[SYS_MEDBAY].room,weaponroom=g->player.systems[SYS_WEAPONS].room;
        for(i=0;i<g->enemy_crew_count;i++)if(g->enemy_crew[i].health>0&&g->enemy_crew[i].ship==0){hostile=g->enemy_crew[i].room;break;}
        /* V46: weapons fire only while manned, so one healthy crewmember stays a gunner. */
        int gunner=-1;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>=40&&g->crew[i].ship==0&&g->crew[i].destination==weaponroom){gunner=i;break;}
        if(gunner<0)for(i=g->crew_count-1;i>=0;i--)if(g->crew[i].health>=40&&g->crew[i].ship==0){gunner=i;break;}
        /* Nobody holds a station in a room without air. */
        {const GameRoom*wr=0;for(i=0;i<g->player.room_count;i++)if(g->player.rooms[i].id==weaponroom)wr=&g->player.rooms[i];if(wr&&wr->oxygen<150)gunner=-1;}
        for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0){
            Crew*crew=&g->crew[i];int target=-1;
            if(crew->health<40||(crew->destination==med&&crew->health<95)){target=med;healing=1;}
            else if(i==gunner)target=weaponroom;
            else if(hostile>=0)target=hostile;
            else if(repair<0){repair=i;}
            /* V60: spare healthy crew put out fires (solar flares start several). */
            else if(crew->health>=50&&burning_room(g)>=0)target=burning_room(g);
            if(target>=0&&crew->destination!=target)game_order_crew(g,i,target);
        }
        if(healing&&!g->player.systems[SYS_MEDBAY].power){
            if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_ENGINES,-1);
            if(game_used_power(g)>=g->player.reactor)game_adjust_power(g,SYS_WEAPONS,-1);
            game_adjust_power(g,SYS_MEDBAY,1);
        }
        if(!healing)game_adjust_power(g,SYS_MEDBAY,-1);
        /* Oxygen damage is the fastest way to lose a crew: repair it first,
         * even while boarders are aboard, in current eight-sector runs. */
        /* V55 emergency: oxygen destroyed and air running out: the healthiest crewmember repairs it now. */
        if(!g->player.systems[SYS_OXYGEN].health&&g->player.oxygen<450){int best=-1;for(i=0;i<g->crew_count;i++)if(g->crew[i].health>0&&g->crew[i].ship==0&&(best<0||g->crew[i].destination==g->player.systems[SYS_OXYGEN].room||(g->crew[best].destination!=g->player.systems[SYS_OXYGEN].room&&g->crew[i].health>g->crew[best].health)))best=i;
            if(best>=0&&g->crew[best].destination!=g->player.systems[SYS_OXYGEN].room){game_order_crew(g,best,g->player.systems[SYS_OXYGEN].room);}if(best==repair)repair=-1;}
        if(repair>=0&&g->player.systems[SYS_OXYGEN].health<g->player.systems[SYS_OXYGEN].capacity&&g->crew[repair].destination!=g->player.systems[SYS_OXYGEN].room)game_order_crew(g,repair,g->player.systems[SYS_OXYGEN].room);
        else if(hostile<0&&repair>=0){int target=-1;
            if(g->player.systems[SYS_OXYGEN].health<1)target=g->player.systems[SYS_OXYGEN].room;
            /* V46 fires grow inside rooms: put out the largest fire that still has air. */
            else if(c->finale_policy&&(target=burning_room(g))>=0){}
            /* Current eight-sector runs repair shields first (the flagship's main threat). */
            else if(c->finale_policy&&g->player.systems[SYS_SHIELDS].health<g->player.systems[SYS_SHIELDS].capacity)target=g->player.systems[SYS_SHIELDS].room;
            else if(g->player.systems[SYS_WEAPONS].health<g->player.systems[SYS_WEAPONS].capacity)target=weaponroom;
            if(target>=0&&g->crew[repair].destination!=target)game_order_crew(g,repair,target);
        }
        if(g->player.systems[SYS_OXYGEN].health>=1)game_adjust_power(g,SYS_OXYGEN,1);
        game_adjust_power(g,SYS_WEAPONS,1);
        /* The V42 flagship has artillery rooms instead of a weapons system. Like a
         * player, knock out the shield-piercing missiles first, then the lasers,
         * firing whole volleys so the four shield layers fall together. */
        if(game_is_flagship(g)){int aim=-1,ready=1;static const int order[]={1,0,2,3};
            for(i=0;i<4&&aim<0;i++)for(int k=0;k<g->artillery_count;k++)if(g->artillery[k].weapon==order[i]&&g->artillery[k].health>0)aim=g->artillery[k].room;
            if(aim<0)aim=g->enemy.systems[SYS_SHIELDS].room;
            for(w=0;w<g->weapon_count;w++)if(game_weapon_powered(g,w)&&g->weapons[w].charge_ticks<g->weapons[w].cooldown_ticks)ready=0;
            if(ready)for(w=0;w<g->weapon_count;w++)game_fire(g,w,aim);
        }
        else{/* V53: against 2+ shield layers, knock out shields first (like a player), then weapons. */
            int aim=g->enemy.systems[SYS_SHIELDS].room>=0&&g->enemy.systems[SYS_SHIELDS].health>=2?g->enemy.systems[SYS_SHIELDS].room:g->enemy.systems[SYS_WEAPONS].room;
            if(aim<0)aim=g->enemy.rooms[0].id;
            for(w=0;w<g->weapon_count;w++)if(g->weapons[w].charge_ticks>=g->weapons[w].cooldown_ticks)game_fire(g,w,aim);}
        /* V62: like a player, jump away when the hull is nearly gone and the FTL drive is ready. */
        if(!flagship&&g->outcome==GAME_PLAYING&&g->player.hull<=10&&g->enemy.hull>3&&g->player_ftl_charge>=GAME_PLAYER_FTL_CHARGE&&c->fuel>0){
            for(int n=0;n<c->node_count;n++)if(n!=c->current&&campaign_linked(c,c->current,n)&&!campaign_node_captured(c,n,1)&&campaign_retreat(c,n,0)){bot_retreats++;return;}}
        campaign_step(c);
    }
    if(flagship&&(c->mode==CAM_EVENT||c->mode==CAM_COMPLETE))flagship_stage_wins++;
    if(flagship&&c->finale_policy&&c->mode==CAM_DEFEAT){flagship_defeats++;flagship_last_defeat=1;return;}
    if(c->mode!=CAM_EVENT&&c->mode!=CAM_COMPLETE){fprintf(stderr,"[%s+%s ion%d] ",c->game.enemy_weapon_count?game_weapon_short(c->game.enemy_weapon.catalog_id):"-",c->game.enemy_weapon_count>1?game_weapon_short(c->game.enemy_weapon_secondary.catalog_id):"-",c->game.player.systems[SYS_WEAPONS].ion_bars);fprintf(stderr,"Fight failure vs %s: sector%d phase%d mode%d hull%d enemy%d ammo%d ticks%d\n",game_enemy_name(&c->game),c->sector,c->boss_phase,c->mode,c->game.player.hull,c->game.enemy.hull,c->game.missiles,t);}
    /* V62: a harness that opts in may lose a few ordinary fights from sector 3 on (original boarder and hazard events); see surrender_run_smoke.c. */
    if(c->mode==CAM_DEFEAT&&tolerate_late_losses&&c->sector>=2&&late_losses<tolerate_late_losses){late_losses++;return;}
    assert(c->mode==CAM_EVENT||c->mode==CAM_COMPLETE);
}
/* Current eight-sector runs: from sector 6, like a player preparing for the
 * flagship, spend spare scrap on shield layers and the reactor to power them. */
static int at_most(int a,int b){return a<b?a:b;}
static void invest(Campaign*c){
    if(!c->finale_policy||c->sector<3||c->mode!=CAM_MAP||c->reward.kind)return;
    Game*g=&c->game;int need=0;for(int i=0;i<g->weapon_count;i++)need+=g->weapons[i].power;
    /* Reactor first: every weapon, four shield bars, two engine bars and oxygen. */
    for(int guard=0;guard<12;guard++){int want=need+at_most(g->player.systems[SYS_SHIELDS].capacity,6)+at_most(g->player.systems[SYS_ENGINES].capacity,2)+1,r=campaign_upgrade_price(c,-1);
        if(g->player.reactor>=want||r<0||c->scrap<r+10||!campaign_upgrade(c,-1))break;}
    while(g->player.systems[SYS_WEAPONS].capacity<need&&campaign_upgrade_price(c,SYS_WEAPONS)>=0&&c->scrap>=campaign_upgrade_price(c,SYS_WEAPONS)+10&&campaign_upgrade(c,SYS_WEAPONS)){}
    for(int guard=0;guard<4&&g->player.systems[SYS_SHIELDS].capacity<6;guard++){int price=campaign_upgrade_price(c,SYS_SHIELDS);
        if(price<0||c->scrap<price+10||!campaign_upgrade(c,SYS_SHIELDS))break;
        int r=campaign_upgrade_price(c,-1);if(r>=0&&c->scrap>=r)campaign_upgrade(c,-1);}
    for(int k=0;k<GAME_SYSTEM_COUNT;k++)while(game_adjust_power(g,k,-1)){}
    game_adjust_power(g,SYS_OXYGEN,1);while(game_adjust_power(g,SYS_WEAPONS,1)){}while(game_adjust_power(g,SYS_SHIELDS,1)){}while(game_adjust_power(g,SYS_ENGINES,1)){}
}
static void settle(Campaign *c) {
    while(c->reward.kind||(c->mode!=CAM_MAP&&c->mode!=CAM_COMPLETE)) {
        if(c->reward.kind){resolve_rewards(c);continue;}
        if(c->mode==CAM_COMBAT)fight(c);
        else if(c->mode==CAM_STORE){
            int cargo_slot=free_cargo(c);
            while(c->game.player.hull<=25&&campaign_purchase(c,2)){}
            if(cargo_slot>=0&&c->game.weapon_count==2&&campaign_upgrade_price(c,SYS_WEAPONS)>=0&&c->scrap>=25+campaign_upgrade_price(c,SYS_WEAPONS)&&campaign_purchase(c,4)){
                assert(campaign_upgrade(c,SYS_WEAPONS));
                if(game_used_power(&c->game)==c->game.player.reactor)game_adjust_power(&c->game,SYS_ENGINES,-1);
                game_adjust_power(&c->game,SYS_WEAPONS,1);
                assert(campaign_swap(c,cargo_slot,2));
            }
                        cargo_slot=free_cargo(c);
            if(cargo_slot>=0&&c->sector>=2&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==2&&c->scrap>=60+campaign_upgrade_price(c,SYS_WEAPONS)+campaign_upgrade_price(c,-1)) {
                assert(campaign_purchase(c,6));assert(campaign_upgrade(c,SYS_WEAPONS));assert(campaign_upgrade(c,-1));
                assert(campaign_swap(c,cargo_slot,2));game_adjust_power(&c->game,SYS_WEAPONS,1);
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
    invest(c);
}
static void full_run(unsigned int seed) {
    Campaign c;int sector,i;const int route[]={1,3,5,7,9,11};campaign_init_legacy_seed(&c,seed);
    for(sector=0;sector<CAM_LEGACY_SECTOR_COUNT;sector++) {
        assert(c.sector==sector);
        for(i=0;i<6;i++) {
            assert(campaign_jump(&c,route[i]));settle(&c);
            if(c.mode==CAM_COMPLETE)break;
        }
    }
    assert(c.mode==CAM_COMPLETE&&c.boss_phase==2&&c.game.player.hull>0&&c.jumps==36);
    printf("Campaign seed %u complete: hull %d, scrap %d, ammo %d\n",seed,c.game.player.hull,c.scrap,c.game.missiles);
}
int main(void){graph_checks();rules_checks();full_run(1);full_run(42);full_run(8675309);assert(battles>=27);reward_evidence();puts("PASS: 256 generated graphs, events/economy/inventory/pursuit/rescue and three real six-sector campaigns including all boss phases.");return 0;}






