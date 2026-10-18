#include "campaign.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int root(const char*id){for(int i=0;i<campaign_imported_count();i++)if(!strcmp(id,campaign_imported_id(i)))return i;assert(0);return -1;}
static int policy;
static void init_seed(Campaign*c,unsigned seed){campaign_init_seed(c,seed);assert(c->event_policy==6);c->event_policy=policy;}
static void init(Campaign*c){init_seed(c,25);}
static void economy(void){
 Campaign c,b;char hint[96];init(&c);int r=root("REFUEL_STATION");assert(campaign_imported_begin(&c,r));assert(campaign_choice_count(&c)==4);
 b=c;for(int i=0;i<20;i++){assert(campaign_choice_enabled(&c,0));campaign_choice_name(&c,0);campaign_choice_hint(&c,0,hint,sizeof hint);}assert(!memcmp(&c,&b,sizeof c));assert(strstr(hint,"+6")&&strstr(hint,"-12"));
 assert(campaign_choice(&c,0)&&c.fuel==18&&c.scrap==8&&c.event_resolved);assert(strstr(campaign_text(&c),"fuel +6")&&strstr(campaign_text(&c),"scrap -12"));b=c;assert(!campaign_choice(&c,0)&&!memcmp(&c,&b,sizeof c));campaign_continue(&c);assert(c.mode==CAM_MAP&&c.imported_node==-1);b=c;assert(!campaign_imported_begin(&c,r)&&!memcmp(&c,&b,sizeof c));
 init(&c);c.scrap=1;assert(campaign_imported_begin(&c,r));b=c;for(int i=0;i<3;i++){assert(!campaign_choice_enabled(&c,i)&&!campaign_choice(&c,i));}assert(!memcmp(&c,&b,sizeof c));assert(campaign_choice(&c,3));
 init(&c);c.scrap=40;c.game.player.hull=20;assert(campaign_imported_begin(&c,root("REPAIR_STATION")));assert(campaign_choice(&c,0)&&c.scrap==0&&c.game.player.hull==30);assert(strstr(campaign_text(&c),"hull +10"));
 init(&c);c.game.missiles=9;assert(campaign_imported_begin(&c,root("SELL_MISSILES_STATION")));assert(campaign_choice_enabled(&c,0)&&!campaign_choice_enabled(&c,1));assert(campaign_choice(&c,0)&&c.game.missiles==4&&c.scrap==35);
 init(&c);c.game.drone_parts=6;assert(campaign_imported_begin(&c,root("SELL_DRONES_STATION")));assert(campaign_choice(&c,1)&&c.game.drone_parts==0&&c.scrap==44);
 init(&c);c.scrap=1000000;c.game.missiles=20;assert(campaign_imported_begin(&c,root("SELL_MISSILES_STATION")));b=c;assert(!campaign_choice(&c,0)&&!memcmp(&c,&b,sizeof c));assert(campaign_choice(&c,3));
}
static void random_effects(void){int costs=0,gains=0;for(unsigned seed=1;seed<=128;seed++){
 Campaign c,d,b;init_seed(&c,seed);c.fuel=4;assert(campaign_imported_begin(&c,root("FUEL_FOR_DRONE")));d=c;b=c;
 for(int i=0;i<10;i++){assert(campaign_choice_enabled(&c,0));}assert(!memcmp(&c,&b,sizeof c));
 assert(campaign_choice(&c,0)&&campaign_choice(&d,0)&&!memcmp(&c,&d,sizeof c));assert(c.fuel>=0&&c.fuel<=2&&c.game.drone_parts>=3&&c.game.drone_parts<=5);
 costs|=1<<(4-c.fuel);gains|=1<<c.game.drone_parts;
 init_seed(&c,seed);c.fuel=3;assert(campaign_imported_begin(&c,root("FUEL_FOR_DRONE")));b=c;assert(!campaign_choice(&c,0)&&!memcmp(&c,&b,sizeof c));
 }assert(costs==28&&gains==(14<<2));
}
static void species_and_wait(void){
 Campaign c,b;init(&c);c.fuel=0;int r=root("NO_FUEL_REFUGEE_DAMAGED");assert(campaign_wait(&c)&&c.sector_jumps==1&&c.event_id==EVENT_IMPORTED);assert(campaign_choice_blue(&c,1)&&!campaign_choice_enabled(&c,1));b=c;assert(!campaign_choice(&c,1)&&!memcmp(&c,&b,sizeof c));
 assert(game_set_crew_species(&c.game,0,0,SPEC_ENGI));assert(campaign_choice_enabled(&c,1));c.game.crew[0].ship=1;assert(!campaign_choice_enabled(&c,1));c.game.crew[0].ship=0;c.game.crew[0].health=0;assert(!campaign_choice_enabled(&c,1));c.game.crew[0].health=100;
 assert(campaign_choice(&c,1)&&!c.event_resolved&&campaign_choice_count(&c)==2&&c.fuel==0&&c.scrap==20);assert(campaign_choice(&c,0)&&c.fuel==6&&c.scrap==10&&c.event_resolved);assert(c.event_seen&(1<<r));
 init(&c);b=c;assert(!campaign_imported_begin(&c,r)&&!memcmp(&b,&c,sizeof c));c.fuel=0;c.sector_jumps=8;assert(campaign_wait(&c)&&c.mode==CAM_COMBAT&&c.event_seen==0); /* Fleet takes precedence. */
 int terminal=0,nested=0;for(unsigned seed=1;seed<=64;seed++){init_seed(&c,seed);c.fuel=0;assert(campaign_imported_begin(&c,r));assert(campaign_choice(&c,2));if(c.event_resolved)terminal++;else{nested++;assert(campaign_choice_count(&c)==2&&campaign_choice(&c,0)&&c.fuel==6&&c.scrap==10);}}
 assert(terminal&&nested);
 init(&c);c.fuel=0;assert(campaign_imported_begin(&c,r));assert(campaign_choice(&c,3)&&c.fuel>=3&&c.fuel<=6&&c.scrap>=35&&c.scrap<=45);
}
static void placement(void){
 Campaign c,b;init(&c);c.node_kind[1]=NODE_SUPPLY;assert(campaign_jump(&c,1)&&c.event_id==EVENT_IMPORTED);int mask=c.event_seen;assert(mask);assert(campaign_choice(&c,campaign_choice_count(&c)-1));while(c.mode!=CAM_MAP){if(campaign_choice_count(&c))assert(campaign_choice(&c,campaign_choice_count(&c)-1));else campaign_continue(&c);}assert(campaign_jump(&c,0));campaign_continue(&c);assert(campaign_jump(&c,1)&&c.event_id==EVENT_AFTERMATH&&c.event_seen==mask);
 init(&c);c.node_kind[1]=NODE_SUPPLY;c.sector_jumps=30;assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT&&!c.event_seen);
 init(&c);assert(campaign_jump(&c,2)&&c.event_id==EVENT_SUPPLY&&!c.event_seen); /* Guaranteed fuel anchor preserved. */
 campaign_init_legacy(&c);b=c;assert(!campaign_imported_begin(&c,0)&&!memcmp(&b,&c,sizeof c));c.fuel=0;assert(campaign_wait(&c)&&c.mode==CAM_MAP);
 init(&c);b=c;assert(!campaign_imported_begin(&c,-1)&&!campaign_imported_begin(&c,campaign_imported_count())&&!memcmp(&c,&b,sizeof c));
}
static void policy_guards(void){
 Campaign c,b;init(&c);assert(campaign_imported_count()>=12);
 const char*ids[]={"REFUEL_STATION","REPAIR_STATION","SELL_MISSILES_STATION","SELL_DRONES_STATION","FUEL_FOR_DRONE","NO_FUEL_REFUGEE_DAMAGED","AUTO_REFUEL_STATION","SQUAT_REFUEL_STATION"};
 for(int i=0;i<8;i++)assert(!strcmp(campaign_imported_id(i),ids[i]));
 if(policy==1)for(int i=6;i<8;i++){b=c;assert(!campaign_imported_begin(&c,i)&&!memcmp(&c,&b,sizeof c));}
}
int main(void){for(policy=1;policy<=2;policy++){economy();random_effects();species_and_wait();placement();policy_guards();}puts("Original events under policies 1 and 2: stable root IDs, legacy combat-root exclusion, exact trades, bounded random effects, blue living/aboard Engi chains, waiting/fleet priority, unique/revisit/legacy guards and pure cost queries passed.");return 0;}
