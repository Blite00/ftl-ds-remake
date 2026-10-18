/* V63: the hidden Crystal homeworld (<secretSector/>). The effect compiles, an
 * event with it jumps the run straight into SECTOR_SECRET (CRYSTAL_HOME pools,
 * Crystal ships, START_BEACON_CRYSTAL), CRYSTAL_UNLOCK unlocks the Crystal
 * cruiser (layout 16), Ancestry is earned only in the Rock cruiser, the sector
 * round-trips through saves, the run continues afterwards and older saves load. */
#include "campaign.h"
#include "save.h"
#include "species.h"
#include "achievements.h"
#include "event_catalog.h"
#include "enemy_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,e;
static unsigned char raw[24000];
static const char*path="tests/v63.sav",*backup="tests/v63.sav.bak";
static void put32(unsigned char*p,unsigned v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static void reheader(size_t n){unsigned h=2166136261u;put32(raw+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^raw[i])*16777619u;put32(raw+16,h);}
static size_t readsave(void){FILE*f=fopen(path,"rb");assert(f);size_t n=fread(raw,1,sizeof raw,f);assert(feof(f)&&!fclose(f));return n;}
static void writesave(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(raw,1,n,f)==n&&!fclose(f));}
static void clean(void){remove(path);remove(backup);}
static int root_named(const char*id){for(int i=0;i<campaign_imported_count();i++){if(!strcmp(campaign_imported_id(i),id))return i;}return -1;}
static int node_has(int node,int kind,int value){const ImportedEventNode*n=&imported_event_nodes[node];
 for(int i=0;i<n->effect_count;i++){const ImportedEventEffect*x=&imported_event_effects[n->first_effect+i];if(x->kind==kind&&(value<0||x->min==value))return 1;}return 0;}
static int pool_has(int sector,int category,int root){for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++){if(imported_event_pools[i].sector==sector&&imported_event_pools[i].category==category&&(root<0||imported_event_pools[i].root==root))return 1;}return 0;}
static int seen(const Campaign*x,int root){return root<31?(x->event_seen>>root)&1:(int)((x->event_seen_ext[(root-31)>>5]>>((root-31)&31))&1u);}
static int linked_from(const Campaign*x,int first){for(int n=first;n<x->node_count-1;n++){if(campaign_linked(x,x->current,n)&&!x->visited[n]&&x->node_kind[n]!=NODE_STORE&&campaign_quest_at(x,n)<0)return n;}return -1;}
static int linked_unvisited(const Campaign*x){return linked_from(x,1);}
static void exit_sector(Campaign*x,int choice){x->mode=CAM_EVENT;x->event_id=EVENT_EXIT;x->event_resolved=0;x->current=x->selected=x->node_count-1;x->visited[x->current]=1;assert(campaign_choice(x,choice));}
/* Run the event node at a linked beacon (as an unmarked quest beacon, like a homeworld special). */
static void visit_event(Campaign*x,int node){int n=linked_unvisited(x),q=-1;assert(n>0);assert(campaign_quest_add(x,node)>=1);
 for(int i=0;i<CAM_QUESTS;i++){if(x->quest_event[i]==node)q=i;}assert(q>=0);
 if(x->quest_node[q]>=0&&x->quest_node[q]!=n){x->node_kind[x->quest_node[q]]=NODE_FIGHT;}x->quest_node[q]=n;x->node_kind[n]=NODE_QUEST;
 assert(campaign_jump(x,n)&&x->mode==CAM_EVENT&&x->event_id==EVENT_IMPORTED);}
/* ROCK_CRYSTAL_BEACON: take the Crystal crew option, then follow it to the coordinates. */
static void crystal_beacon(Campaign*x,int rock_node){
 visit_event(x,rock_node);
 int blue=-1;for(int i=0;i<campaign_choice_count(x);i++){if(campaign_choice_blue(x,i)){assert(campaign_choice_enabled(x,i));blue=i;}}
 assert(blue>=0&&campaign_choice(x,blue));
 while(x->mode==CAM_EVENT&&!x->event_resolved){assert(campaign_choice_count(x)==1&&campaign_choice(x,0));}
}
static void same(const Campaign*a,const Campaign*b){
 assert(a->sector==b->sector&&a->current==b->current&&a->rng==b->rng&&a->fuel==b->fuel&&a->scrap==b->scrap&&a->event_policy==b->event_policy);
 assert(!memcmp(a->sector_types,b->sector_types,sizeof a->sector_types)&&!memcmp(a->next_sector,b->next_sector,sizeof a->next_sector));
 assert(!memcmp(a->links,b->links,sizeof a->links)&&!memcmp(a->node_kind,b->node_kind,sizeof a->node_kind)&&!memcmp(a->visited,b->visited,sizeof a->visited));
 assert(!memcmp(a->quest_event,b->quest_event,sizeof a->quest_event)&&!memcmp(a->quest_node,b->quest_node,sizeof a->quest_node));
 assert(a->specials_done==b->specials_done&&a->quest_hidden==b->quest_hidden&&!memcmp(a->event_seen_ext,b->event_seen_ext,sizeof a->event_seen_ext));
}
int main(void){
 /* 1. The compiled catalog: the effect, the Rock homeworld special and the CRYSTAL_HOME pools. */
 assert(SECTOR_SECRET==6&&SECTOR_TYPE_COUNT==7&&IE_SECRET_SECTOR==18&&!strcmp(campaign_sector_name(SECTOR_SECRET),"Crystal"));
 int secret_effects=0;for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT;i++){secret_effects+=imported_event_effects[i].kind==IE_SECRET_SECTOR;}assert(secret_effects>=1);
 int rock=root_named("ROCK_CRYSTAL_BEACON"),start=root_named("START_BEACON_CRYSTAL");
 assert(rock>=0&&start>=0&&pool_has(SECTOR_ENGI,7,rock)&&pool_has(SECTOR_SECRET,8,start));
 for(int k=0;k<4;k++)assert(pool_has(SECTOR_SECRET,(int[]){0,1,4,5}[k],-1));
 {int crystal_fight=root_named("CRYSTAL_FIGHT");assert(crystal_fight>=0&&pool_has(SECTOR_SECRET,0,crystal_fight));}
 int rock_node=imported_event_roots[rock].node,unlock=-1;
 for(int i=0;i<IMPORTED_EVENT_NODE_COUNT;i++){if(node_has(i,IE_UNLOCK,8)){assert(node_has(i,IE_AUGMENT,-1));unlock=i;}}
 assert(unlock>=0);
 /* Homeworld special: Engi/Rock sectors sometimes hide the Crystal beacon. */
 {int placed=0;for(unsigned s=6300;s<6340&&!placed;s++){campaign_init_seed(&c,s);c.next_sector[0]=SECTOR_ENGI;exit_sector(&c,0);
   for(int q=0;q<CAM_QUESTS;q++)placed|=c.quest_event[q]==rock_node&&(c.quest_hidden&(1u<<q))&&c.quest_node[q]>0;}assert(placed);}

 /* 2. A Rock cruiser with a Crystal crewmember follows the beacon into the secret sector. */
 assert(campaign_init_ship(&c,6363,12)&&c.game.player_ship==12);
 exit_sector(&c,0);exit_sector(&c,1);assert(c.sector==2&&c.mode==CAM_MAP);
 assert(game_set_crew_species(&c.game,0,0,SPEC_CRYSTAL));
 int types_before[CAM_SECTOR_COUNT];memcpy(types_before,c.sector_types,sizeof types_before);
 crystal_beacon(&c,rock_node);
 assert(c.secret_jump==1&&strstr(c.text,"Crystal sector")&&c.sector==2);
 assert(!campaign_save(&c,path)); /* Never saved mid-event. */
 campaign_continue(&c);
 assert(c.sector==3&&c.sector_types[3]==SECTOR_SECRET&&!c.secret_jump&&!memcmp(types_before,c.sector_types,3*sizeof(int)));
 assert(c.next_sector[0]>=SECTOR_CIVILIAN&&c.next_sector[0]<=SECTOR_PIRATE&&c.next_sector[1]>=SECTOR_CIVILIAN&&c.next_sector[1]<=SECTOR_PIRATE);
 /* 3. Ancestry (find the secret sector with the Rock cruiser). */
 assert(c.game.ach.earned&(1u<<ACH_ROCK_CRYSTAL));
 /* The start beacon (START_BEACON_CRYSTAL) marks the CRYSTAL_UNLOCK quest. */
 assert(c.mode==CAM_EVENT&&c.event_id==EVENT_IMPORTED&&seen(&c,start)&&campaign_choice_count(&c)==1);
 assert(campaign_choice(&c,0));campaign_continue(&c);assert(c.mode==CAM_MAP);
 {int q=-1;for(int i=0;i<CAM_QUESTS;i++){if(c.quest_event[i]==unlock)q=i;}assert(q>=0&&c.quest_node[q]>0);}

 /* 4. Save/load in the secret sector (format 32, unchanged). */
 clean();assert(campaign_save(&c,path)&&campaign_load(&d,path));same(&c,&d);
 {size_t n=readsave();assert(raw[8]==32&&n>0);}
 e=c;e.event_policy=5;assert(!campaign_save(&e,path)); /* Only original-event runs reach it. */
 e=c;e.sector_types[0]=SECTOR_SECRET;assert(!campaign_save(&e,path));
 e=c;e.sector_types[4]=SECTOR_SECRET;assert(!campaign_save(&e,path)); /* Not a future sector. */
 e=c;e.next_sector[0]=SECTOR_SECRET;assert(!campaign_save(&e,path)); /* Never an offered route. */
 assert(campaign_load(&d,path));same(&c,&d);

 /* 5. CRYSTAL_UNLOCK unlocks the Crystal cruiser (class 8, Type A layout 16). */
 c.unlock_pending=0;
 {int q=-1;for(int i=0;i<CAM_QUESTS;i++){if(c.quest_event[i]==unlock)q=i;}int n=linked_unvisited(&c);assert(n>0);
  c.node_kind[c.quest_node[q]]=NODE_FIGHT;c.quest_node[q]=n;c.node_kind[n]=NODE_QUEST;
  int fuel=c.fuel;assert(campaign_jump(&c,n)&&c.event_id==EVENT_IMPORTED);
  assert(c.unlock_pending==1u<<16&&strstr(c.text,"Ship unlocked")&&c.fuel>=fuel-1+3);}
 campaign_continue(&c);assert(c.mode==CAM_MAP);

 /* 6. CRYSTAL_HOME beacons: fights draw HOSTILE/BOARDERS_CRYSTAL and meet Crystal ships. */
 {int fights=0,crystal_ships=0,supplies=0;
  for(int trial=0;trial<24;trial++){e=c;e.rng+=(unsigned)trial*7919u;int n=linked_from(&e,3);if(n<0){continue;}int kind=trial&1?NODE_SUPPLY:NODE_FIGHT;e.node_kind[n]=kind;
   Campaign before=e;assert(campaign_jump(&e,n));if(e.event_id!=EVENT_IMPORTED)continue;
   int root=-1;for(int r=0;r<campaign_imported_count();r++){if(seen(&e,r)&&!seen(&before,r))root=r;}
   if(root<0){continue;} /* A repeatable root already seen. */
   if(kind==NODE_FIGHT){assert(pool_has(SECTOR_SECRET,0,root)||pool_has(SECTOR_SECRET,6,root));fights++;
    if(!strcmp(campaign_imported_id(root),"CRYSTAL_FIGHT")&&e.imported_battle==1&&campaign_choice(&e,0)){
     assert(e.mode==CAM_COMBAT&&e.game.enemy_ship>=0&&!strncmp(imported_enemies[e.game.enemy_ship].blueprint,"CRYSTAL_",8));crystal_ships++;}}
   else{assert(pool_has(SECTOR_SECRET,1,root)||pool_has(SECTOR_SECRET,4,root)||pool_has(SECTOR_SECRET,5,root)||root<12);supplies++;}}
  assert(fights>0&&crystal_ships>0&&supplies>0);
  printf("crystal beacons: %d fights (%d Crystal ships), %d supply\n",fights,crystal_ships,supplies);}

 /* 7. The run continues: the secret sector's exit leads to an ordinary sector. */
 {int next=c.next_sector[1];exit_sector(&c,1);assert(c.sector==4&&c.sector_types[3]==SECTOR_SECRET&&c.sector_types[4]==next);
  clean();assert(campaign_save(&c,path)&&campaign_load(&d,path));same(&c,&d);}

 /* 8. Not this close to the Last Stand, and no Ancestry outside the Rock cruiser. */
 assert(campaign_init_ship(&c,6364,12));for(int i=0;i<6;i++){exit_sector(&c,i&1);}assert(c.sector==6&&c.next_sector[0]==SECTOR_FINAL);
 assert(game_set_crew_species(&c.game,0,0,SPEC_CRYSTAL));crystal_beacon(&c,rock_node);
 assert(!c.secret_jump&&strstr(c.text,"out of reach"));campaign_continue(&c);assert(c.sector==6&&c.mode==CAM_MAP&&!(c.game.ach.earned&(1u<<ACH_ROCK_CRYSTAL)));
 assert(campaign_init_ship(&c,6365,0));exit_sector(&c,0);assert(game_set_crew_species(&c.game,0,0,SPEC_CRYSTAL));
 crystal_beacon(&c,rock_node);campaign_continue(&c);assert(c.sector==2&&c.sector_types[2]==SECTOR_SECRET&&!(c.game.ach.earned&(1u<<ACH_ROCK_CRYSTAL)));
 assert(campaign_init_ship(&c,6366,13));exit_sector(&c,0);assert(game_set_crew_species(&c.game,0,0,SPEC_CRYSTAL));
 crystal_beacon(&c,rock_node);campaign_continue(&c);assert(c.sector_types[2]==SECTOR_SECRET&&(c.game.ach.earned&(1u<<ACH_ROCK_CRYSTAL)));

 /* 9. Older saves still load: a format-31 and a format-30 prefix of an ordinary run. */
 campaign_init_seed(&c,6367);exit_sector(&c,0);clean();assert(campaign_save(&c,path));
 {size_t n=readsave();assert(raw[8]==32);
  put32(raw+8,31);reheader(n-8);writesave(n-8);assert(campaign_load(&d,path)&&d.sector==1&&d.sector_types[1]==c.sector_types[1]&&!d.specials_done);
  n=readsave();put32(raw+8,30);reheader(n-36);writesave(n-36);assert(campaign_load(&d,path)&&d.sector==1&&!memcmp(d.sector_types,c.sector_types,sizeof d.sector_types));}
 clean();
 puts("PASS: secret sector effect compiled, Crystal beacon jump, CRYSTAL_HOME pools and ships, CRYSTAL_UNLOCK layout 16, Ancestry (Rock only), save32 round-trip/validation, run continues, format30/31 saves load.");
 return 0;
}
