/* V61: quest beacons (placement, arrival, sector change, save31 with catalog hash),
 * fleet delay, quest ships by name and crew species mixes. */
#include "campaign.h"
#include "save.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static unsigned char raw[24000];
static void put32(unsigned char*p,unsigned v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static void reheader(size_t n){unsigned h=2166136261u;put32(raw+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^raw[i])*16777619u;put32(raw+16,h);}
static int linked_unvisited(const Campaign*x){for(int n=1;n<x->node_count-1;n++)if(campaign_linked(x,x->current,n)&&!x->visited[n]&&x->node_kind[n]!=NODE_STORE)return n;return -1;}
int main(void){
 /* The catalog compiles quest effects, named quest ships and crew mixes. */
 int quests=0,delay=-1,mixes=0,target=-1;
 for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT;i++)if(imported_event_effects[i].kind==IE_QUEST){quests++;assert(imported_event_effects[i].min>=0&&imported_event_effects[i].min<IMPORTED_EVENT_NODE_COUNT);if(target<0)target=imported_event_effects[i].min;}
 for(int i=0;i<IMPORTED_EVENT_NODE_COUNT&&delay<0;i++)if(imported_event_nodes[i].pursuit<0&&!imported_event_nodes[i].choice_count&&!imported_event_nodes[i].alt_count&&imported_event_nodes[i].ship<0&&imported_event_nodes[i].hostile<0)delay=i;
 for(int i=0;i<IMPORTED_EVENT_SHIP_COUNT;i++)if(imported_event_ships[i].crew0>=0){mixes++;assert(imported_event_ships[i].crew0_percent+imported_event_ships[i].crew1_percent+imported_event_ships[i].crew2_percent>0);}
 assert(quests>0&&target>=0&&mixes>0);
 /* Placement: an unvisited ordinary beacon becomes a QUEST beacon. */
 campaign_init_seed(&c,6101);c.mode=CAM_MAP;assert(campaign_quest_add(&c,target)==1);
 int q=-1;for(int i=0;i<CAM_QUESTS;i++)if(c.quest_event[i]==target)q=i;
 assert(q>=0&&c.quest_node[q]>0&&c.quest_node[q]<c.node_count-1&&!c.visited[c.quest_node[q]]&&c.node_kind[c.quest_node[q]]==NODE_QUEST&&campaign_quest_at(&c,c.quest_node[q])==q);
 /* Save31 keeps the quest; another event catalog drops it. */
 assert(campaign_save(&c,"tests/v61.sav")&&campaign_load(&d,"tests/v61.sav")&&d.quest_event[q]==target&&d.quest_node[q]==c.quest_node[q]);
 {FILE*f=fopen("tests/v61.sav","rb");size_t n=fread(raw,1,sizeof raw,f);fclose(f);assert(raw[8]==32);
  put32(raw+n-44,0xdeadbeefu);reheader(n);remove("tests/v61.sav.bak");f=fopen("tests/v61.sav","wb");fwrite(raw,1,n,f);fclose(f);
  assert(campaign_load(&d,"tests/v61.sav")&&d.quest_event[q]==-1&&d.quest_node[q]==-1);}
 remove("tests/v61.sav");remove("tests/v61.sav.bak");
 /* Arriving at the quest beacon runs the quest event instead of the beacon's own. */
 campaign_init_seed(&c,6102);c.mode=CAM_MAP;{int n=linked_unvisited(&c);assert(n>0);assert(campaign_quest_add(&c,target)>=1);
  for(int i=0;i<CAM_QUESTS;i++)if(c.quest_event[i]==target){c.node_kind[c.quest_node[i]]=NODE_FIGHT;c.quest_node[i]=n;c.node_kind[n]=NODE_QUEST;}
  assert(campaign_jump(&c,n)&&campaign_quest_at(&c,n)<0&&(c.mode==CAM_EVENT||c.mode==CAM_COMBAT)&&c.event_id==EVENT_IMPORTED);}
 /* Leaving the sector loses placed quests; quests waiting for room appear in the next sector. */
 campaign_init_seed(&c,6103);assert(campaign_quest_add(&c,target)==1);assert(campaign_quest_add(&c,target)==1);
 for(int i=0;i<CAM_QUESTS;i++)if(c.quest_event[i]>=0&&i==1){c.node_kind[c.quest_node[i]]=NODE_FIGHT;c.quest_node[i]=-1;}
 c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0)&&c.sector==1);
 {int open=0,placed=0;for(int i=0;i<CAM_QUESTS;i++)if(c.quest_event[i]>=0&&!(c.quest_hidden&(1u<<i))){open++;placed+=c.quest_node[i]>=0&&c.node_kind[c.quest_node[i]]==NODE_QUEST;}assert(open==1&&placed==1);}
 /* Four open quests at most. */
 campaign_init_seed(&c,6104);{int free=0;for(int i=0;i<CAM_QUESTS;i++){free+=c.quest_event[i]<0;}for(int i=0;i<free;i++){assert(campaign_quest_add(&c,target));}}assert(!campaign_quest_add(&c,target));
 /* Fleet delay pushes the rebels back, never below zero. */
 if(delay>=0){campaign_init_seed(&c,6105);c.mode=CAM_MAP;int n=linked_unvisited(&c);assert(n>0);c.sector_jumps=5;c.scrap=100;assert(campaign_quest_add(&c,delay));
  for(int i=0;i<CAM_QUESTS;i++)if(c.quest_event[i]==delay){c.node_kind[c.quest_node[i]]=NODE_FIGHT;c.quest_node[i]=n;c.node_kind[n]=NODE_QUEST;}
  assert(campaign_jump(&c,n));fprintf(stderr,"delay node %d pursuit %d jumps %d mode %d text %.60s|",delay,imported_event_nodes[delay].pursuit,c.sector_jumps,c.mode,c.text);assert(c.sector_jumps==6+imported_event_nodes[delay].pursuit||c.sector_jumps==0);}
 printf("PASS: quests (%d quest effects, %d crew mixes): placement, arrival, sector change, cap, save31 + catalog hash, fleet delay.\n",quests,mixes);
 return 0;
}
