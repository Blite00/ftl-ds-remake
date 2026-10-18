#include "campaign.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Synthetic combat outcomes below isolate reward/routing rules, not playability. */
static void win(Campaign*c){c->game.outcome=GAME_VICTORY;c->result_ticks=1000000;campaign_step(c);}
static void graph(void){
 Campaign c,d;
 for(unsigned seed=1;seed<=1024;seed++){
  int reached[CAM_NODE_COUNT]={0},queue[CAM_NODE_COUNT],head=0,tail=0;
  campaign_init_seed(&c,seed);campaign_init_seed(&d,seed);
  assert(c.node_count==23&&c.sector_count==8);
  assert(!memcmp(c.links,d.links,sizeof c.links));assert(!memcmp(c.node_y,d.node_y,sizeof c.node_y));
  assert(!memcmp(c.node_kind,d.node_kind,sizeof c.node_kind));
  reached[0]=1;queue[tail++]=0;
  while(head<tail){int a=queue[head++];for(int b=0;b<c.node_count;b++){
   assert(campaign_linked(&c,a,b)==campaign_linked(&c,b,a));
   if(campaign_linked(&c,a,b)&&!reached[b]){reached[b]=1;queue[tail++]=b;}
  }}
  assert(tail==23);
  for(int i=0;i<c.node_count;i++){
   assert(reached[i]&&!(c.links[i]>>c.node_count));assert(!campaign_linked(&c,i,i));
   assert(campaign_node_x(&c,i)>=16&&campaign_node_x(&c,i)<=238);
   assert(campaign_node_y(&c,i)>=56&&campaign_node_y(&c,i)<=134);
  }
  assert(c.node_kind[8]==NODE_STORE&&c.node_kind[17]==NODE_STORE);
  assert(c.node_kind[20]==NODE_REPAIR&&c.node_kind[22]==NODE_EXIT);
  assert(c.links[23]==0&&c.stock[23][0]==0);
  assert(!campaign_linked(&c,0,23)&&!campaign_linked(&c,-1,0));
  assert(campaign_node_x(&c,23)==-1&&campaign_node_y(&c,23)==-1);
  assert(!campaign_can_jump(&c,23)&&!campaign_node_captured(&c,23,100));
  /* Anchor path offers two stores and repair before the exit. */
  int previous=0;for(int n=2;n<=20;n+=3){assert(campaign_linked(&c,previous,n));previous=n;}
  assert(campaign_linked(&c,20,22));
 }
 campaign_init_legacy(&c);assert(c.node_count==12&&c.sector_count==6);
 assert(campaign_node_x(&c,11)==238&&campaign_node_x(&c,12)==-1);
 assert(c.node_kind[5]==NODE_STORE&&c.node_kind[9]==NODE_REPAIR);
}
static void fleet(void){
 Campaign c,d;
 campaign_init(&c);c.sector_jumps=8;assert(campaign_fleet_x(&c,0)==0);
 d=c;assert(!campaign_node_captured(&c,0,0)&&campaign_node_captured(&c,0,1));
 assert(!campaign_node_captured(&c,1,1)&&campaign_node_captured(&c,1,2));
 assert(!memcmp(&c,&d,sizeof c));
 for(int n=0;n<c.node_count;n++){assert(campaign_node_captured(&c,n,100));assert(!campaign_node_captured(&c,n,-100));}
 campaign_init_legacy(&c);c.sector_jumps=9;assert(campaign_node_captured(&c,1,0)&&campaign_node_captured(&c,2,0));
 assert(!campaign_node_captured(&c,3,0));
 /* A captured store cannot trade; normal stores retain stock through visits. */
 campaign_init(&c);c.current=5;c.sector_jumps=30;int scrap=c.scrap,stock=c.stock[8][0],fuel=c.fuel;
 assert(campaign_jump(&c,8)&&c.mode==CAM_COMBAT&&c.pursued);
 assert(!campaign_purchase(&c,0));win(&c);assert(c.mode==CAM_EVENT&&c.scrap==scrap&&c.fuel==fuel&&c.stock[8][0]==stock);
 campaign_step(&c);assert(c.fuel==fuel);campaign_continue(&c);assert(c.mode==CAM_MAP);
 assert(campaign_jump(&c,5));win(&c);campaign_continue(&c);assert(campaign_jump(&c,8)&&c.mode==CAM_COMBAT);
 win(&c);assert(c.scrap==scrap&&c.fuel==fuel);
 /* Capture also suppresses supply rewards. */
 campaign_init(&c);c.sector_jumps=30;scrap=c.scrap;assert(campaign_jump(&c,2));win(&c);
 assert(c.event_id==EVENT_AFTERMATH&&c.scrap==scrap&&!campaign_choice(&c,0));
 /* Exit patrol must be won; then sector advance has no captured cache. */
 campaign_init(&c);c.current=20;c.sector_jumps=30;scrap=c.scrap;fuel=c.fuel;
 assert(campaign_jump(&c,22)&&c.mode==CAM_COMBAT);assert(!campaign_choice(&c,0));win(&c);
 assert(c.event_id==EVENT_EXIT&&!c.event_resolved&&c.fuel==fuel&&c.scrap==scrap);
 campaign_step(&c);assert(c.fuel==fuel);assert(campaign_choice(&c,0));
 assert(c.sector==1&&c.node_count==23&&c.sector_count==8&&c.mode==CAM_MAP&&c.fuel==fuel&&c.scrap==scrap);
 assert(!campaign_choice(&c,0));
 /* Final captured exit also loses the tender's hull and ammunition. */
 campaign_init(&c);c.sector=7;c.current=20;c.sector_jumps=30;c.game.player.hull=20;
 int missiles=c.game.missiles;assert(campaign_jump(&c,22));win(&c);assert(campaign_choice(&c,0));
 assert(c.mode==CAM_COMBAT&&c.boss_phase==0&&c.game.player.hull==20&&c.game.missiles==missiles);
}
static void waits(void){
 Campaign c,d;campaign_init(&c);int fuel=c.fuel,scrap=c.scrap;
 assert(campaign_wait(&c)&&c.sector_jumps==1&&c.fuel==fuel&&c.scrap==scrap);
 c.mode=CAM_STORE;d=c;assert(!campaign_wait(&c)&&!memcmp(&c,&d,sizeof c));
 c.mode=CAM_MAP;c.game.crew[0].ship=1;d=c;assert(!campaign_wait(&c)&&!campaign_rescue(&c)&&!memcmp(&c,&d,sizeof c));
 c.game.crew[0].ship=0;c.sector_jumps=8;assert(campaign_wait(&c)&&c.mode==CAM_COMBAT&&c.fuel==fuel);
 d=c;assert(!campaign_wait(&c)&&!memcmp(&c,&d,sizeof c));
 campaign_init(&c);c.fuel=0;c.sector_jumps=7;scrap=c.scrap;
 assert(campaign_rescue(&c)&&c.sector_jumps==9&&c.fuel==2&&c.scrap==scrap-10&&c.mode==CAM_COMBAT);
 assert(!campaign_rescue(&c));win(&c);assert(c.fuel==3);campaign_step(&c);assert(c.fuel==3);
 campaign_init(&c);c.fuel=0;c.scrap=0;c.game.player.hull=1;
 assert(campaign_rescue(&c)&&c.game.player.hull==1&&c.sector_jumps==2&&c.mode==CAM_MAP);
}
int main(void){graph();fleet();waits();puts("large_map_smoke: 1024 seeds, active bounds, fleet, captured services/exit, waits and rescue passed");return 0;}
