/* V41 original final sector: flagship route/movement cadence, base countdown
 * and loss, interception, post-phase jumps and reversal at the base, rebel
 * spread, repair stations, base supplies, victory and save25 round trips. */
#include "campaign.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static void finale(unsigned seed){
 campaign_init_seed(&c,seed);c.sector=c.sector_count-2;for(int i=1;i<c.sector_count-1;i++)c.sector_types[i]=SECTOR_REBEL;c.next_sector[0]=SECTOR_FINAL;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;
 assert(campaign_choice(&c,0)&&c.sector==c.sector_count-1&&c.mode==CAM_MAP);
 assert(campaign_finale_active(&c)&&campaign_base_node(&c)==c.node_count-1);c.fuel=99;
}
/* Fixture move with a legitimate visited chain from the start (save validation). */
static void teleport(int n){int prev[CAM_NODE_COUNT],q[CAM_NODE_COUNT],h=0,t=0;for(int i=0;i<c.node_count;i++)prev[i]=-2;prev[0]=-1;q[t++]=0;
 while(h<t){int v=q[h++];for(int u=0;u<c.node_count;u++)if(prev[u]==-2&&campaign_linked(&c,v,u)){prev[u]=v;q[t++]=u;}}
 assert(prev[n]!=-2);for(int v=n;v>=0;v=prev[v]){c.visited[v]=1;c.sector_jumps++;}c.current=c.selected=n;}
static int count_bits(unsigned v){int n=0;while(v){n+=v&1u;v>>=1;}return n;}
static void win_fight(void){assert(c.mode==CAM_COMBAT);c.game.enemy.hull=0;for(int t=0;t<600&&c.mode==CAM_COMBAT;t++)campaign_step(&c);}
static int linked_to(int n){for(int i=0;i<c.node_count;i++)if(i!=n&&campaign_linked(&c,i,n)&&i!=campaign_boss_node(&c)&&i!=c.node_count-1)return i;return -1;}
int main(void){int kinds[NODE_EXIT+1];
 for(unsigned seed=1;seed<=40;seed++){finale(seed);
  /* Route: 4..6 linked beacons ending at the base; the start is rebel-held. */
  assert(c.boss_path_len>=4&&c.boss_path_len<=CAM_BOSS_PATH_MAX&&c.boss_path[c.boss_path_len-1]==c.node_count-1&&!c.boss_loc&&!c.boss_move);
  for(int i=1;i<c.boss_path_len;i++)assert(campaign_linked(&c,c.boss_path[i-1],c.boss_path[i]));
  assert(campaign_node_rebel(&c,c.boss_path[0])&&!campaign_node_rebel(&c,c.node_count-1));
  memset(kinds,0,sizeof kinds);for(int i=0;i<c.node_count;i++)kinds[c.node_kind[i]]++;
  assert(kinds[NODE_STORE]==1&&kinds[NODE_REPAIR]==3&&kinds[NODE_FIGHT]>=6&&kinds[NODE_EXIT]==1&&kinds[NODE_START]==1);
  assert(!strcmp(campaign_node_name(&c,c.node_count-1),"Federation base"));
  /* Waiting: the flagship moves on every second update; after it reaches the
   * base, three more jumps destroy it while the player is elsewhere. */
  {int len=c.boss_path_len,waits=0;
   while(c.mode==CAM_MAP){int before=c.boss_loc;unsigned rebels=c.fleet_nodes;assert(campaign_wait(&c));waits++;
    if(c.mode!=CAM_MAP)break;
    if(before<len-1)assert(c.boss_loc==before+(waits%2==0));else assert(c.boss_loc==before);assert((c.fleet_nodes&rebels)==rebels);
    if(c.boss_loc==len-1)assert(campaign_base_turns_left(&c)==CAM_BASE_TURNS-c.base_arrived&&!c.boss_move);}
   assert(c.mode==CAM_DEFEAT&&waits==2*(len-1)+3&&strstr(c.text,"Federation base"));}
 }
 /* Rebel control spreads one or two beacons per update and never takes the base. */
 finale(7);{unsigned before=c.fleet_nodes;for(int i=0;i<4;i++)campaign_wait(&c);assert(count_bits(c.fleet_nodes)>count_bits(before));assert(!((c.fleet_nodes>>(c.node_count-1))&1u));}
 /* Interception: jumping onto the flagship's beacon starts phase 1. */
 finale(3);{int boss=campaign_boss_node(&c),from=linked_to(boss);assert(from>=0&&!c.boss_move);teleport(from);
  assert(campaign_jump(&c,boss)&&c.mode==CAM_COMBAT&&c.boss_phase==0&&c.game.enemy_kind==ENEMY_FLAGSHIP);
  win_fight();assert(c.mode==CAM_EVENT&&c.boss_resume_phase==1&&c.boss_move==1&&c.boss_phase==-1);campaign_continue(&c);assert(c.mode==CAM_MAP);
  /* Beaten away from the base, it jumps toward the base on the next update. */
  {int loc=c.boss_loc;assert(campaign_wait(&c));assert(c.boss_loc==loc+1&&!c.boss_reversed);}
  /* Save25 round trip mid-finale preserves the full route state. */
  assert(campaign_save(&c,"tests/final-sector.sav"));campaign_init_seed(&d,999);assert(campaign_load(&d,"tests/final-sector.sav"));
  assert(d.finale_policy==1&&d.boss_path_len==c.boss_path_len&&d.boss_loc==c.boss_loc&&d.boss_move==c.boss_move&&d.fleet_nodes==c.fleet_nodes&&d.fleet_pending==c.fleet_pending&&d.boss_resume_phase==1&&!memcmp(d.boss_path,c.boss_path,sizeof c.boss_path));
  {FILE*f=fopen("tests/final-sector.sav","rb");unsigned char h[12];assert(f&&fread(h,1,12,f)==12&&!fclose(f));assert(h[8]==32&&!h[9]);}
  remove("tests/final-sector.sav");remove("tests/final-sector.sav.bak");}
 /* Defending at the base: arrival there starts combat; a win turns the route
  * around for one beacon, then the flagship heads back to the base. */
 finale(11);{int base=c.node_count-1;teleport(base);c.base_greeted=1;
  int guard=0;while(c.mode==CAM_MAP&&guard++<40)campaign_wait(&c);
  assert(c.mode==CAM_COMBAT&&campaign_boss_node(&c)==base);win_fight();campaign_continue(&c);
  assert(c.boss_reversed&&c.boss_move&&!c.base_arrived&&campaign_boss_node(&c)==base);
  campaign_wait(&c);assert(c.mode==CAM_MAP&&campaign_boss_node(&c)!=base&&!c.boss_reversed&&!c.boss_move);
  campaign_wait(&c);assert(c.boss_move);campaign_wait(&c);assert(c.mode==CAM_COMBAT&&campaign_boss_node(&c)==base&&c.boss_phase==1);
  win_fight();campaign_continue(&c);guard=0;while(c.mode==CAM_MAP&&guard++<40)campaign_wait(&c);
  assert(c.mode==CAM_COMBAT&&c.boss_phase==2);win_fight();assert(c.mode==CAM_COMPLETE&&strstr(c.text,"Victory"));}
 /* Repair stations and the base's one-time supplies. */
 finale(5);{int rep=-1;for(int i=1;i<c.node_count-1;i++)if(c.node_kind[i]==NODE_REPAIR&&!campaign_node_rebel(&c,i)&&!c.boss_move){rep=i;break;}
  assert(rep>=0);c.fleet_pending&=~(1u<<rep);int from=linked_to(rep);c.current=c.selected=from>=0?from:0;
  c.game.player.hull=10;c.game.player.systems[SYS_ENGINES].health=0;int fuel=c.fuel,missiles=c.game.missiles,parts=c.game.drone_parts,scrap=c.scrap;
  if(campaign_linked(&c,c.current,rep)&&campaign_boss_node(&c)!=rep){assert(campaign_jump(&c,rep));
   if(c.mode==CAM_EVENT){assert(c.game.player.hull==25&&c.game.player.systems[SYS_ENGINES].health==c.game.player.systems[SYS_ENGINES].capacity&&c.fuel==fuel+4&&c.game.missiles==missiles+4&&c.game.drone_parts==parts+5&&c.scrap>=scrap+22&&c.scrap<=scrap+44);}}}
 finale(6);{int base=c.node_count-1,from=-1;for(int i=0;i<base;i++)if(campaign_linked(&c,i,base)&&i!=campaign_boss_node(&c))from=i;
  c.current=c.selected=from;c.fleet_nodes=c.fleet_pending=0;int hull=c.game.player.hull=15,fuel=c.fuel;
  if(campaign_boss_node(&c)!=base){assert(campaign_jump(&c,base));if(c.mode==CAM_EVENT){assert(c.base_greeted&&c.game.player.hull==hull+10&&c.fuel==fuel-1+10);campaign_continue(&c);
   c.current=c.selected=from;c.visited[from]=1;fuel=c.fuel;assert(campaign_jump(&c,base));if(c.mode==CAM_EVENT)assert(c.fuel==fuel-1);}}}
 /* Save24 migration: runs before the final sector adopt V41 on arrival; a run
  * already inside the final sector keeps its interim exit battle. */
 for(int inside=0;inside<2;inside++){
  campaign_init_seed(&c,inside?21:20);for(int i=1;i<c.sector_count-1;i++)c.sector_types[i]=SECTOR_REBEL;
  if(inside){c.sector=c.sector_count-2;c.next_sector[0]=SECTOR_FINAL;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0));}
  else {c.sector=3;c.sector_types[4]=0;c.sector_types[5]=0;c.sector_types[6]=0;}
  c.event_policy=5; /* save24 predates the V53 sector-list policy. */
  assert(campaign_save(&c,"tests/final-v24.sav"));
  static unsigned char raw[16384];FILE*f=fopen("tests/final-v24.sav","rb");size_t n=fread(raw,1,sizeof raw,f);fclose(f);
  n-=464;raw[8]=24;raw[12]=(unsigned char)((n-20)&255);raw[13]=(unsigned char)((n-20)>>8);
  {unsigned h=2166136261u;for(size_t i=20;i<n;i++)h=(h^raw[i])*16777619u;for(int i=0;i<4;i++)raw[16+i]=(unsigned char)(h>>(8*i));}
  f=fopen("tests/final-v24.sav","wb");fwrite(raw,1,n,f);fclose(f);
  assert(campaign_load(&d,"tests/final-v24.sav"));assert(d.finale_policy==!inside&&!d.boss_path_len&&!campaign_finale_active(&d));
  if(inside){d.mode=CAM_EVENT;d.event_id=EVENT_EXIT;d.event_resolved=0;assert(campaign_choice(&d,0)&&d.mode==CAM_COMBAT&&d.boss_phase==0);}
  remove("tests/final-v24.sav");remove("tests/final-v24.sav.bak");
 }
 /* Legacy six-sector runs keep the interim exit battle. */
 campaign_init_legacy_seed(&c,3);assert(!c.finale_policy&&!campaign_finale_active(&c));
 puts("PASS: V41 final sector route, cadence, countdown/base loss, interception, reversal, rebel spread, stations, base supplies, victory and save25.");return 0;
}
