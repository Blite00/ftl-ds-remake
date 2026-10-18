/* V62: event stores, crew loss, boarders, system status limits and taller choice
 * boxes for long original labels. */
#include "campaign.h"
#include "subsystems.h"
#include "species.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c;
static Game g;
static int find_node(int kind){for(int i=0;i<IMPORTED_EVENT_NODE_COUNT;i++){const ImportedEventNode*n=&imported_event_nodes[i];if(n->ship>=0||n->hostile>=0||n->alt_count)continue;
 for(int k=0;k<n->effect_count;k++)if(imported_event_effects[n->first_effect+k].kind==kind)return i;}return -1;}
/* Play a compiled node as a quest beacon reached by a jump. */
static int play(int node,unsigned seed){campaign_init_seed(&c,seed);c.mode=CAM_MAP;c.scrap=200;c.fuel=10;
 int n=-1;for(int i=1;i<c.node_count-1&&n<0;i++)if(campaign_linked(&c,c.current,i)&&!c.visited[i]&&c.node_kind[i]!=NODE_STORE)n=i;
 if(n<0||!campaign_quest_add(&c,node))return 0;
 for(int q=0;q<CAM_QUESTS;q++)if(c.quest_event[q]==node){if(c.quest_node[q]>=0)c.node_kind[c.quest_node[q]]=NODE_FIGHT;c.quest_node[q]=n;c.node_kind[n]=NODE_QUEST;}
 return campaign_jump(&c,n)&&c.event_id==EVENT_IMPORTED;}
int main(void){
 int counts[17]={0};for(int i=0;i<IMPORTED_EVENT_EFFECT_COUNT;i++){int k=imported_event_effects[i].kind;assert(k>=0&&k<=IE_SECRET_SECTOR);if(k<17)counts[k]++;}
 assert(counts[IE_STORE]&&counts[IE_REMOVECREW]&&counts[IE_BOARDERS]&&counts[IE_STATUS]);
 /* Status: limit, divide, loss and clear on main systems; caps on sensors; enemy trimming. */
 game_init(&g);assert(game_begin_encounter(&g,ENEMY_REBEL,1,0));g.player.reactor=20;while(game_adjust_power(&g,SYS_SHIELDS,1)){}
 assert(g.player.systems[SYS_SHIELDS].power==2);game_apply_status(&g,0,0,SYS_SHIELDS,1);assert(g.player.systems[SYS_SHIELDS].power==1&&!game_adjust_power(&g,SYS_SHIELDS,1));
 game_apply_status(&g,3,0,SYS_SHIELDS,0);assert(game_adjust_power(&g,SYS_SHIELDS,1));
 g.player.systems[SYS_WEAPONS].capacity=g.player.systems[SYS_WEAPONS].health=4;while(game_adjust_power(&g,SYS_WEAPONS,1)){}
 game_apply_status(&g,1,0,SYS_WEAPONS,2);assert(g.player.systems[SYS_WEAPONS].power<=2);
 game_apply_status(&g,2,0,SYS_ENGINES,2);assert(g.player.systems[SYS_ENGINES].power==0);
 g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;game_apply_status(&g,0,0,6,0);assert(subsystem_level(&g,0,SUB_SENSORS)==0);
 {int before=g.enemy.systems[SYS_ENGINES].power;game_apply_status(&g,0,1,SYS_ENGINES,0);assert(g.enemy.systems[SYS_ENGINES].power==0||before==0);}
 /* A jump clears every status. */
 campaign_init_seed(&c,6201);game_apply_status(&c.game,0,0,SYS_SHIELDS,0);assert(c.game.status_cap[0][SYS_SHIELDS]==0);
 c.mode=CAM_MAP;for(int n=0;n<c.node_count;n++)if(campaign_can_jump(&c,n)){assert(campaign_jump(&c,n));break;}assert(c.game.status_cap[0][SYS_SHIELDS]==-1);
 /* Boarders aboard with no ship: fought during ship management. */
 game_init(&g);g.enemy.hull=0;g.outcome=GAME_VICTORY;assert(game_spawn_intruders(&g,2,SPEC_HUMAN)==2);
 {int aboard=0;for(int i=0;i<g.enemy_crew_count;i++){aboard+=g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0&&g.enemy_crew[i].species==SPEC_HUMAN;}assert(aboard==2);}
 for(int i=0;i<g.crew_count;i++){game_order_crew(&g,i,g.enemy_crew[0].room);}for(int t=0;t<90*GAME_HZ;t++){game_service_step(&g);}
 {int aboard=0;for(int i=0;i<g.enemy_crew_count;i++){aboard+=g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==0;}assert(aboard<2);}
 /* Original events: a store opens after its event, a crewmember leaves, boarders beam in. */
 int store=find_node(IE_STORE),loss=find_node(IE_REMOVECREW),board=find_node(IE_BOARDERS);
 if(store>=0){int ok=0;for(unsigned s=6210;s<6230&&!ok;s++)if(play(store,s)){int here=c.current;while(c.mode==CAM_EVENT&&!c.event_resolved&&campaign_choice_count(&c)){campaign_choice(&c,0);}
   if(c.mode==CAM_EVENT){campaign_continue(&c);ok=c.mode==CAM_STORE&&c.node_kind[here]==NODE_STORE;}}assert(ok);}
 if(loss>=0){int ok=0;for(unsigned s=6230;s<6250&&!ok;s++){campaign_init_seed(&c,s);int crew=c.game.crew_count;if(play(loss,s))ok=c.game.crew_count==crew-1;}assert(ok);}
 if(board>=0){int ok=0;for(unsigned s=6250;s<6270&&!ok;s++)if(play(board,s)){for(int i=0;i<c.game.enemy_crew_count;i++){ok|=c.game.enemy_crew[i].health>0&&c.game.enemy_crew[i].ship==0;}}assert(ok);}
 printf("PASS: event status limits, boarders (%d), stores (%d), crew loss (%d), status effects (%d); nodes store %d loss %d boarders %d.\n",counts[IE_BOARDERS],counts[IE_STORE],counts[IE_REMOVECREW],counts[IE_STATUS],store,loss,board);
 return 0;
}
