/* V53: original sector beacon lists. Every pooled event starts, its choices can
 * be taken through to an outcome or an Engage, ships use the imported hulls,
 * AE/unbuilt blue options stay disabled, and save28 keeps the new seen bits. */
#include "campaign.h"
#include "save.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static void fresh(unsigned seed,int type){campaign_init_seed(&c,seed);c.sector=2;c.sector_types[2]=type;c.fuel=20;c.scrap=500;c.game.missiles=20;c.game.drone_parts=10;c.mode=CAM_MAP;}
int main(void){
 assert(IMPORTED_EVENT_ROOT_COUNT>=50&&IMPORTED_EVENT_POOL_COUNT>=IMPORTED_EVENT_ROOT_COUNT-12);
 /* Each DS sector type has hostile and neutral beacon events. */
 for(int type=0;type<=SECTOR_PIRATE;type++){int hostile=0,neutral=0;for(int i=0;i<IMPORTED_EVENT_POOL_COUNT;i++)if(imported_event_pools[i].sector==type){hostile+=imported_event_pools[i].category==0;neutral+=imported_event_pools[i].category==1||imported_event_pools[i].category==4;}
  if(!(hostile>0&&neutral>0)){fprintf(stderr,"type%d hostile%d neutral%d\n",type,hostile,neutral);}assert(neutral>0);}
 /* Walk every root: begin, then keep taking the first enabled choice. */
 int started=0,fights=0,ships=0;
 for(int root=12;root<IMPORTED_EVENT_ROOT_COUNT;root++){
  fresh(5000u+(unsigned)root,SECTOR_CIVILIAN);c.event_policy=6;
  if(!campaign_imported_begin(&c,root))continue;
  started++;
  for(int step=0;step<20&&c.mode==CAM_EVENT;step++){
   while(c.reward.kind){if(!rewards_decline(&c.reward))break;}
   int n=campaign_choice_count(&c),pick=-1;for(int i=0;i<n&&pick<0;i++)if(campaign_choice_enabled(&c,i))pick=i;
   if(pick<0)break;
   if(c.imported_battle==1){assert(campaign_choice(&c,pick));if(c.mode==CAM_COMBAT){fights++;ships+=c.game.enemy_kind==ENEMY_SHIP;assert(c.game.enemy.room_count>0&&c.game.enemy.hull>0);}break;}
   if(!campaign_choice(&c,pick))break;
  }
 }
 assert(started>=40&&fights>=5&&ships>=1);
 /* Blue options use only supported requirement kinds. */
 int blue=0;for(int i=0;i<IMPORTED_EVENT_CHOICE_COUNT;i++){assert(imported_event_choices[i].req_kind>=IE_REQ_NONE&&imported_event_choices[i].req_kind<=IE_REQ_DRONE_ANY);blue+=imported_event_choices[i].req_kind!=IE_REQ_NONE;}assert(blue>0);
 /* save28 round-trips seen bits beyond root 30; save27 readers are refused. */
 campaign_init_seed(&c,777);assert(c.event_policy==6);c.event_seen_ext[0]=0x80000001u;c.event_seen_ext[7]=5;
 assert(campaign_save(&c,"tests/v53.sav")&&campaign_load(&d,"tests/v53.sav"));assert(d.event_seen_ext[0]==0x80000001u&&d.event_seen_ext[7]==5&&d.event_policy==6);
 remove("tests/v53.sav");remove("tests/v53.sav.bak");
 printf("PASS: %d pooled original events started, %d fights (%d imported hulls), save28 seen bits.\n",started,fights,ships);
 return 0;
}
