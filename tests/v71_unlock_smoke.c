/* V71: a Kestrel (A or B) reaching sector 5 unlocks the Engi Cruiser (layout 6, The Torus);
   the unlock reaches the profile file, survives a death, and the hangar shows it.
   A run continued past sector 5 (crossed under an older build) also earns it. */
#include "campaign.h"
#include "hangar.h"
#include "profile.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
static int next_hop(const Campaign*k,int to){int dist[CAM_NODE_COUNT],q[CAM_NODE_COUNT],h=0,tl=0;for(int i=0;i<k->node_count;i++){dist[i]=-1;}dist[to]=0;q[tl++]=to;
 while(h<tl){int a=q[h++];for(int b=0;b<k->node_count;b++){if(dist[b]<0&&campaign_linked(k,a,b)){dist[b]=dist[a]+1;q[tl++]=b;}}}
 int best=-1;for(int i=0;i<k->node_count;i++){if(campaign_can_jump(k,i)&&dist[i]>=0&&(best<0||dist[i]<dist[best]))best=i;}return best;}
/* Play (enemies destroyed, rewards declined) until the run enters sector index 4 = "Sector 5". */
static int reach_sector5(unsigned seed,int layout){
 campaign_init_seed(&c,seed);assert(game_init_ship(&c.game,layout));c.sector=3;
 for(int guard=0;guard<200&&c.sector==3;guard++){
  int exit=-1;for(int i=0;i<c.node_count;i++){if(c.node_kind[i]==NODE_EXIT)exit=i;}
  if(c.mode==CAM_MAP){int to=exit>=0&&campaign_can_jump(&c,exit)?exit:exit>=0?next_hop(&c,exit):-1;if(to<0||!campaign_jump(&c,to))return 0;}
  for(int g=0;g<40&&c.mode!=CAM_MAP&&c.sector==3;g++){
   if(c.mode==CAM_COMBAT){c.game.enemy.hull=0;for(int k=0;k<5*GAME_HZ&&c.mode==CAM_COMBAT;k++){campaign_step(&c);}}
   if(c.reward.kind){rewards_decline(&c.reward);continue;}
   if(c.mode==CAM_EVENT&&!c.event_resolved){if(!campaign_choice(&c,0)){campaign_continue(&c);}}else{campaign_continue(&c);}}
 }
 return c.sector==4;}
int main(void){
 static const char*path="tests/v71-profile.dat";remove(path);{char b[64];snprintf(b,sizeof b,"%s.bak",path);remove(b);}
 int ok=0;for(unsigned seed=1;seed<40&&!ok;seed++){ok=reach_sector5(seed,1);} /* Kestrel B */
 assert(ok&&(c.unlock_pending&(1u<<6)));
 /* main.c: pending unlocks go straight into the profile file. */
 Profile p;profile_load(&p,path);assert(!profile_layout_unlocked(&p,6));
 for(int k=0;k<18;k++){if(c.unlock_pending&(1u<<k)){profile_unlock_layout(&p,k);}}c.unlock_pending=0;assert(profile_save(&p,path));
 /* Death: the run ends; the next New Run re-reads the profile and the hangar shows the Torus. */
 c.game.player.hull=0;c.game.outcome=GAME_DEFEAT;
 {Profile q;static Hangar h;assert(profile_load(&q,path)==1);hangar_open(&h,&q);assert(hangar_unlocked(&h,6)&&!hangar_unlocked(&h,7)&&hangar_unlocked(&h,0));}
 /* Other ships don't earn it; a Kestrel run continued in sector 6 does. */
 campaign_init_seed(&c,7);assert(game_init_ship(&c.game,4));c.sector=5;campaign_check_unlocks(&c);assert(!c.unlock_pending);
 campaign_init_seed(&c,7);c.sector=5;campaign_check_unlocks(&c);assert(c.unlock_pending==(1u<<6));
 campaign_init_seed(&c,7);c.sector=3;campaign_check_unlocks(&c);assert(!c.unlock_pending);
 remove(path);{char b[64];snprintf(b,sizeof b,"%s.bak",path);remove(b);}
 printf("PASS: Kestrel B reaching sector 5 unlocks the Engi Cruiser; saved to the profile, kept after death, shown in the hangar; continued runs past sector 5 earn it.\n");
 return 0;}
