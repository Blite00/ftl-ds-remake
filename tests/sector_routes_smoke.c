#include "campaign.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Synthetic exits/combat entry isolate routing and pool membership, not earned wins. */
static void exit_event(Campaign*c){c->mode=CAM_EVENT;c->event_id=EVENT_EXIT;c->event_resolved=0;}
static void routes(void){
 Campaign c,d;
 for(unsigned seed=1;seed<=256;seed++){
  campaign_init_seed(&c,seed);campaign_init_seed(&d,seed);
  assert(c.route_policy==1&&c.sector_types[0]==SECTOR_CIVILIAN);
  assert(!memcmp(&c,&d,sizeof c));
  for(int sector=0;sector<8;sector++){
   int count=sector<6?2:sector==6?1:0;
   assert(c.sector==sector&&campaign_route_count(&c)==count);
   for(int i=sector+1;i<8;i++)assert(c.sector_types[i]==SECTOR_UNCHARTED);
   if(count==2){assert(c.next_sector[0]>=SECTOR_CIVILIAN&&c.next_sector[0]<=SECTOR_PIRATE);assert(c.next_sector[1]>=SECTOR_CIVILIAN&&c.next_sector[1]<=SECTOR_PIRATE);assert(c.next_sector[0]!=c.next_sector[1]);}
   if(count==1)assert(c.next_sector[0]==SECTOR_FINAL&&c.next_sector[1]==0);
   if(!count){assert(c.next_sector[0]==0&&c.next_sector[1]==0);break;}
   exit_event(&c);d=c;
   for(int i=0;i<20;i++){
    assert(campaign_choice_count(&c)==count);assert(campaign_route_count(&c)==count);
    assert(strlen(campaign_choice_name(&c,0))>0);assert(strlen(campaign_sector_summary(c.next_sector[0]))>0);
    assert(campaign_choice_enabled(&c,0));
   }
   assert(!memcmp(&c,&d,sizeof c));
   assert(!campaign_choice(&c,-1)&&!campaign_choice(&c,count));assert(!memcmp(&c,&d,sizeof c));
   if(count==2){campaign_continue(&c);assert(!memcmp(&c,&d,sizeof c));}
   c.game.crew[0].ship=1;d=c;assert(!campaign_choice_enabled(&c,0)&&!campaign_choice(&c,0));assert(!memcmp(&c,&d,sizeof c));c.game.crew[0].ship=0;
   int selected=(seed+sector)%count,expected=c.next_sector[selected],fuel=c.fuel,scrap=c.scrap;
   c.pursued=sector%2;
   assert(campaign_choice(&c,selected)&&c.mode==CAM_MAP);
   assert(c.sector_types[c.sector]==expected);
   assert(c.fuel==fuel+(sector%2?0:3)&&c.scrap==scrap+(sector%2?0:12));
   d=c;assert(!campaign_choice(&c,selected)&&!memcmp(&c,&d,sizeof c));
  }
  assert(c.sector_types[7]==SECTOR_FINAL);
 }
 campaign_init_legacy(&c);assert(!c.route_policy&&!campaign_route_count(&c));
 for(int i=0;i<8;i++)assert(c.sector_types[i]==0);
 exit_event(&c);assert(campaign_choice_count(&c)==1);campaign_continue(&c);assert(c.sector==1&&c.mode==CAM_MAP);
 assert(c.next_sector[0]==0&&c.next_sector[1]==0);
}
static void pools(void){
 int totals[SECTOR_TYPE_COUNT][NODE_EXIT+1]={{0}};
 int enemy_seen[SECTOR_TYPE_COUNT][ENEMY_KIND_COUNT]={{0}};
 static const unsigned allowed[]={0,
  (1<<ENEMY_SCOUT)|(1<<ENEMY_INTERCEPTOR)|(1<<ENEMY_ASSAULT)|(1<<ENEMY_REBEL),
  (1<<ENEMY_SCOUT)|(1<<ENEMY_ARMORED)|(1<<ENEMY_INTERCEPTOR)|(1<<ENEMY_ASSAULT),
  (1<<ENEMY_REBEL)|(1<<ENEMY_REBEL_BOARDER)|(1<<ENEMY_INTERCEPTOR)|(1<<ENEMY_ASSAULT),
  (1<<ENEMY_ASSAULT)|(1<<ENEMY_INTERCEPTOR)|(1<<ENEMY_ARMORED)|(1<<ENEMY_SCOUT)|(1<<ENEMY_REBEL_BOARDER)};
 /* V52: every sector type also meets original ships of its peoples. */
 static const char*const peoples[]={"","JELLY_BUTTON ROCK_SCOUT ROCK_FIGHT FED_SCOUT FED_BOMBER CIRCLE_SCOUT CIRCLE_BOMBER ZOLTAN_FIGHTER","CIRCLE_ ROCK_ ZOLTAN_","REBEL_SKINNY_ELITE","_P MANTIS_"};
 for(int type=SECTOR_CIVILIAN;type<=SECTOR_PIRATE;type++)for(unsigned seed=1;seed<=512;seed++){
  Campaign c;campaign_init_seed(&c,seed);c.next_sector[0]=type;exit_event(&c);assert(campaign_choice(&c,0));
  assert(c.sector==1&&c.sector_types[1]==type);
  assert(c.node_kind[2]==NODE_SUPPLY&&c.node_kind[8]==NODE_STORE&&c.node_kind[17]==NODE_STORE&&c.node_kind[20]==NODE_REPAIR&&c.node_kind[14]==NODE_QUEST);
  for(int i=0;i<c.node_count;i++)totals[type][c.node_kind[i]]++;
  assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT);
  if(c.game.enemy_kind==ENEMY_SHIP){const char*bp=game_ship_blueprint(c.game.enemy_ship),*list=peoples[type];int match=0;char word[24];
   for(const char*q=list;*q;){int n=0;while(q[n]&&q[n]!=' ')n++;snprintf(word,sizeof word,"%.*s",n,q);if(n&&strstr(bp,word))match=1;q+=n;while(*q==' ')q++;}
   assert(match);}
  else assert(allowed[type]&(1u<<c.game.enemy_kind));
  enemy_seen[type][c.game.enemy_kind]++;
 }
 assert(totals[SECTOR_REBEL][NODE_FIGHT]>totals[SECTOR_CIVILIAN][NODE_FIGHT]*2);
 assert(totals[SECTOR_ENGI][NODE_REPAIR]>totals[SECTOR_REBEL][NODE_REPAIR]*2);
 assert(totals[SECTOR_CIVILIAN][NODE_SUPPLY]>totals[SECTOR_REBEL][NODE_SUPPLY]*2);
 for(int type=SECTOR_CIVILIAN;type<=SECTOR_PIRATE;type++)for(int enemy=0;enemy<ENEMY_KIND_COUNT;enemy++)if((allowed[type]|(type==SECTOR_REBEL?0u:1u<<ENEMY_SHIP))&(1u<<enemy))assert(enemy_seen[type][enemy]>0);
}
int main(void){routes();pools();puts("sector_routes_smoke: 256 eight-sector histories, pure offers, guarded choices, legacy path and 2048 weighted pool samples passed");return 0;}
