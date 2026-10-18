#include "campaign.h"
#include "achievements.h"
#include "drones.h"
#include "species.h"
#include "boarding.h"
#include "outcome.h"
#include "enemy_catalog.h"
#include "weapon_catalog.h"
#include "enemy_weapon_variants.h"
#include <stdio.h>
#include <string.h>

static unsigned int random_next(Campaign *c) { c->rng=c->rng*1664525u+1013904223u;return c->rng; }
static int bounded(Campaign *c,int n){return (int)((random_next(c)>>8)%(unsigned int)n);}
static int campaign_begin_enemy(Campaign*c,int kind,int phase);
static int pending_ship=-1; /* V52/V53: imported hull for the next ENEMY_SHIP encounter. */
#include "event_runtime.inc"
const char *campaign_sector_name(int type){
    static const char *const names[]={"Uncharted","Civilian","Engi","Rebel","Pirate","Last Stand","Crystal"};
    return type>=0&&type<SECTOR_TYPE_COUNT?names[type]:"Unknown";
}
const char *campaign_sector_summary(int type){
    static const char *const descriptions[]={"Legacy encounter mix","Supplies and distress","Repairs and supplies","More hostile patrols","Hostiles and salvage","Final approach","Hidden Crystal homeworld"};
    return type>=0&&type<SECTOR_TYPE_COUNT?descriptions[type]:"Unknown route";
}
int campaign_route_count(const Campaign*c){
    if(c->route_policy!=1||c->sector<0||c->sector>=c->sector_count-1)return 0;
    return c->sector==c->sector_count-2?1:2;
}
static void sector_routes(Campaign*c){
    int count=campaign_route_count(c);
    c->next_sector[0]=c->next_sector[1]=SECTOR_UNCHARTED;
    if(count==1)c->next_sector[0]=SECTOR_FINAL;
    else if(count==2){
        c->next_sector[0]=SECTOR_CIVILIAN+bounded(c,4);
        c->next_sector[1]=SECTOR_CIVILIAN+(c->next_sector[0]-SECTOR_CIVILIAN+1+bounded(c,3))%4;
    }
}
/* Authored weights over the existing event dispatcher, not imported XML events. */
static int sector_node(Campaign*c){
    static const int weights[SECTOR_TYPE_COUNT][7]={
        {1,1,1,1,1,1,1}, {1,4,3,1,2,1,1}, {1,3,1,1,4,2,1},
        {6,1,2,1,1,1,1}, {4,2,3,1,1,1,2}, {5,1,1,1,2,1,1},
        {5,6,0,2,0,2,0} /* V63 Crystal: CRYSTAL_HOME hostile, neutral/items/nothing and store beacons. */
    };
    int type=c->sector_types[c->sector],total=0,roll;
    if(type<0||type>=SECTOR_TYPE_COUNT)type=SECTOR_UNCHARTED;
    for(int i=0;i<7;i++)total+=weights[type][i];
    roll=bounded(c,total);
    for(int i=0;i<7;i++){if(roll<weights[type][i])return NODE_FIGHT+i;roll-=weights[type][i];}
    return NODE_FIGHT;
}
/* V52 original ship pools by sector type (imported_enemies indices):
 * nebula: slug, Mantis, Zoltan, Crystal; civilian: SHIPS_CIVILIAN (slug button, Rock,
 * Federation, Engi) and Zoltan; Engi: Engi, Rock, Zoltan; Rebel and final: the Rebel
 * elite fighter; pirate: the base pirate variants and Mantis. */
static void quests_place_specials(Campaign*c);
static int sector_ship(Campaign*c,int type){
    /* V63: the hidden Crystal sector fields Crystal ships only (CRYSTAL_SCOUT/BOMBER). */
    if(type==SECTOR_SECRET){int crystal[IMPORTED_ENEMY_COUNT],count=0;
        for(int i=0;i<IMPORTED_ENEMY_COUNT;i++)if(!strncmp(imported_enemies[i].blueprint,"CRYSTAL_",8))crystal[count++]=i;
        if(count)return crystal[bounded(c,count)];}
    static const int nebula[]={5,6,7,11,12,13,18,19,20,21},civilian[]={5,8,9,14,15,16,17,18},engi[]={16,17,8,9,10,18,19},
        rebel[]={4},pirate[]={22,23,24,25,26,27,28,29,30,31,32,33,34,35,11,12};
    const int*pool=type==SECTOR_CIVILIAN?civilian:type==SECTOR_ENGI?engi:type==SECTOR_PIRATE?pirate:(type==SECTOR_REBEL||type==SECTOR_FINAL)?rebel:nebula;
    int n=type==SECTOR_CIVILIAN?8:type==SECTOR_ENGI?7:type==SECTOR_PIRATE?16:(type==SECTOR_REBEL||type==SECTOR_FINAL)?1:10;
    return pool[bounded(c,n)];
}
static int sector_enemy(Campaign*c){
    static const int pools[SECTOR_TYPE_COUNT][6]={
        {ENEMY_SCOUT,ENEMY_INTERCEPTOR,ENEMY_ARMORED,ENEMY_ASSAULT,ENEMY_REBEL,ENEMY_REBEL_BOARDER},
        {ENEMY_SCOUT,ENEMY_SCOUT,ENEMY_INTERCEPTOR,ENEMY_INTERCEPTOR,ENEMY_ASSAULT,ENEMY_REBEL},
        {ENEMY_SCOUT,ENEMY_SCOUT,ENEMY_ARMORED,ENEMY_ARMORED,ENEMY_INTERCEPTOR,ENEMY_ASSAULT},
        {ENEMY_REBEL,ENEMY_REBEL,ENEMY_REBEL_BOARDER,ENEMY_REBEL_BOARDER,ENEMY_INTERCEPTOR,ENEMY_ASSAULT},
        {ENEMY_ASSAULT,ENEMY_ASSAULT,ENEMY_INTERCEPTOR,ENEMY_ARMORED,ENEMY_SCOUT,ENEMY_REBEL_BOARDER},
        {ENEMY_REBEL,ENEMY_REBEL_BOARDER,ENEMY_INTERCEPTOR,ENEMY_ARMORED,ENEMY_ASSAULT,ENEMY_REBEL},
        {ENEMY_SCOUT,ENEMY_INTERCEPTOR,ENEMY_ARMORED,ENEMY_ASSAULT,ENEMY_SCOUT,ENEMY_INTERCEPTOR}
    };
    int type=c->route_policy==1?c->sector_types[c->sector]:SECTOR_UNCHARTED;
    if(c->sector==0){
        /* Authored DS progression: keep the guaranteed first contact gentle,
         * then use existing burst-capable profiles. Historical policies/maps
         * retain their old pool and consume no additional random draw. */
        if(c->current==1||c->route_policy!=1||c->event_policy<4)return ENEMY_SCOUT;
        return bounded(c,2)?ENEMY_ARMORED:ENEMY_INTERCEPTOR;
    }
    if(type<0||type>=SECTOR_TYPE_COUNT)type=SECTOR_UNCHARTED;
    /* V52: current runs meet original ships of the sector's peoples most of the time. */
    /* Rebel sectors keep the original SHIPS_REBEL/SHIPS_AUTO hulls (the existing
     * kinds); Elite Fighters belong to the Last Stand lists. */
    if(c->route_policy==1&&type==SECTOR_SECRET){pending_ship=sector_ship(c,type);return ENEMY_SHIP;}
    if(c->route_policy==1&&c->event_policy>=4&&type!=SECTOR_FINAL&&type!=SECTOR_REBEL){
        if(bounded(c,10)<6){pending_ship=sector_ship(c,type);return ENEMY_SHIP;}}
    return pools[type][bounded(c,6)];
}
static int layer(const Campaign *c,int n){return c->node_count==CAM_LEGACY_NODE_COUNT?(n==0?0:n==11?6:(n+1)/2):(n==0?0:n==22?8:(n-1)/3+1);}
static void finale_generate(Campaign*c);
static void repair_hull(Campaign *c,int amount);
static void connect(Campaign *c,int a,int b){c->links[a]|=1<<b;c->links[b]|=1<<a;}
static void sector_generate_legacy(Campaign *c) {
    int i,j;
    memset(c->links,0,sizeof c->links);memset(c->visited,0,sizeof c->visited);
    for(i=0;i<CAM_LEGACY_NODE_COUNT;i++) {
        c->node_kind[i]=NODE_FIGHT+bounded(c,7);
        c->node_y[i]=(i&1?67:123)+bounded(c,15)-7;
        for(j=0;j<CAM_OFFER_COUNT;j++)c->stock[i][j]=j<3?3:1;
        c->stock[i][7]=99;c->stock[i][13]=3;
    }
    c->node_kind[0]=NODE_START;c->node_kind[11]=NODE_EXIT;
    c->node_kind[1]=NODE_FIGHT;c->node_kind[2]=NODE_SUPPLY;
    c->node_kind[5]=NODE_STORE;c->node_kind[9]=NODE_REPAIR;
    c->node_kind[8]=NODE_QUEST;
    c->node_y[0]=c->node_y[11]=94;
    connect(c,0,1);connect(c,0,2);
    for(i=1;i<9;i+=2) {
        connect(c,i,i+2);connect(c,i+1,i+3);
        if(bounded(c,2))connect(c,i,i+3);else connect(c,i+1,i+2);
        if(bounded(c,2))connect(c,i,i+1);
    }
    connect(c,9,11);connect(c,10,11);
    c->current=0;c->selected=1;c->visited[0]=1;c->sector_jumps=0;
    c->event_id=EVENT_NONE;c->event_resolved=1;c->mode=CAM_MAP;c->pursued=0;c->imported_node=-1;c->imported_ship=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;
    snprintf(c->text,sizeof c->text,"Sector %d/%d. Linked routes can be revisited. Fleet advances after 8 jumps. Store at beacon 6; exit at 12.",c->sector+1,c->sector_count);
}
/* Authored DS map: seven three-beacon columns, with guaranteed forward routes. */
static void sector_generate(Campaign *c){
    int i,j,col,row;
    memset(c->links,0,sizeof c->links);memset(c->visited,0,sizeof c->visited);
    memset(c->stock,0,sizeof c->stock);memset(c->node_kind,0,sizeof c->node_kind);
    memset(c->node_y,0,sizeof c->node_y);
    if(c->node_count==CAM_LEGACY_NODE_COUNT){sector_generate_legacy(c);return;}
    for(i=0;i<c->node_count;i++){
        c->node_kind[i]=c->route_policy==1?sector_node(c):NODE_FIGHT+bounded(c,7);
        c->node_y[i]=60+((i-1+3)%3)*35+bounded(c,9)-4;
        for(j=0;j<CAM_OFFER_COUNT;j++)c->stock[i][j]=j<3?3:1;
        c->stock[i][7]=99;c->stock[i][13]=3;
    }
    c->node_kind[0]=NODE_START;c->node_kind[22]=NODE_EXIT;
    c->node_y[0]=c->node_y[22]=95;
    c->node_kind[1]=NODE_FIGHT;c->node_kind[2]=NODE_SUPPLY;
    c->node_kind[8]=NODE_STORE;c->node_kind[17]=NODE_STORE;
    c->node_kind[20]=NODE_REPAIR;c->node_kind[14]=NODE_QUEST;
    for(row=0;row<3;row++){connect(c,0,1+row);connect(c,19+row,22);}
    for(col=0;col<6;col++){
        int base=1+col*3;
        for(row=0;row<3;row++)connect(c,base+row,base+row+3);
        row=bounded(c,2);connect(c,base+row,base+row+4);
        row=bounded(c,2);connect(c,base+row+1,base+row+3);
        if(bounded(c,2)){row=bounded(c,2);connect(c,base+row,base+row+1);}
    }
    c->current=0;c->selected=1;c->visited[0]=1;c->sector_jumps=0;
    c->event_id=EVENT_NONE;c->event_resolved=1;c->mode=CAM_MAP;c->pursued=0;c->imported_node=-1;c->imported_ship=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;
    if(c->route_policy==1)sector_routes(c);
    if(c->finale_policy==1&&c->route_policy==1&&c->sector==c->sector_count-1){
        finale_generate(c);
        snprintf(c->text,sizeof c->text,"Last Stand. The rebel flagship is advancing on the Federation base at beacon 23. Intercept it; if it holds the base for 3 jumps, the war is lost.");return;
    }
    snprintf(c->text,sizeof c->text,"Sector %d/%d. Explore 23 linked beacons. Fleet advances after 8 jumps. Stores in columns 3 and 6; exit at 23.",c->sector+1,c->sector_count);
}
int campaign_linked(const Campaign *c,int a,int b){return a>=0&&a<c->node_count&&b>=0&&b<c->node_count&&(c->links[a]&(1<<b))!=0;}
/* V41 original final sector. Reconstructed from FTL 1.6.14 StarMap::UpdateBoss
 * (0x5f4ed0), ForceBossJump (0x5eed40), ReverseBossPath (0x5eece0) and the
 * base-loss test at 0x5eee40; see docs/V41_FINAL_SECTOR.md. */
int campaign_finale_active(const Campaign*c){return c&&c->finale_policy==1&&c->route_policy==1&&c->sector==c->sector_count-1&&c->boss_path_len>=2&&c->boss_path_len<=CAM_BOSS_PATH_MAX;}
int campaign_base_node(const Campaign*c){return campaign_finale_active(c)?c->node_count-1:-1;}
int campaign_boss_node(const Campaign*c){return campaign_finale_active(c)&&c->boss_loc>=0&&c->boss_loc<c->boss_path_len?c->boss_path[c->boss_loc]:-1;}
int campaign_boss_moves_next(const Campaign*c){return campaign_finale_active(c)&&c->boss_move;}
int campaign_base_turns_left(const Campaign*c){return campaign_finale_active(c)&&c->base_arrived>0?CAM_BASE_TURNS-c->base_arrived:-1;}
int campaign_node_rebel(const Campaign*c,int n){return campaign_finale_active(c)&&n>=0&&n<c->node_count&&((c->fleet_nodes>>n)&1u);}
static int boss_at_base(const Campaign*c){return c->boss_path[c->boss_loc]==c->node_count-1;}
static void boss_reverse(Campaign*c){
    for(int i=0,j=c->boss_path_len-1;i<j;i++,j--){int t=c->boss_path[i];c->boss_path[i]=c->boss_path[j];c->boss_path[j]=t;}
    c->boss_loc=c->boss_path_len-1-c->boss_loc;c->boss_reversed^=1;
}
/* After a defeated phase the flagship moves on the next jump; beaten at the
 * base, it first turns its route around and the base countdown restarts. */
static void boss_force_jump(Campaign*c){
    c->boss_move=1;
    if(!c->boss_reversed&&c->boss_loc==c->boss_path_len-1){boss_reverse(c);c->base_arrived=0;}
}
static void fleet_take(Campaign*c,int n){if(n>=0&&n<c->node_count-1)c->fleet_nodes|=1u<<n;c->fleet_pending&=~(1u<<n);}
static void boss_advance(Campaign*c){
    c->boss_loc++;
    if(!boss_at_base(c))fleet_take(c,c->boss_path[c->boss_loc]);
    if(c->boss_loc==c->boss_path_len-1)c->base_arrived=1;
}
/* One call per player jump or wait. The flagship moves on alternate calls; at
 * the base the countdown advances each call, capped at CAM_BASE_TURNS. */
static void boss_update(Campaign*c){
    int toggle=1;
    if(!boss_at_base(c)){
        if(c->boss_loc==c->boss_path_len-1)boss_reverse(c);
        c->base_arrived=0;
        if(c->boss_move)boss_advance(c);
    }else if(c->boss_move){
        if(!c->base_arrived)boss_advance(c);
        else {c->boss_move=0;toggle=0;}
    }else c->base_arrived=c->base_arrived?(c->base_arrived<CAM_BASE_TURNS?c->base_arrived+1:CAM_BASE_TURNS):0;
    if(toggle)c->boss_move=!c->boss_move;
    /* A retreat lasts one beacon: once the flagship has moved, it turns back. */
    if(!c->boss_move&&c->boss_reversed)boss_reverse(c);
    if(c->boss_loc==c->boss_path_len-1)c->boss_move=0;
    /* Rebel control spreads: last jump's marked beacons fall, then one new
     * beacon is marked, two once the flagship is past its second beacon. */
    {int cand[CAM_NODE_COUNT],n=0;
     for(int i=0;i<c->node_count;i++){
        if((c->fleet_pending>>i)&1u)fleet_take(c,i);
        else if(i!=c->node_count-1&&!((c->fleet_nodes>>i)&1u))cand[n++]=i;
     }
     if(n){c->fleet_pending|=1u<<cand[bounded(c,n)];
        if(n>1&&(c->boss_loc>1||c->boss_reversed))c->fleet_pending|=1u<<cand[bounded(c,n)];}
    }
}
/* Last Stand node mix from sector_data.xml FINAL: 1 store, 3 repair stations,
 * 6 hostile beacons; the remaining neutral beacons approximate BOSS_NEUTRAL. */
static void finale_generate(Campaign*c){
    int pool[CAM_NODE_COUNT],n=0,base=c->node_count-1,dist[CAM_NODE_COUNT],queue[CAM_NODE_COUNT],head=0,tail=0,cand[CAM_NODE_COUNT],count=0;
    pool[n++]=NODE_STORE;for(int i=0;i<3;i++)pool[n++]=NODE_REPAIR;for(int i=0;i<6;i++)pool[n++]=NODE_FIGHT;
    while(n<base-1){int r=bounded(c,5);pool[n++]=r<2?NODE_SUPPLY:r<4?NODE_DISTRESS:NODE_FIGHT;}
    for(int i=n-1;i>0;i--){int j=bounded(c,i+1),t=pool[i];pool[i]=pool[j];pool[j]=t;}
    for(int i=1;i<base;i++)c->node_kind[i]=pool[i-1];
    c->node_kind[0]=NODE_START;c->node_kind[base]=NODE_EXIT;
    for(int i=0;i<c->node_count;i++)dist[i]=-1;
    dist[base]=0;queue[tail++]=base;
    while(head<tail){int v=queue[head++];for(int u=0;u<c->node_count;u++)if(dist[u]<0&&campaign_linked(c,v,u)){dist[u]=dist[v]+1;queue[tail++]=u;}}
    /* Generated routes hold 4..6 beacons (UpdateBoss path size check). */
    for(int i=1;i<base;i++)if(dist[i]>=3&&dist[i]<=CAM_BOSS_PATH_MAX-1)cand[count++]=i;
    c->fleet_nodes=c->fleet_pending=0;c->boss_loc=c->boss_move=c->boss_reversed=c->base_arrived=c->base_greeted=0;c->boss_path_len=0;memset(c->boss_path,0,sizeof c->boss_path);
    if(!count)return;
    {int at=cand[bounded(c,count)];
     c->boss_path[c->boss_path_len++]=at;
     while(at!=base&&c->boss_path_len<CAM_BOSS_PATH_MAX){int next[CAM_NODE_COUNT],k=0;
        for(int u=0;u<c->node_count;u++)if(dist[u]==dist[at]-1&&campaign_linked(c,at,u))next[k++]=u;
        at=next[bounded(c,k)];c->boss_path[c->boss_path_len++]=at;}
    }
    fleet_take(c,c->boss_path[0]);
}
int campaign_node_x(const Campaign *c,int n){return n>=0&&n<c->node_count?16+layer(c,n)*222/(c->node_count==CAM_LEGACY_NODE_COUNT?6:8):-1;}
int campaign_node_y(const Campaign *c,int n){return n>=0&&n<c->node_count?c->node_y[n]:-1;}
const char *campaign_node_name(const Campaign *c,int n){
    static const char *const names[]={"Departure","Hostile ship","Supplies","Distress","Store","Repair dock","Courier","Solar flare","Exit"};
    if(n<0||n>=c->node_count||c->node_kind[n]<0||c->node_kind[n]>NODE_EXIT)return "Unknown";
    if(campaign_finale_active(c)&&c->node_kind[n]==NODE_EXIT)return "Federation base";
    if(campaign_finale_active(c)&&c->node_kind[n]==NODE_REPAIR)return "Repair station";
    return names[c->node_kind[n]];
}
/* V55 Adv. FTL Navigation also allows a jump to any explored beacon of the sector. */
static int travel_destination(const Campaign*c,int n){return c&&c->node_count>0&&c->node_count<=CAM_NODE_COUNT&&n>=0&&n<c->node_count&&n!=c->current&&c->fuel>0&&c->jumps<1000000&&c->sector_jumps<1000000&&(campaign_linked(c,c->current,n)||(game_augment_count(&c->game,AUG_FTL_JUMPER)&&c->visited[n]));}
int campaign_can_jump(const Campaign *c,int n){return c&&c->reward.kind==REWARD_NONE&&c->mode==CAM_MAP&&c->game.outcome!=GAME_DEFEAT&&!game_away_crew(&c->game)&&travel_destination(c,n);}
int campaign_can_retreat(const Campaign*c,int n){return c&&c->reward.kind==REWARD_NONE&&c->mode==CAM_COMBAT&&travel_destination(c,n)&&game_can_depart(&c->game,1);}
/* Single bounded staging object makes departure and arrival one transaction. */
static Campaign travel_work;
static void init_seed(Campaign *c,unsigned int seed,int legacy,int layout){
    int i;memset(c,0,sizeof(*c));if(!game_init_ship(&c->game,layout))game_init(&c->game);
    c->node_count=legacy?CAM_LEGACY_NODE_COUNT:CAM_GENERATED_NODE_COUNT;c->sector_count=legacy?CAM_LEGACY_SECTOR_COUNT:CAM_SECTOR_COUNT;
    c->event_policy=legacy?0:6;c->imported_node=-1;c->imported_ship=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;
    c->route_policy=legacy?0:1;if(!legacy)c->sector_types[0]=SECTOR_CIVILIAN;
    c->seed=seed?seed:1;c->rng=c->seed;c->fuel=12;c->scrap=20;c->boss_phase=-1;c->boss_resume_phase=-1;c->finale_policy=legacy?0:1;
    c->game.rng=c->seed;for(i=0;i<CAM_CARGO_COUNT;i++){c->cargo[i]=-1;c->drone_cargo[i]=-1;}
    for(i=0;i<CAM_QUESTS;i++){c->quest_event[i]=c->quest_node[i]=-1;}
    sector_generate(c);
}
void campaign_init_seed(Campaign *c,unsigned int seed){init_seed(c,seed,0,0);}
/* V57: a new run on any playable layout (player_ships.h index). */
int campaign_init_ship(Campaign *c,unsigned int seed,int layout){if(layout<0||layout>=game_layout_count()||!game_layout_supported(layout))return 0;init_seed(c,seed,0,layout);return 1;}
void campaign_init_legacy_seed(Campaign *c,unsigned int seed){init_seed(c,seed,1,0);}
void campaign_init_legacy_layout(Campaign *c,int layout){init_seed(c,0x46544c44u,1,layout);}
void campaign_init_legacy(Campaign *c){campaign_init_legacy_seed(c,0x46544c44u);}
void campaign_init(Campaign *c){campaign_init_seed(c,0x46544c44u);}
static void resolved(Campaign *c,const char *text){c->mode=CAM_EVENT;c->event_resolved=1;c->event_id=EVENT_AFTERMATH;snprintf(c->text,sizeof c->text,"%s",text);}
/* Bounded DS equipment mixes from supported source-list components. The
 * component data is imported; pairings/provisioning are authored and audited.
 * Stage the complete arrival to avoid partial mutation on a rejected loadout. */
/* V51: from sector 3 on, enemy weapons come from the original base-game list.
 * Allowed rarity grows with the sector (rarer, stronger weapons later); the
 * loadout must fit the ship's weapon system. Support bombs (heal, lockdown) and
 * crystal weapons are never chosen. Deterministic per run, sector and beacon. */
static int enemy_weapon_ok(int id,int sector){
    if(id==0||id==31||id==32||id>=33||id<0||id>=GAME_CATALOG_COUNT)return 0;
    return imported_weapon_catalog[id].rarity<=1+sector/3;
}
static void enemy_weapon_pick(const Campaign*c,int*ids,int count,int maximum){
    unsigned h=(c->seed*2654435761u)^((unsigned)c->sector*40503u)^((unsigned)c->current*69069u);
    for(int attempt=0;attempt<24;attempt++){int trial[2],power=0,ok=1;
        for(int i=0;i<count;i++){int pool=0;for(int id=0;id<GAME_CATALOG_COUNT;id++)pool+=enemy_weapon_ok(id,c->sector);
            h=h*1664525u+1013904223u;int roll=pool?(int)((h>>8)%(unsigned)pool):0;trial[i]=2;
            for(int id=0;id<GAME_CATALOG_COUNT;id++)if(enemy_weapon_ok(id,c->sector)&&roll--==0){trial[i]=id;break;}
            power+=trial[i]==GAME_ENEMY_ARTEMIS?imported_enemy_artemis.power:imported_weapon_catalog[trial[i]].power;}
        if(power>maximum)ok=0;
        if(ok){for(int i=0;i<count;i++)ids[i]=trial[i];return;}
    }
}
static Game encounter_work;
static int campaign_begin_enemy(Campaign*c,int kind,int phase){
 Game*g=&encounter_work;*g=c->game;g->interim_flagship=c->finale_policy==0;g->flagship_authentic=c->flagship_original;
 g->enemy_ship=kind==ENEMY_SHIP?pending_ship:-1;
 if(!game_begin_encounter(g,kind,c->sector,phase))return 0;
 if(c->event_policy>=4&&c->route_policy==1&&c->sector>=1&&kind!=ENEMY_FLAGSHIP){
  int ids[2]={2,3},count=0;
  if(kind==ENEMY_SCOUT||kind==ENEMY_REBEL){count=2;if((c->seed+(unsigned)c->current+(unsigned)c->sector)&1u)ids[1]=GAME_ENEMY_ARTEMIS;}
  else if(kind==ENEMY_REBEL_BOARDER)count=2;
  else if(kind==ENEMY_ASSAULT||kind==ENEMY_REBEL_FAT)count=1;
  if(count){
   int imported=game_enemy_import_index(g),room=-1,maximum=0,needed=0;
   if(imported<0||imported>=IMPORTED_ENEMY_COUNT)return 0;
   const ImportedEnemy*ship=&imported_enemies[imported];
   for(int i=0;i<ship->system_count;i++)if(!strcmp(ship->systems[i].name,"weapons")){room=ship->systems[i].room;maximum=ship->systems[i].maximum;break;}
   if(c->sector>=2&&maximum>0){int budget=g->enemy.reactor-g->enemy_drone.power-g->teleporter[1].power;
    for(int i=0;i<GAME_SYSTEM_COUNT;i++)if(i!=SYS_WEAPONS&&i!=SYS_ENGINES)budget-=g->enemy.systems[i].power;
    enemy_weapon_pick(c,ids,count,budget<maximum?budget:maximum);}
   for(int i=0;i<count;i++)needed+=ids[i]==GAME_ENEMY_ARTEMIS?imported_enemy_artemis.power:imported_weapon_catalog[ids[i]].power;
   if(room<0||needed>maximum)return 0;
   int index=-1;for(int i=0;i<g->enemy.room_count;i++)if(g->enemy.rooms[i].id==room)index=i;
   if(index<0||(g->enemy.rooms[index].system>=0&&g->enemy.rooms[index].system!=SYS_WEAPONS))return 0;
   GameSystemState*w=&g->enemy.systems[SYS_WEAPONS];w->room=room;w->capacity=w->health=w->power=needed;g->enemy.rooms[index].system=SYS_WEAPONS;
   int total=g->enemy_drone.power+g->teleporter[1].power;
   for(int i=0;i<GAME_SYSTEM_COUNT;i++)total+=g->enemy.systems[i].power;
   if(total>g->enemy.reactor){int take=total-g->enemy.reactor;if(take>g->enemy.systems[SYS_ENGINES].power)return 0;g->enemy.systems[SYS_ENGINES].power-=take;}
   if(!game_enemy_set_loadout(g,ids,count,ship->missiles))return 0;
  }
 }
 game_set_environment(g,c->beacon_env);game_enforce_status(g,1);
 if(c->pending_boarders>0){game_spawn_intruders(g,c->pending_boarders,c->pending_boarder_species);c->pending_boarders=0;}
 c->game=*g;return 1;
}
static void flagship_capture_crew(Campaign*c){
    if(!game_is_flagship(&c->game)||c->game.enemy.hull<=0)return;
    game_flagship_crew_counts(&c->game,c->boss_crew);c->boss_crew_valid=1;
}
static int start_combat(Campaign *c,int kind){
    if(!campaign_begin_enemy(c,kind,c->boss_phase<0?0:c->boss_phase))return 0;
    /* StartStage(1) always reseeds eleven crew; later stages keep survivors. */
    if(kind==ENEMY_FLAGSHIP&&c->boss_phase>0&&c->boss_crew_valid)game_flagship_set_crew(&c->game,c->boss_crew);
    c->mode=CAM_COMBAT;c->event_resolved=1;c->result_ticks=0;
    snprintf(c->text,sizeof c->text,c->boss_phase>=0?"Flagship phase %d/3. Defeat this weapon platform to continue.":"Hostile contact in sector %d. Destroy its weapons and recover salvage.",c->boss_phase>=0?c->boss_phase+1:c->sector+1);
    return 1;
}
/* Fleet motion is an authored approximation. Forecasts do not mutate the run. */
int campaign_fleet_x(const Campaign*c,int extra){
    int jumps=c->sector_jumps;
    if(extra>0)jumps+=extra>1000000?1000000:extra;
    if(jumps<=8)return 0;
    if(c->node_count==CAM_LEGACY_NODE_COUNT)return jumps>=14?255:16+(jumps-8)*37;
    return jumps>=25?255:16+(jumps-8)*14;
}
int campaign_node_captured(const Campaign*c,int n,int extra){
    if(n<0||n>=c->node_count||campaign_finale_active(c))return 0;
    if(extra<0)extra=0;
    if(extra>1000000)extra=1000000;
    if(c->sector_jumps+extra<=8)return 0;
    if(c->node_count==CAM_LEGACY_NODE_COUNT)return layer(c,n)<=c->sector_jumps+extra-8;
    return campaign_node_x(c,n)<=campaign_fleet_x(c,extra);
}
static int patrol(Campaign*c){
    if(!start_combat(c,ENEMY_INTERCEPTOR))return 0;
    c->pursued=1;
    snprintf(c->text,sizeof c->text,"The rebel fleet has caught this beacon. Defeat the patrol; only emergency fuel can be recovered.");
    return 1;
}
/* V41: the flagship moves first; meeting it starts the current phase. A base
 * held for CAM_BASE_TURNS while the player is elsewhere loses the campaign. */
static int flagship_combat(Campaign*c){
    int phase=c->boss_resume_phase<0?0:c->boss_resume_phase;
    c->boss_phase=phase;
    if(!start_combat(c,ENEMY_FLAGSHIP)){c->boss_phase=-1;return 0;}
    c->boss_resume_phase=phase;
    snprintf(c->text,sizeof c->text,"The rebel flagship engages. Phase %d/3: disable it before it reaches the Federation base.",phase+1);
    return 1;
}
static int base_lost(Campaign*c){
    c->mode=CAM_DEFEAT;c->event_resolved=1;
    snprintf(c->text,sizeof c->text,"The rebel flagship destroyed the Federation base after %d jumps. The war is lost. Start a new run to try again.",c->jumps);
    return 1;
}
/* Returns -1 when the ordinary destination handling should continue. */
static int finale_arrival(Campaign*c,int n,int seen){
    if(!campaign_finale_active(c))return -1;
    boss_update(c);
    if(campaign_boss_node(c)==n)return flagship_combat(c);
    if(c->base_arrived>=CAM_BASE_TURNS)return base_lost(c);
    if(n!=c->node_count-1&&campaign_node_rebel(c,n)){
        /* BOSS_FLEETS_REBEL: a hostile Rebel ship; LOW scrap_only reward is
         * approximated by the emergency-fuel patrol payout. */
        if(!start_combat(c,ENEMY_REBEL))return 0;
        c->pursued=1;snprintf(c->text,sizeof c->text,"Rebel forces hold this beacon. Defeat their ship.");return 1;
    }
    if(c->node_kind[n]==NODE_EXIT){
        if(!c->base_greeted){c->base_greeted=1;repair_hull(c,10);c->fuel+=10;
            resolved(c,"Federation base. The admiral welcomes you: the base repairs 10 hull and supplies 10 fuel. The flagship is coming; stop it before it reaches this beacon.");}
        else resolved(c,"Federation base. Defenses are holding for now. The flagship must not reach this beacon.");
        return 1;
    }
    if(c->node_kind[n]==NODE_REPAIR&&!seen){
        /* BOSS_REPAIR_STATION: -15 damage, full repair, missiles 4, fuel 5, drones 5, scrap 22..44. */
        int scrap=22+bounded(c,23);repair_hull(c,15);
        for(int i=0;i<GAME_SYSTEM_COUNT;i++)c->game.player.systems[i].health=c->game.player.systems[i].capacity;
        c->game.missiles+=4;c->fuel+=5;c->game.drone_parts+=5;c->scrap+=scrap;
        snprintf(c->text,sizeof c->text,"A Federation repair station patches 15 hull, repairs every system and supplies 4 missiles, 5 fuel, 5 drone parts and %d scrap.",scrap);
        c->mode=CAM_EVENT;c->event_resolved=1;c->event_id=EVENT_AFTERMATH;
        return 1;
    }
    return -1;
}
int campaign_wait(Campaign*c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    if(c->mode!=CAM_MAP||game_away_crew(&c->game)||c->sector_jumps>=1000000)return 0;
    c->sector_jumps++;
    if(campaign_finale_active(c)){
        boss_update(c);
        if(campaign_boss_node(c)==c->current)return flagship_combat(c);
        if(c->base_arrived>=CAM_BASE_TURNS)return base_lost(c);
        snprintf(c->text,sizeof c->text,"You wait at this beacon. %s",c->boss_move?"The flagship will jump next turn.":"The flagship holds position.");
    }
    else if(campaign_node_captured(c,c->current,0))patrol(c);
    else snprintf(c->text,sizeof c->text,"You wait at this beacon. The fleet advanced one jump; no supplies recovered.");
    if(c->mode==CAM_MAP&&c->fuel==0&&c->event_policy>=1){
        for(int i=0;i<IMPORTED_EVENT_ROOT_COUNT;i++)if(imported_event_roots[i].fuel_only){campaign_imported_begin(c,i);break;}
    }
    return 1;
}
static int arrive(Campaign*c,int n){
    int seen;
    /* Event ownership ends at departure, before destination interpretation. */
    c->imported_ship=c->imported_node=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;c->event_id=EVENT_NONE;c->event_resolved=1;c->boss_phase=-1;c->result_ticks=0;
    drones_reset(&c->game); /* Successful travel retires paid encounter deployments. */
    seen=c->visited[n];c->fuel--;c->jumps++;c->sector_jumps++;c->current=c->selected=n;c->visited[n]=1;
    /* V59: hazard beacons (Stealth) and nebula beacons (Slug: 30 before sector 8). */
    if(c->node_kind[n]==NODE_HAZARD)c->ach_flags|=ACH_FLAG_HAZARD;
    c->beacon_env=ENV_NONE;game_clear_status(&c->game);c->pending_boarders=0;c->store_after_event=0;c->secret_jump=0;
    if(c->route_policy==1&&c->sector_types[c->sector]==SECTOR_UNCHARTED&&++c->ach_counter[ACH_RUN_NEBULA]>=30&&c->sector<7)achievements_earn(&c->game,ACH_SLUG_NEBULA);
    c->pursued=campaign_node_captured(c,n,0);
    if(c->pursued)return patrol(c);
    {int finale=finale_arrival(c,n,seen);if(finale>=0)return finale;}
    if(c->node_kind[n]==NODE_STORE){c->mode=CAM_STORE;snprintf(c->text,sizeof c->text,"Buy supplies, crew and weapons. Weapons enter cargo; install them from ship management. Stock persists on revisits.");return 1;}
    if(quest_arrive(c,n))return 1;
    if(seen&&c->node_kind[n]!=NODE_EXIT){resolved(c,"This beacon has already been searched. No new salvage. The fleet keeps advancing.");return 1;}
    if(c->event_policy>=1&&((c->node_kind[n]==NODE_SUPPLY&&n!=2)||(c->node_kind[n]==NODE_REPAIR&&n!=20)||(c->node_kind[n]==NODE_FIGHT&&n!=1&&c->sector>=1)||(c->event_policy==6&&(c->node_kind[n]==NODE_DISTRESS||c->node_kind[n]==NODE_HAZARD)))&&imported_pool(c,c->node_kind[n]))return 1;
    if(c->node_kind[n]==NODE_FIGHT)return start_combat(c,sector_enemy(c));
    c->mode=CAM_EVENT;c->event_resolved=0;
    switch(c->node_kind[n]) {
    case NODE_SUPPLY:c->event_id=EVENT_SUPPLY;snprintf(c->text,sizeof c->text,"A salvage tender offers supplies. Take fuel, or trade the cache for scrap and ammunition.");break;
    case NODE_DISTRESS:c->event_id=EVENT_DISTRESS;snprintf(c->text,sizeof c->text,"A civilian transport needs fuel. Deliver 2 fuel for 24 scrap, salvage its abandoned cargo, or escort it through a hostile patrol.");break;
    case NODE_REPAIR:c->event_id=EVENT_REPAIR;snprintf(c->text,sizeof c->text,"A Federation repair dock offers a free 6 hull patch or a full hull repair for 18 scrap.");break;
    case NODE_QUEST:c->event_id=EVENT_QUEST;snprintf(c->text,sizeof c->text,"%s",c->quest==0?"A courier asks you to carry navigation codes to another relay. Accept the delivery or take emergency supplies.":c->quest==1?"A courier relay can receive your codes. Deliver for 45 scrap and 4 fuel, or keep the codes for a later relay.":"The courier mission is complete. The relay offers emergency supplies.");break;
    case NODE_HAZARD:c->event_id=EVENT_HAZARD;snprintf(c->text,sizeof c->text,"A solar flare threatens a derelict. A quick salvage costs 3 hull for 25 scrap; a shielded approach needs 4 installed shield bars. You may leave.");break;
    case NODE_EXIT:c->event_id=EVENT_EXIT;snprintf(c->text,sizeof c->text,"%s",c->sector==c->sector_count-1?(c->boss_resume_phase>=0?"The flagship awaits your return. Retry the same phase at full enemy hull; no additional tender supplies are available.":"The rebel flagship blocks the final exit. A Federation tender restores 10 hull and supplies 6 missiles before the three-phase battle."):"Sector exit reached. The next sector has new routes and stronger enemies. A Federation cache supplies 3 fuel and 12 scrap.");break;
    default:resolved(c,"A quiet beacon. No new contacts.");break;
    }
    return 1;
}
int campaign_jump(Campaign*c,int n){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    if(!campaign_can_jump(c,n)){if(c&&c->mode==CAM_MAP)snprintf(c->text,sizeof c->text,"%s",c->fuel<=0?"No fuel. Request rescue from ship management.":"Select a linked beacon.");return 0;}
    travel_work=*c;
    if(!arrive(&travel_work,n))return 0;
    *c=travel_work;return 1;
}
int campaign_retreat(Campaign*c,int n,int abandon_away){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    if((abandon_away!=0&&abandon_away!=1)||!campaign_can_retreat(c,n)||!game_can_depart(&c->game,abandon_away))return 0;
    travel_work=*c;flagship_capture_crew(&travel_work);
    if(!game_depart(&travel_work.game,abandon_away)||!arrive(&travel_work,n))return 0;
    *c=travel_work;return 1;
}
int campaign_choice_count(const Campaign *c){
    if(c->mode!=CAM_EVENT||c->event_resolved)return 0;
    if(c->event_id==EVENT_IMPORTED)return imported_active(c)?((c->imported_battle==1||c->imported_battle==4)?1:imported_event_nodes[c->imported_node].choice_count):0;
    if(c->event_id==EVENT_DISTRESS||c->event_id==EVENT_HAZARD)return 3;
    if(c->event_id==EVENT_EXIT){int routes=campaign_route_count(c);return routes?routes:1;}
    return 2;
}
const char *campaign_choice_name(const Campaign *c,int n){
    if(c->event_id==EVENT_IMPORTED){if(imported_active(c)&&!c->event_resolved&&(c->imported_battle==1||c->imported_battle==4))return n==0?(c->imported_battle==1?"Engage":"Continue"):"";const ImportedEventChoice*q=imported_choice(c,n);return q?q->text:"";}
    static const char *const labels[][3]={
        {"Continue",0,0},{"Take 4 fuel","Take 16 scrap + 3 missiles",0},
        {"Deliver 2 fuel: +24 scrap","Salvage: +10 scrap","Escort: fight for reward"},
        {"Free patch: +6 hull","Full repair: 18 scrap",0},
        {"Accept / deliver codes","Take 2 fuel instead",0},
        {"Risk 3 hull: +25 scrap","Shields 4: +25 scrap","Leave safely"},
        {"Continue to next stage",0,0},{"Continue",0,0}};
    if(n<0||n>=campaign_choice_count(c)||c->event_id<0||c->event_id>EVENT_AFTERMATH)return "";
    if(c->event_id==EVENT_EXIT&&campaign_route_count(c))return campaign_sector_name(c->next_sector[n]);
    return labels[c->event_id][n];
}
int campaign_choice_enabled(const Campaign *c,int n){
    if(c->reward.kind!=REWARD_NONE)return 0;
    if(n<0||n>=campaign_choice_count(c))return 0;
    if(c->event_id==EVENT_IMPORTED)return imported_enabled(c,n);
    if(c->event_id==EVENT_EXIT){
        if(game_away_crew(&c->game))return 0;
        if(campaign_route_count(c)){int type=c->next_sector[n];return campaign_route_count(c)==1?type==SECTOR_FINAL:type>=SECTOR_CIVILIAN&&type<=SECTOR_PIRATE;}
    }
    if(c->event_id==EVENT_DISTRESS&&n==0)return c->fuel>=2;
    if(c->event_id==EVENT_REPAIR&&n==1)return c->scrap>=18;
    if(c->event_id==EVENT_HAZARD&&n==0)return c->game.player.hull>3;
    if(c->event_id==EVENT_HAZARD&&n==1)return c->game.player.systems[SYS_SHIELDS].capacity>=4;
    return 1;
}
static void repair_hull(Campaign *c,int amount){c->game.player.hull+=amount;if(c->game.player.hull>c->game.player.max_hull)c->game.player.hull=c->game.player.max_hull;}
/* Original unlock: reaching sector 5 in a Kestrel (A or B) unlocks the Engi Cruiser (The Torus).
 * V71: checked from sector 5 on, and when a saved run is continued, so runs that crossed sector 5
 * under an older build still earn it; the profile ignores ships already unlocked. */
void campaign_check_unlocks(Campaign*c){if(c&&c->sector>=4&&c->game.player_ship/2==0){c->unlock_pending|=1u<<6;}}
/* Enter the next sector (type is ignored by pre-route runs); cache adds the exit's Federation supplies. */
static void sector_advance(Campaign*c,int type,int cache){
    drones_reset(&c->game);c->sector++;
    campaign_check_unlocks(c);
    /* V59: Federation/Zoltan sector-5 and Stealth sector-8 achievements. */
    if(c->sector==4&&!(c->ach_flags&ACH_FLAG_WEAPONS)){achievements_earn(&c->game,ACH_FED_UPGRADE);}
    if(c->sector==4&&!(c->ach_flags&ACH_FLAG_REACTOR)){achievements_earn(&c->game,ACH_ZOLTAN_MANPOWER);}
    if(c->sector==7&&!(c->ach_flags&ACH_FLAG_HAZARD)){achievements_earn(&c->game,ACH_STEALTH_TACTICAL);}
    if(c->route_policy==1){c->sector_types[c->sector]=type;}
    if(cache&&!c->pursued){c->fuel+=3;c->scrap+=12;}
    sector_generate(c);quests_new_sector(c);
}
/* V63 <secretSector/>: like the original, the hidden Crystal homeworld replaces the
 * next sector at once (no fuel, no exit cache); its start beacon (START_BEACON_CRYSTAL)
 * marks the CRYSTAL_UNLOCK quest, and its exit leads on to the following sector. */
static int secret_sector_enter(Campaign*c){
    c->secret_jump=0;
    if(!secret_sector_open(c))return 0;
    sector_advance(c,SECTOR_SECRET,0);
    achievements_earn(&c->game,ACH_ROCK_CRYSTAL); /* Ancestry: only counts in the Rock cruiser. */
    for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++){const ImportedEventPool*e=&imported_event_pools[i];
        if(e->sector==SECTOR_SECRET&&e->category==8&&imported_begin(c,e->root,1))return 1;}
    snprintf(c->text,sizeof c->text,"Sector %d/%d: the hidden Crystal homeworld. Explore 23 linked beacons; exit at 23.",c->sector+1,c->sector_count);
    return 1;
}
int campaign_choice(Campaign *c,int n){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    int id=c->event_id;if(!campaign_choice_enabled(c,n))return 0;
    if(id==EVENT_IMPORTED){if(c->imported_battle==4){c->imported_battle=2;c->mode=CAM_COMBAT;c->event_resolved=1;return 1;}if(c->imported_battle==1)return imported_engage(c);const ImportedEventChoice*q=imported_choice(c,n);int ok=q?imported_enter(c,q->next):0;
        /* V59 Federation: four crew (species) blue options by sector 5. */
        if(ok&&q->req_kind==IE_REQ_SPECIES&&++c->ach_counter[ACH_RUN_CREW_CHOICES]>=4&&c->sector<=4)achievements_earn(&c->game,ACH_FED_DIPLOMACY);
        return ok;}
    if(id==EVENT_EXIT){
        if(game_away_crew(&c->game))return 0;
        if(c->sector<c->sector_count-1){sector_advance(c,c->next_sector[n],1);}
        else {
            int phase=c->boss_resume_phase<0?0:c->boss_resume_phase;
            int old_phase=c->boss_phase;c->boss_phase=phase;
            if(!start_combat(c,ENEMY_FLAGSHIP)){c->boss_phase=old_phase;return 0;}
            if(c->boss_resume_phase<0&&!c->pursued){repair_hull(c,10);c->game.missiles+=6;}
            c->boss_resume_phase=phase;
        }return 1;
    }
    if(id==EVENT_SUPPLY){if(n==0)c->fuel+=4;else{c->scrap+=16;c->game.missiles+=3;}resolved(c,n==0?"Recovered 4 fuel.":"Recovered 16 scrap and 3 missiles.");}
    else if(id==EVENT_DISTRESS){if(n==0){c->fuel-=2;c->scrap+=24;resolved(c,"Delivered 2 fuel. The grateful crew pays 24 scrap.");}else if(n==1){c->scrap+=10;resolved(c,"Recovered 10 scrap from abandoned cargo.");}else{return start_combat(c,ENEMY_SCOUT);}}
    else if(id==EVENT_REPAIR){if(n==1){c->scrap-=18;repair_hull(c,30);}else repair_hull(c,6);resolved(c,"Hull repairs complete. Crew and system repairs remain available in ship management.");}
    else if(id==EVENT_QUEST){if(n==1||c->quest==2){c->fuel+=2;resolved(c,"The relay gives you 2 fuel.");}else if(c->quest==0){c->quest=1;resolved(c,"Navigation codes aboard. Find a courier relay at another beacon for payment.");}else {c->quest=2;c->scrap+=45;c->fuel+=4;resolved(c,"Codes delivered. Received 45 scrap and 4 fuel.");}}
    else if(id==EVENT_HAZARD){if(n<2){c->scrap+=25;if(n==0)c->game.player.hull-=3;}resolved(c,n==2?"You leave before the flare hits.":n==0?"Salvaged 25 scrap. Solar damage: 3 hull.":"Your shields protect the salvage approach. Recovered 25 scrap.");}
    return 1;
}
void campaign_continue(Campaign *c){
    if(c&&c->reward.kind!=REWARD_NONE)return;
    if(campaign_surrender_pending(c))return;
    int i;if(game_away_crew(&c->game)){snprintf(c->text,sizeof c->text,"Crew remain aboard the enemy. Open SHIP then BOARD to recall them or explicitly abandon them.");return;}
    if(c->mode==CAM_EVENT&&!c->event_resolved){if(c->event_id==EVENT_EXIT&&campaign_choice_count(c)==1)campaign_choice(c,0);return;}
    if(c->mode!=CAM_EVENT&&c->mode!=CAM_STORE)return;
    /* V62: an event that opened a store at this beacon continues into it. */
    if(c->mode==CAM_EVENT&&c->store_after_event&&c->game.outcome!=GAME_DEFEAT){c->store_after_event=0;c->mode=CAM_STORE;snprintf(c->text,sizeof c->text,"Buy supplies, crew and weapons. Weapons enter cargo; install them from ship management. Stock persists on revisits.");return;}
    boarding_prepare_departure(&c->game);
    /* V63: closing a <secretSector/> event jumps straight into the Crystal sector. */
    if(c->mode==CAM_EVENT&&c->secret_jump&&c->game.outcome!=GAME_DEFEAT&&secret_sector_enter(c))return;
    if(!campaign_finale_active(c)&&c->boss_phase>=0&&c->boss_phase<2&&c->game.outcome==GAME_VICTORY){
        int old_phase=c->boss_phase;c->boss_phase++;
        if(!start_combat(c,ENEMY_FLAGSHIP)){c->boss_phase=old_phase;return;}
        c->boss_resume_phase=c->boss_phase;repair_hull(c,4);c->game.missiles+=2;return;
    }
    c->mode=CAM_MAP;c->event_id=EVENT_NONE;c->event_resolved=1;c->imported_node=-1;c->imported_ship=-1;c->imported_battle=0;c->imported_surrender_threshold=-1;c->imported_escape_warned=0;
    for(i=0;i<c->node_count;i++)if(campaign_can_jump(c,i)&&!c->visited[i]){c->selected=i;break;}
    if(campaign_finale_active(c)){int turns=campaign_base_turns_left(c);
        if(turns>0)snprintf(c->text,sizeof c->text,"The flagship is attacking the Federation base: %d jump%s left to engage it.",turns,turns==1?"":"s");
        else snprintf(c->text,sizeof c->text,"Last Stand. The flagship %s. Phase %d/3 awaits; intercept it before it reaches the base.",c->boss_move?"jumps next turn":"holds position",(c->boss_resume_phase<0?0:c->boss_resume_phase)+1);
    }
    else snprintf(c->text,sizeof c->text,"Sector %d/%d. Choose a linked beacon, or manage the ship. Fleet distance: %d jumps.",c->sector+1,c->sector_count,8-c->sector_jumps);
}
int campaign_can_service_home(const Campaign*c){
    return c&&c->reward.kind==REWARD_NONE&&(c->mode==CAM_MAP||c->mode==CAM_STORE||c->mode==CAM_EVENT)&&c->imported_battle!=2&&c->imported_battle!=4&&c->imported_battle!=5&&!game_away_crew(&c->game);
}
int campaign_home_threat(const Campaign*c){
    if(!c)return 0;
    const Game*g=&c->game;
    if(g->player.hull<=0||!game_alive_crew(g))return 1;
    for(int i=0;i<g->enemy_crew_count;i++){
        const Crew*r=&g->enemy_crew[i];if(r->ship==0&&(r->health>0||(r->id&&!r->death_processed)))return 1;
    }
    for(int i=0;i<g->player.room_count;i++)if(g->player.rooms[i].fires||g->player.rooms[i].breaches||g->player.rooms[i].oxygen<100)return 1;
    return 0;
}
static void home_service_result(Campaign*c){
    if(c->game.outcome==GAME_DEFEAT){
        c->mode=CAM_DEFEAT;
        snprintf(c->text,sizeof c->text,"The Kestrel was lost during ship management. Check oxygen, doors and crew health on your next run.");
    }
}
int campaign_service_home(Campaign*c){
    if(!campaign_can_service_home(c))return 0;
    Game*g=&c->game;
    if(g->outcome==GAME_DEFEAT||g->player.hull<=0||!game_alive_crew(g)){
        g->outcome=GAME_DEFEAT;g->result_reason=g->player.hull<=0?RESULT_PLAYER_HULL:RESULT_PLAYER_CREW;
        home_service_result(c);return 1;
    }
    if(g->paused)return 0;
    game_service_step(g);home_service_result(c);return 1;
}
void campaign_step(Campaign *c){
    if(c&&c->reward.kind!=REWARD_NONE)return;
    if(c->mode==CAM_EVENT&&!c->event_resolved)return;
    if(c->mode==CAM_MAP||c->mode==CAM_EVENT||c->mode==CAM_STORE){
        if(campaign_can_service_home(c))campaign_service_home(c);
        else {game_service_step(&c->game);home_service_result(c);}
        return;
    }
    if(c->mode!=CAM_COMBAT)return;
    if(c->game.outcome==GAME_PLAYING){
        game_step(&c->game);
        /* BossShip::OnLoop refreshes crewCounts every frame until the hull is gone. */
        if(c->game.enemy.hull>0)flagship_capture_crew(c);
        /* Simulation resolves deaths and departure first. Offers precede a new
         * escape warning; refusing retains the existing charge and gets its warning. */
        if(imported_surrender_offer(c))return;
        if(!c->game.paused&&c->game.enemy_escape_active&&!c->game.enemy_departed&&c->game.outcome==GAME_PLAYING&&c->imported_battle==2&&imported_escape_warning(c))return;
    }
    if(c->game.outcome!=GAME_PLAYING&&c->result_ticks<outcome_duration(&c->game)){
        if(!c->game.paused)c->result_ticks++;
        return;
    }
    if(c->game.outcome==GAME_ESCAPE){
        if(c->imported_battle==2){imported_outcome(c);return;}
        resolved(c,"The enemy escaped. No salvage recovered.");return;
    }
    if(c->game.outcome==GAME_VICTORY){
        /* V59: Mantis (20 crews killed by sector 6) and Crystal (10 Rock ships). */
        if(c->game.result_reason==RESULT_ENEMY_CREW&&++c->ach_counter[ACH_RUN_CREW_SHIPS]>=20&&c->sector<=5)achievements_earn(&c->game,ACH_MANTIS_CREW_DEAD);
        if(c->game.enemy_kind==ENEMY_SHIP&&c->game.enemy_ship>=0&&c->game.enemy_ship<IMPORTED_ENEMY_COUNT&&!strncmp(imported_enemies[c->game.enemy_ship].blueprint,"ROCK_",5)&&++c->ach_counter[ACH_RUN_ROCK_SHIPS]>=10)achievements_earn(&c->game,ACH_CRYSTAL_CLASH);
        if(c->imported_battle==2){imported_outcome(c);return;}
        if(c->boss_phase==2){c->mode=CAM_COMPLETE;snprintf(c->text,sizeof c->text,"Victory! The flagship is defeated. Completed %d sectors in %d jumps with %d hull. Seed %u.",c->sector_count,c->jumps,c->game.player.hull,c->seed);return;}
        if(c->boss_phase>=0&&campaign_finale_active(c)){
            int scrap=30+bounded(c,11);c->scrap+=scrap;
            c->boss_resume_phase=c->boss_phase+1;boss_force_jump(c);c->boss_phase=-1;
            snprintf(c->text,sizeof c->text,c->boss_resume_phase==1?"The flagship breaks off and jumps away to repair. Salvage: %d scrap. Its next form is stronger; catch it before it reaches the base.":"The flagship is crippled but escapes once more. Salvage: %d scrap. Finish it before it reaches the base.",scrap);
            c->mode=CAM_EVENT;c->event_resolved=1;c->event_id=EVENT_AFTERMATH;return;
        }
        if(c->boss_phase>=0){resolved(c,"Flagship platform destroyed. Tender restores 4 hull and supplies 2 missiles before the next phase. Continue when ready.");return;}
        c->fuel+=c->pursued?1:2;if(!c->pursued){campaign_gain_scrap(c,18+c->sector*3);c->game.missiles+=2;}
        /* V55 Drone Recovery Arm: deployed drones come back as parts after a win. */
        if(game_augment_count(&c->game,AUG_DRONE_RECOVERY))for(int i=0;i<c->game.drones.slot_count&&i<GAME_MAX_DRONES;i++)if(c->game.drones.slots[i].deployed)c->game.drone_parts++;
        if(c->pursued&&c->node_kind[c->current]==NODE_EXIT){
            c->mode=CAM_EVENT;c->event_id=EVENT_EXIT;c->event_resolved=0;
            snprintf(c->text,sizeof c->text,"Exit patrol defeated. Recovered 1 emergency fuel. The Federation cache is gone; continue onward.");return;
        }
        resolved(c,c->pursued?"Patrol defeated. Recovered 1 emergency fuel; no scrap. Move toward the exit.":c->game.result_reason==RESULT_ENEMY_CREW?"Enemy crew defeated. Ship secured intact; recovered scrap, 2 fuel and 2 missiles.":"Enemy destroyed. Recovered scrap, 2 fuel and 2 missiles. Manage repairs before your next jump.");
    } else if(c->game.outcome==GAME_DEFEAT){c->mode=CAM_DEFEAT;snprintf(c->text,sizeof c->text,"The Kestrel was lost in sector %d after %d jumps. Start a new run to try again.",c->sector+1,c->jumps);}
}
static int recruit_slot(const Campaign *c){int i;for(i=0;i<c->game.crew_count;i++)if(c->game.crew[i].health<=0)return i;return c->game.crew_count<GAME_MAX_CREW?c->game.crew_count:-1;}
static int drone_cargo_free(const Campaign *c){int i;for(i=0;i<CAM_CARGO_COUNT;i++)if(c->drone_cargo[i]<0)return i;return -1;}
static int cargo_free(const Campaign *c){int i;for(i=0;i<CAM_CARGO_COUNT;i++)if(c->cargo[i]<0)return i;return -1;}
int campaign_offer_count(const Campaign *c){return c->mode==CAM_STORE?CAM_OFFER_COUNT:0;}
int campaign_offer_augment(const Campaign *c){
    unsigned h=(c->seed^0xa76e17u)+(unsigned)c->sector*31337u+(unsigned)c->current*7907u;int pool[AUG_COUNT],n=0;
    for(int id=0;id<AUG_COUNT;id++)if(game_augment_rarity(id)>=1)pool[n++]=id;
    h^=h>>13;h*=0x5bd1e995u;h^=h>>15;return n?pool[h%(unsigned)n]:-1;
}
void campaign_gain_scrap(Campaign *c,int amount){
    if(amount<=0){c->scrap+=amount;return;}
    amount+=amount*10*game_augment_count(&c->game,AUG_SCRAP_COLLECTOR)/100;
    if(game_augment_count(&c->game,AUG_REPAIR_ARM)&&c->game.player.hull<c->game.player.max_hull){c->game.player.hull++;amount-=amount*15/100;}
    c->scrap+=amount;
}
/* V51: weapon offers 4,5,6 are the standard DS stock (Basic, Heavy, Burst II).
 * Offers 8,9,10 are the store's three original-style weapon slots: legacy runs keep
 * Ion/Halberd/Bomb; current runs draw from the base-game list (rarity 1-5, rarer
 * weapons less often, no duplicates), seeded by run, sector and beacon. */
int campaign_offer_weapon(const Campaign *c,int n){
    static const int slots[]={4,5,6,8,9,10},legacy[]={2,3,1,4,5,6};int k=-1,picked[6],i;
    for(i=0;i<6;i++)if(slots[i]==n)k=i;
    if(k<0)return -1;
    if(c->route_policy!=1||k<3)return legacy[k];
    for(i=3;i<=k;i++){unsigned h=(c->seed^0x57ea9d1u)+(unsigned)c->sector*7919u+(unsigned)c->current*104729u+(unsigned)i*2654435761u;int total=0,id,pick=-1;
        for(id=0;id<GAME_CATALOG_COUNT;id++){int r=game_weapon_rarity(id),dup=0;for(int j=3;j<i;j++)dup|=picked[j]==id;if(game_weapon_player(id)&&r>=1&&r<=5&&!dup)total+=6-r;}
        h^=h>>13;h*=0x5bd1e995u;h^=h>>15;int roll=total?(int)(h%(unsigned)total):0;
        for(id=0;id<GAME_CATALOG_COUNT&&pick<0;id++){int r=game_weapon_rarity(id),dup=0;for(int j=3;j<i;j++)dup|=picked[j]==id;if(game_weapon_player(id)&&r>=1&&r<=5&&!dup){roll-=6-r;if(roll<0)pick=id;}}
        picked[i]=pick<0?2:pick;}
    return picked[k];
}
/* V56: offer 14 is a store-seeded drone schematic of store rarity (all base-game
 * player drones except Combat I, which offer 12 always sells). Older route
 * policies keep Defense I. */
/* V58: offer 15 installs one system this ship lacks (original prices): cloaking,
 * shields, doors, medbay or sensors, chosen per store. The Kestrel lacks only
 * cloaking, so its stores keep selling cloaking. -1 when nothing is missing. */
static int system_missing(const Game*g,int code){
    switch(code){case 10:return g->player_cloak.capacity<=0;case SYS_SHIELDS:case SYS_MEDBAY:return g->player.systems[code].capacity<=0&&game_ship_room(g,code)>=0;
    case 6:return g->subsystems[0][SUB_SENSORS].capacity<=0&&game_ship_room(g,6)>=0;case 7:return g->subsystems[0][SUB_DOORS].capacity<=0&&game_ship_room(g,7)>=0;default:return 0;}}
int campaign_offer_system(const Campaign *c){static const int codes[]={10,SYS_SHIELDS,7,SYS_MEDBAY,6};int pool[5],n=0;
    unsigned h=(c->seed^0x5157e3u)+(unsigned)c->sector*6007u+(unsigned)c->current*2971u;
    for(int i=0;i<5;i++)if(system_missing(&c->game,codes[i]))pool[n++]=codes[i];
    h^=h>>13;h*=0x5bd1e995u;h^=h>>15;return n?pool[h%(unsigned)n]:-1;}
int campaign_system_price(int code){switch(code){case 10:return 150;case SYS_SHIELDS:return 125;case 7:return 60;case SYS_MEDBAY:return 50;case 6:return 40;default:return 0;}}
static int install_system(Campaign*c,int code){Game*g=&c->game;int room=game_ship_room(g,code);
    if(code==10)return game_cloak_install(g);
    if(room<0)return 0;
    if(code==6||code==7){Subsystem*s=&g->subsystems[0][code==7?SUB_DOORS:SUB_SENSORS];s->room=room;s->capacity=s->health=1;return 1;}
    {GameSystemState*s=&g->player.systems[code];s->room=room;s->capacity=s->health=code==SYS_SHIELDS?2:1;s->power=0;
     for(int i=0;i<g->player.room_count;i++)if(g->player.rooms[i].id==room)g->player.rooms[i].system=code;}
    return 1;}
int campaign_offer_drone(const Campaign *c){
    unsigned h=(c->seed^0xd2a0e5u)+(unsigned)c->sector*7919u+(unsigned)c->current*104729u;int pool[GAME_DRONE_CATALOG_COUNT],n=0;
    if(c->route_policy!=1)return 1;
    for(int id=1;id<GAME_DRONE_CATALOG_COUNT;id++)if(drones_rarity(id)>=1&&drones_rarity(id)<=2+c->sector/2)pool[n++]=id;
    h^=h>>13;h*=0x5bd1e995u;h^=h>>15;return n?pool[h%(unsigned)n]:1;
}
const char *campaign_offer_name(const Campaign *c,int n){
    static const char *const names[]={"3 fuel","3 missiles","5 hull repair","Human crew","Basic laser (cargo)","Heavy laser (cargo)","Burst II (cargo)","Manage / sell equipment","Ion Blast I (cargo)","Halberd Beam (cargo)","Small Bomb (cargo)","Drone control + Combat I","Combat I (drone cargo)","3 drone parts","Defense I (drone cargo)"};
    static const char*crew_names[]={"Human crew","Engi crew","Mantis crew","Rock crew","Slug crew","Zoltan crew"};
    if(n==3)return crew_names[campaign_species_offer(c)];
    if(n==15){static const char*const names[]={"Shield system (install)",0,0,0,"Medbay (install)",0,"Sensors (install)","Doors (install)",0,0,"Cloaking system (install)"};int code=campaign_offer_system(c);return code>=0&&names[code]?names[code]:"Systems (none missing)";}
    if(n==14){static char drone[48];snprintf(drone,sizeof drone,"%s (drone cargo)",drones_name(campaign_offer_drone(c)));return drone;}
    if(n==16){static char aug[48];snprintf(aug,sizeof aug,"%s (augment)",game_augment_name(campaign_offer_augment(c)));return aug;}
    if(n>=8&&campaign_offer_weapon(c,n)>=0&&c->route_policy==1){static char weapon[48];snprintf(weapon,sizeof weapon,"%s (cargo)",game_weapon_name(campaign_offer_weapon(c,n)));return weapon;}
    return n>=0&&n<CAM_OFFER_COUNT?names[n]:"";
}
int campaign_species_offer(const Campaign*c){return (int)((c->seed+(unsigned)c->sector*3u+(unsigned)c->current)%6u);}
int campaign_offer_price(const Campaign *c,int n){static const int prices[]={6,6,8,45,25,40,60,0,30,65,45,85,50,24,50};static const int crew_prices[]={45,50,55,55,45,60};if(n==3)return crew_prices[campaign_species_offer(c)];if(n==14)return drones_price(campaign_offer_drone(c));if(n==15)return campaign_system_price(campaign_offer_system(c));if(n==16)return game_augment_cost(campaign_offer_augment(c));if(c->route_policy==1&&n>=8&&campaign_offer_weapon(c,n)>=0)return game_weapon_price(campaign_offer_weapon(c,n));return n>=0&&n<CAM_OFFER_COUNT?prices[n]:0;}
int campaign_purchase(Campaign *c,int n){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    int price,slot=-1;if(c->mode!=CAM_STORE||c->current<0||c->current>=c->node_count||n<0||n>=CAM_OFFER_COUNT||c->stock[c->current][n]<=0)return 0;
    if(n==7)return 0; /* Inventory navigation requires an explicit item choice. */
    price=campaign_offer_price(c,n);if(c->scrap<price)return 0;
    if(n==2&&c->game.player.hull>=c->game.player.max_hull)return 0;
    if(n==3){slot=recruit_slot(c);if(slot<0||slot>=GAME_MAX_CREW)return 0;}
    if(((n>=4&&n<=6)||(n>=8&&n<=10))&&cargo_free(c)<0)return 0;
    if(n==11&&(c->game.drones.installed||c->game.drone_parts>1000000-2))return 0;
    if((n==12||n==14)&&drone_cargo_free(c)<0)return 0;
    if(n==13&&c->game.drone_parts>1000000-3)return 0;
    if(n==15&&campaign_offer_system(c)<0)return 0;
    if(n==16){int id=campaign_offer_augment(c);Game probe=c->game;if(id<0||game_add_augment(&probe,id)<0)return 0;}
    /* Authored package: installation must succeed before spending or stocking. */
    if(n==11){
        if(!drones_install(&c->game))return 0;
        drones_equip(&c->game,0,0);c->game.drone_parts+=2;
    }
    c->scrap-=price;c->stock[c->current][n]--;
    if(n==0)c->fuel+=3;else if(n==1)c->game.missiles+=3;else if(n==2)repair_hull(c,5);
    else if(n==3){
        int room=c->game.player.systems[SYS_MEDBAY].room;
        /* Prefer medbay, then another room with free friendly floor space. */
        for(int pass=-1;pass<c->game.player.room_count;pass++){
            int candidate=pass<0?room:c->game.player.rooms[pass].id,occupants=0,area=0;
            for(int r=0;r<c->game.player.room_count;r++)if(c->game.player.rooms[r].id==candidate)area=c->game.player.rooms[r].w*c->game.player.rooms[r].h;
            for(int k=0;k<c->game.crew_count;k++)if(c->game.crew[k].health>0&&c->game.crew[k].ship==0&&c->game.crew[k].room==candidate)occupants++;
            if(occupants<area){room=candidate;break;}
        }
        game_init_crew(&c->game,0,slot,room);if(slot==c->game.crew_count)c->game.crew_count++;
        game_set_crew_species(&c->game,0,slot,campaign_species_offer(c));species_refresh_power(&c->game);
    }
    else if(n==12||n==14)c->drone_cargo[drone_cargo_free(c)]=n==14?campaign_offer_drone(c):0;
    else if(n==13)c->game.drone_parts+=3;
    else if(n==15)install_system(c,campaign_offer_system(c));
    else if(n==16)game_add_augment(&c->game,campaign_offer_augment(c));
    else if(n!=11&&n<15){slot=cargo_free(c);c->cargo[slot]=campaign_offer_weapon(c,n);}
    snprintf(c->text,sizeof c->text,"%s complete. %d scrap remaining.",n==7?"Sale":"Purchase",c->scrap);return 1;
}
int campaign_upgrade_price(const Campaign *c,int system){
    static const int maximum[]={8,8,8,3,3,3};int cap;
    if(system==-1)return c->game.player.reactor<20?20+c->game.player.reactor:-1;
    if(system<0||system>=GAME_SYSTEM_COUNT)return -1;
    cap=c->game.player.systems[system].capacity;
    if(c->game.player.systems[system].room<0||cap>=maximum[system])return -1;
    return 10+cap*8;
}
int campaign_upgrade(Campaign *c,int system){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    int price;if(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))return 0;price=campaign_upgrade_price(c,system);if(price<0||c->scrap<price)return 0;
    c->scrap-=price;if(system==-1)c->game.player.reactor++;else{c->game.player.systems[system].capacity++;c->game.player.systems[system].health++;}
    if(system==-1)c->ach_flags|=ACH_FLAG_REACTOR;else if(system==SYS_WEAPONS)c->ach_flags|=ACH_FLAG_WEAPONS;
    snprintf(c->text,sizeof c->text,"Upgrade installed. Allocate reactor power in ship management.");return 1;
}
int campaign_swap(Campaign *c,int cargo,int slot){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    int old;
    if((c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))||cargo<0||cargo>=CAM_CARGO_COUNT||c->cargo[cargo]<0||slot<0||slot>c->game.weapon_count||slot>=GAME_MAX_WEAPONS)return 0;
    old=slot<c->game.weapon_count?c->game.weapons[slot].catalog_id:-1;
    if(!game_equip(&c->game,slot,c->cargo[cargo]))return 0;
    c->cargo[cargo]=old;return 1;
}
int campaign_rescue(Campaign *c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
    if(c->mode!=CAM_MAP||c->fuel>0||game_away_crew(&c->game)||c->sector_jumps>999998)return 0;
    c->sector_jumps+=2;c->fuel=2;
    if(c->scrap>=10){c->scrap-=10;snprintf(c->text,sizeof c->text,"Rescue delivered 2 fuel for 10 scrap. The fleet advanced 2 jumps.");}
    else {repair_hull(c,-2);if(c->game.player.hull<1)c->game.player.hull=1;snprintf(c->text,sizeof c->text,"Emergency salvage yielded 2 fuel at a cost of up to 2 hull. The fleet advanced 2 jumps.");}
    if(campaign_node_captured(c,c->current,0))patrol(c);
    return 1;
}




int campaign_teleporter_price(const Campaign*c){
 const Teleporter*t=&c->game.teleporter[0];return !t->installed?120:t->capacity<3?40:-1;
}
int campaign_install_teleporter(Campaign*c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_STORE||c->game.teleporter[0].installed||c->scrap<120)return 0;
 if(!boarding_install(&c->game,0))return 0;
 c->scrap-=120;snprintf(c->text,sizeof c->text,"Teleporter installed. Open BOARD to allocate power and gather crew.");return 1;
}
int campaign_upgrade_teleporter(Campaign*c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 Teleporter*t=&c->game.teleporter[0];
 if(!t->installed||t->capacity>=3||c->scrap<40||(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved)))return 0;
 t->capacity++;t->health++;c->scrap-=40;snprintf(c->text,sizeof c->text,"Teleporter upgraded. Allocate additional power to shorten its cooldown.");return 1;
}
int campaign_abandon_boarders(Campaign*c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_EVENT||!c->event_resolved||c->game.outcome!=GAME_VICTORY)return 0;
 int count=game_abandon_crew(&c->game);
 if(!game_alive_crew(&c->game)){c->mode=CAM_DEFEAT;c->game.outcome=GAME_DEFEAT;c->game.result_reason=RESULT_PLAYER_CREW;snprintf(c->text,sizeof c->text,"No crew remain after abandoning the boarding party.");}
 else snprintf(c->text,sizeof c->text,"%d stranded crew abandoned. Their identities will not return.",count);
 return count;
}

/* Costs match the locally imported base subsystem upgrade tables. */
int campaign_artillery_upgrade_price(const Campaign*c){static const int prices[]={30,50,80};int level=c->game.player_artillery.capacity;return level<1||level>=4?-1:prices[level-1];}
int campaign_upgrade_artillery(Campaign*c){int price=campaign_artillery_upgrade_price(c);Artillery*a=&c->game.player_artillery;
    if(c->reward.kind!=REWARD_NONE||(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))||price<0||c->scrap<price)return 0;
    c->scrap-=price;a->capacity++;a->health++;snprintf(c->text,sizeof c->text,"Artillery Beam upgraded to level %d.",a->capacity);return 1;}
int campaign_cloak_upgrade_price(const Campaign*c){int level=c->game.player_cloak.capacity;return level<1||level>=3?-1:level==1?45:60;}
int campaign_upgrade_cloak(Campaign*c){int price=campaign_cloak_upgrade_price(c);
    if(c->reward.kind!=REWARD_NONE||(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))||price<0||c->scrap<price)return 0;
    c->scrap-=price;c->game.player_cloak.capacity++;c->game.player_cloak.health++;snprintf(c->text,sizeof c->text,"Cloaking upgraded to level %d.",c->game.player_cloak.capacity);return 1;}
int campaign_subsystem_upgrade_price(const Campaign*c,int kind){
 static const int prices[2][2]={{35,50},{25,40}};
 if(kind<0||kind>=SUB_COUNT)return -1;
 const Subsystem*s=&c->game.subsystems[0][kind];
 return s->capacity>=1&&s->capacity<3?prices[kind][s->capacity-1]:-1;
}
int campaign_upgrade_subsystem(Campaign*c,int kind){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))return 0;
 int price=campaign_subsystem_upgrade_price(c,kind);if(price<0||c->scrap<price)return 0;
 Subsystem*s=&c->game.subsystems[0][kind];s->capacity++;s->health++;c->scrap-=price;
 snprintf(c->text,sizeof c->text,"%s upgraded. No reactor power required.",kind==SUB_DOORS?"Doors":"Sensors");return 1;
}

int campaign_dismiss_crew(Campaign*c,int index){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_MAP&&c->mode!=CAM_STORE&&!(c->mode==CAM_EVENT&&c->event_resolved))return 0;
 Game*g=&c->game;if(index<0||index>=g->crew_count||g->crew[index].ship!=0||g->crew[index].health<=0||game_alive_crew(g)<=1)return 0;
 char name[GAME_CREW_NAME_SIZE];snprintf(name,sizeof name,"%s",g->crew[index].name);
 for(int i=index;i<g->crew_count-1;i++)g->crew[i]=g->crew[i+1];
 memset(&g->crew[--g->crew_count],0,sizeof g->crew[0]);species_refresh_power(g);
 snprintf(c->text,sizeof c->text,"%s left the crew. Their position is now available.",name);return 1;
}

/* Equipment mutations are limited to management states; sales require a store. */
static int equipment_management(const Campaign*c){return c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved);}
int campaign_unequip(Campaign*c,int slot){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c)||slot<0||slot>=c->game.weapon_count)return 0;
 int cargo=cargo_free(c);if(cargo<0)return 0;
 int id=c->game.weapons[slot].catalog_id;if(!game_remove_weapon(&c->game,slot))return 0;
 c->cargo[cargo]=id;snprintf(c->text,sizeof c->text,"Weapon moved to cargo.");return 1;
}
int campaign_reorder_weapon(Campaign*c,int from,int to){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c)||!game_reorder_weapon(&c->game,from,to))return 0;
 snprintf(c->text,sizeof c->text,"Weapon order updated. Power priority is left to right.");return 1;
}
int campaign_weapon_sell_price(const Campaign*c,int installed,int index){
 int id;if(installed!=0&&installed!=1)return -1;
 if(installed){if(index<0||index>=c->game.weapon_count)return -1;id=c->game.weapons[index].catalog_id;}
 else {if(index<0||index>=CAM_CARGO_COUNT)return -1;id=c->cargo[index];}
 return game_weapon_player(id)?game_weapon_price(id)/2:-1;
}
int campaign_sell_weapon(Campaign*c,int installed,int index){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_STORE)return 0;
 int price=campaign_weapon_sell_price(c,installed,index);if(price<0||c->scrap>1000000-price)return 0;
 if(installed){if(!game_remove_weapon(&c->game,index))return 0;}else c->cargo[index]=-1;
 c->scrap+=price;snprintf(c->text,sizeof c->text,"Selected weapon sold for %d scrap.",price);return 1;
}

/* Drone costs are authored demake economy values, not verified base-game prices. */
int campaign_drone_upgrade_price(const Campaign*c){
 const DroneControl*d=&c->game.drones;
 return d->installed&&d->capacity>=2&&d->capacity<8?20+10*d->capacity:-1;
}
int campaign_upgrade_drones(Campaign*c){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c))return 0;
 int price=campaign_drone_upgrade_price(c);if(price<0||c->scrap<price)return 0;
 c->game.drones.capacity++;c->game.drones.health++;c->scrap-=price;
 snprintf(c->text,sizeof c->text,"Drone control upgraded. Allocate reactor power on the DRONE page.");return 1;
}
int campaign_drone_swap(Campaign*c,int cargo,int slot){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c)||cargo<0||cargo>=CAM_CARGO_COUNT||slot<0||slot>=c->game.drones.slot_count||slot>=GAME_MAX_DRONES)return 0;
 int id=c->drone_cargo[cargo],old=c->game.drones.slots[slot].catalog_id;
 if(!drones_equip(&c->game,slot,id))return 0;
 c->drone_cargo[cargo]=old;snprintf(c->text,sizeof c->text,"Drone schematic installed. Deployment uses one part per encounter.");return 1;
}
int campaign_drone_unequip(Campaign*c,int slot){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c)||slot<0||slot>=c->game.drones.slot_count||slot>=GAME_MAX_DRONES)return 0;
 int cargo=drone_cargo_free(c);if(cargo<0)return 0;
 int id=c->game.drones.slots[slot].catalog_id;if(!drones_remove(&c->game,slot))return 0;
 c->drone_cargo[cargo]=id;snprintf(c->text,sizeof c->text,"Drone schematic moved to cargo.");return 1;
}
int campaign_drone_reorder(Campaign*c,int from,int to){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(!equipment_management(c)||!drones_reorder(&c->game,from,to))return 0;
 snprintf(c->text,sizeof c->text,"Drone order updated. Power priority is left to right.");return 1;
}
int campaign_drone_sell_price(const Campaign*c,int installed,int index){
 int id;if(installed!=0&&installed!=1)return -1;
 if(installed){if(!c->game.drones.installed||index<0||index>=c->game.drones.slot_count||index>=GAME_MAX_DRONES)return -1;id=c->game.drones.slots[index].catalog_id;}
 else {if(index<0||index>=CAM_CARGO_COUNT)return -1;id=c->drone_cargo[index];}
 return id>=0&&id<GAME_DRONE_CATALOG_COUNT?drones_price(id)/2:-1;
}
int campaign_sell_drone(Campaign*c,int installed,int index){
    if(c&&c->reward.kind!=REWARD_NONE)return 0;
 if(c->mode!=CAM_STORE)return 0;
 int price=campaign_drone_sell_price(c,installed,index);if(price<0||c->scrap>1000000-price)return 0;
 if(installed){if(!drones_remove(&c->game,index))return 0;}else c->drone_cargo[index]=-1;
 c->scrap+=price;snprintf(c->text,sizeof c->text,"Selected drone schematic sold for %d scrap.",price);return 1;
}
