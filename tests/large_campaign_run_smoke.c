/* Ordinary-action eight-sector pilots. Legacy helper functions supply combat,
 * servicing and economy; this driver chooses routes from the actual new graph. */
#include "campaign.h"
static int branch_side, branch_picks[2], types_seen;
static int imported_choices, imported_transactions, original_seen;
static int original_battles, original_hull_wins, original_crew_wins, original_escapes, original_ship_mask, original_enemy_mask;
static int route_choice(Campaign *c,int n);
static int pilot_fire(Game*g,int weapon,int room);
#define game_fire pilot_fire
#define campaign_choice route_choice
#define settle legacy_settle
#define main legacy_campaign_driver_main
#include "campaign_v10_smoke.c"
#undef main
#undef settle
#undef campaign_choice
#undef game_fire
/* The second branch pilot deliberately lets warning scouts escape to exercise
 * pursuit consequences in an otherwise ordinarily earned complete campaign. */
static int pilot_fire(Game*g,int weapon,int room){return branch_side==1&&g->enemy_escape_total?0:game_fire(g,weapon,room);}
static int route_choice(Campaign*c,int n){
 int type=0,branch=-1;
 if(c->event_id==EVENT_IMPORTED&&c->mode==CAM_EVENT&&!c->event_resolved){
  int count=campaign_choice_count(c);n=c->imported_ship>=0?0:count-1;
  for(int i=0;i<count;i++)if(campaign_choice_enabled(c,i)){
   const char *label=campaign_choice_name(c,i);
   if((c->fuel<12&&(strstr(label,"Buy 6 fuel")||strstr(label,"Trade some scrap")||strstr(label,"Accept it.")))||
      (c->game.player.hull<=20&&strstr(label,"Repair 10"))||
      (c->game.missiles>=11&&strstr(label,"Sell 5 missiles"))||
      (c->game.drone_parts>=3&&strstr(label,"Sell 3 drone"))){n=i;break;}
  }
  if(branch_side==1)for(int i=0;i<count;i++)if(campaign_choice_enabled(c,i)&&!strcmp(campaign_choice_name(c,i),"Aid the civilian ship.")){n=i;break;}
  if(!campaign_choice_enabled(c,n))for(int i=0;i<count;i++)if(campaign_choice_enabled(c,i)){n=i;break;}
 }
 if(c->event_id==EVENT_EXIT&&c->mode==CAM_EVENT&&!c->event_resolved&&campaign_route_count(c)){
  if(campaign_route_count(c)==2){n=branch_side;branch=n;}
  type=c->next_sector[n];
 }
 if(c->event_id==EVENT_EXIT&&c->sector==7)fprintf(stderr,"Final loadout seed%u branch%d hull%d scrap%d ammo%d reactor%d weapons%d ids %d %d %d / power%d\n",c->seed,branch_side,c->game.player.hull,c->scrap,c->game.missiles,c->game.player.reactor,c->game.weapon_count,c->game.weapons[0].catalog_id,c->game.weapons[1].catalog_id,c->game.weapons[2].catalog_id,c->game.player.systems[SYS_WEAPONS].power);
 int was_imported=c->mode==CAM_EVENT&&c->event_id==EVENT_IMPORTED&&!c->event_resolved;
 int fuel=c->fuel,scrap=c->scrap,ammo=c->game.missiles,parts=c->game.drone_parts,hull=c->game.player.hull;
 int ok=campaign_choice(c,n);
 if(ok&&was_imported){
  imported_choices++;original_seen|=c->event_seen;
  int spent=(c->fuel<fuel)+(c->scrap<scrap)+(c->game.missiles<ammo)+(c->game.drone_parts<parts);
  int gained=(c->fuel>fuel)+(c->scrap>scrap)+(c->game.missiles>ammo)+(c->game.drone_parts>parts)+(c->game.player.hull>hull);
  if(spent&&gained)imported_transactions++;
 }
 if(ok&&type){assert(c->sector_types[c->sector]==type);types_seen|=1<<type;if(branch>=0)branch_picks[branch]++;}
 return ok;
}
/* New-campaign pilot buys equipment and upgrades incrementally. This avoids
 * requiring the complete weapon/reactor bundle in one store visit. */
static void prepare_power(Campaign*c){
 int need=0;for(int i=0;i<c->game.weapon_count;i++)need+=c->game.weapons[i].power;
 if(c->game.player.systems[SYS_WEAPONS].power<need){
  while(c->game.player.systems[SYS_WEAPONS].power<need&&c->game.player.systems[SYS_ENGINES].power>0){
   game_adjust_power(&c->game,SYS_ENGINES,-1);if(!game_adjust_power(&c->game,SYS_WEAPONS,1))break;
  }
 }
}
static void settle(Campaign*c){
 while(c->reward.kind||(c->mode!=CAM_MAP&&c->mode!=CAM_COMPLETE&&c->mode!=CAM_DEFEAT)){
  if(c->reward.kind){resolve_rewards(c);continue;}
  if(c->mode==CAM_COMBAT){
   int ship=c->imported_ship,kind=c->game.enemy_kind;prepare_power(c);fight(c);
   if(ship>=0&&c->game.outcome!=GAME_PLAYING){assert(ship<4);original_battles++;original_ship_mask|=1<<ship;original_enemy_mask|=1<<kind;
    if(c->game.result_reason==RESULT_ENEMY_HULL)original_hull_wins++;
    else if(c->game.result_reason==RESULT_ENEMY_CREW)original_crew_wins++;
    else if(c->game.result_reason==RESULT_ENEMY_ESCAPE)original_escapes++;
    else assert(0);
    printf("Original combat: seed%u branch%d sector%d ship%d enemy%d reason%d hull%d\n",c->seed,branch_side,c->sector,ship,kind,c->game.result_reason,c->game.player.hull);
   }
  }
  else if(c->mode==CAM_STORE){
   int cargo_slot=free_cargo(c);
   while(c->game.player.hull<=25&&campaign_purchase(c,2)){}
   while(c->fuel<8&&campaign_purchase(c,0)){}
   while(c->game.missiles<(c->sector>=5?12:6)&&campaign_purchase(c,1)){}
   if(cargo_slot>=0&&c->game.weapon_count==2&&campaign_upgrade_price(c,SYS_WEAPONS)>=0&&c->scrap>=25+campaign_upgrade_price(c,SYS_WEAPONS)&&campaign_purchase(c,4)){
    assert(campaign_upgrade(c,SYS_WEAPONS)&&campaign_swap(c,cargo_slot,2));
   }
   cargo_slot=free_cargo(c);
   if(cargo_slot>=0&&c->game.weapon_count==3&&c->game.weapons[2].catalog_id==2&&c->scrap>=60&&campaign_purchase(c,6)){
    assert(campaign_swap(c,cargo_slot,2));assert(campaign_sell_weapon(c,0,cargo_slot));
   }
   int need=0;for(int i=0;i<c->game.weapon_count;i++)need+=c->game.weapons[i].power;
   while(c->game.player.systems[SYS_WEAPONS].capacity<need&&campaign_upgrade(c,SYS_WEAPONS)){}
   if(c->game.player.systems[SYS_WEAPONS].capacity>=need&&c->game.player.reactor<need+4)campaign_upgrade(c,-1);
   campaign_continue(c);
  }else if(campaign_choice_count(c)){
   int id=c->event_id,n=0;
   if(id==EVENT_HAZARD)n=2;
   if(id==EVENT_DISTRESS&&c->fuel<5)n=1;
   if(id==EVENT_REPAIR&&c->game.player.hull<20&&campaign_choice_enabled(c,1))n=1;
   assert(route_choice(c,n));
  }else{service(c);prepare_power(c);campaign_continue(c);}
 }
 invest(c); /* Shared late-game shield investment (campaign_v10_smoke.c). */
}
#include "save.h"
static int coldloads;
static void coldload(Campaign*c){resolve_rewards(c);Campaign d;assert(c->mode==CAM_MAP);assert(campaign_save(c,"tests/large-run.sav"));FILE*f=fopen("tests/large-run.sav","rb");unsigned char header[12];assert(f&&fread(header,1,sizeof header,f)==sizeof header&&!fclose(f));assert(header[8]==32&&!header[9]&&!header[10]&&!header[11]);campaign_init_seed(&d,999);assert(campaign_load(&d,"tests/large-run.sav"));
 assert(d.event_policy==c->event_policy&&d.event_seen==c->event_seen&&d.imported_node==-1&&d.imported_ship==-1&&d.imported_battle==0);
 assert(d.route_policy==c->route_policy&&!memcmp(d.sector_types,c->sector_types,sizeof c->sector_types)&&!memcmp(d.next_sector,c->next_sector,sizeof c->next_sector));
 assert(d.node_count==c->node_count&&d.sector_count==c->sector_count&&d.sector==c->sector&&d.current==c->current&&d.selected==c->selected);
 assert(d.rng==c->rng&&d.game.rng==c->game.rng&&d.scrap==c->scrap&&d.fuel==c->fuel&&d.sector_jumps==c->sector_jumps);
 assert(!memcmp(c->links,d.links,sizeof c->links)&&!memcmp(c->visited,d.visited,sizeof c->visited)&&!memcmp(c->stock,d.stock,sizeof c->stock));
 assert(!memcmp(c->node_kind,d.node_kind,sizeof c->node_kind)&&!memcmp(c->node_y,d.node_y,sizeof c->node_y));
 assert(d.game.player.hull==c->game.player.hull&&d.game.weapon_count==c->game.weapon_count);*c=d;coldloads++;
}
static int next_step(const Campaign*c,int target){int dist[CAM_NODE_COUNT],prev[CAM_NODE_COUNT],used[CAM_NODE_COUNT]={0};
 for(int i=0;i<c->node_count;i++){dist[i]=100000;prev[i]=-1;}dist[c->current]=0;
 for(int pass=0;pass<c->node_count;pass++){int a=-1;for(int i=0;i<c->node_count;i++)if(!used[i]&&(a<0||dist[i]<dist[a]))a=i;if(a<0||dist[a]>=100000)break;used[a]=1;
  for(int b=0;b<c->node_count;b++)if(campaign_linked(c,a,b)){int cost=1+(c->node_kind[b]==NODE_FIGHT?2:0)+(c->visited[b]?1:0)+(campaign_node_captured(c,b,1)?100:0)+(campaign_node_rebel(c,b)&&b!=campaign_boss_node(c)?3:0);if(dist[a]+cost<dist[b]){dist[b]=dist[a]+cost;prev[b]=a;}}
 }
 assert(prev[target]>=0);while(prev[target]!=c->current){target=prev[target];assert(target>=0);}return target;
}
static int destination(const Campaign*c){int visits=0,best=-1,bx=10000,x=campaign_node_x(c,c->current);
 /* V41 final sector: intercept the moving flagship instead of heading for the base. */
 if(campaign_finale_active(c)&&campaign_boss_node(c)>=0&&campaign_boss_node(c)!=c->current)return campaign_boss_node(c);
 for(int i=0;i<c->node_count;i++)if(c->node_kind[i]==NODE_STORE&&c->visited[i])visits++;
 if(visits<2)for(int i=0;i<c->node_count;i++)if(!c->visited[i]&&c->node_kind[i]==NODE_STORE&&campaign_node_x(c,i)>x&&!campaign_node_captured(c,i,1)&&campaign_node_x(c,i)<bx){best=i;bx=campaign_node_x(c,i);}
 if(best>=0)return best;
 if(c->game.player.hull<25)for(int i=0;i<c->node_count;i++)if(!c->visited[i]&&c->node_kind[i]==NODE_REPAIR&&campaign_node_x(c,i)>x&&!campaign_node_captured(c,i,1))return i;
 return c->node_count-1;
}
int main(void){setvbuf(stdout,NULL,_IONBF,0);unsigned seeds[]={1,42,8675309};int runs=0,totaljumps=0;coldloads=0;
 for(branch_side=0;branch_side<2;branch_side++)for(unsigned k=0;k<sizeof seeds/sizeof seeds[0];k++){int choices_before=imported_choices,trades_before=imported_transactions;Campaign c;campaign_init_seed(&c,seeds[k]);c.event_policy=4;/* Historical policy4 earned-run coverage. */assert(c.node_count==23&&c.sector_count==8&&c.route_policy==1&&c.event_policy==4);types_seen|=1<<c.sector_types[0];coldload(&c);
  flagship_last_defeat=0;for(int stop=0;stop<200&&c.mode!=CAM_COMPLETE&&c.mode!=CAM_DEFEAT;stop++){
   assert(c.mode==CAM_MAP);if(!c.fuel){assert(campaign_rescue(&c));settle(&c);if(c.mode!=CAM_MAP)break;}
   if(campaign_finale_active(&c)&&campaign_boss_node(&c)==c.current){assert(campaign_wait(&c));settle(&c);if(c.mode==CAM_COMPLETE||c.mode==CAM_DEFEAT)break;assert(c.mode==CAM_MAP);continue;}
   int n=next_step(&c,destination(&c));assert(campaign_jump(&c,n));settle(&c);if(c.mode==CAM_COMPLETE||c.mode==CAM_DEFEAT)break;assert(c.mode==CAM_MAP);coldload(&c);
  }
  if((c.mode!=CAM_COMPLETE&&!flagship_last_defeat)||c.jumps<56)fprintf(stderr,"Large campaign failure seed%u mode%d sector%d jumps%d hull%d: %s\n",seeds[k],c.mode,c.sector,c.jumps,c.game.player.hull,c.text);
  /* Seven full sectors (>=8 jumps each) plus a V41 flagship interception. */
  assert(c.sector==7&&c.jumps>=56&&(c.mode==CAM_COMPLETE?c.boss_phase==2:(c.mode==CAM_DEFEAT&&flagship_last_defeat)));runs++;totaljumps+=c.jumps;
  printf("Imported progress: choices%d trades%d seen0x%x\n",imported_choices-choices_before,imported_transactions-trades_before,c.event_seen);
  printf("Eight-sector seed%u branch%d complete: jumps%d hull%d scrap%d\n",seeds[k],branch_side,c.jumps,c.game.player.hull,c.scrap);
  const char*paths[]={"tests/large-run.sav","tests/large-run.sav.bak","tests/large-run.sav.tmp","tests/large-run.sav.bad"};for(int i=0;i<4;i++){FILE*f=fopen(paths[i],"rb");if(f){fclose(f);assert(!remove(paths[i]));}}
 }
 assert(runs==6&&coldloads>=336&&branch_picks[0]==18&&branch_picks[1]==18);
 printf("Flagship (DS tuning): %d engagements, %d stage wins, %d run-ending defeats.\n",flagship_engagements,flagship_stage_wins,flagship_defeats);
 assert((types_seen&62)==62);assert(imported_choices>0&&imported_transactions>0&&original_seen);
 assert(original_escapes>0);assert(reward_offers[REWARD_WEAPON]>0&&reward_accepts[REWARD_WEAPON]>0&&reward_declines[REWARD_WEAPON]>0);
 assert(original_battles>0&&original_hull_wins+original_crew_wins+original_escapes==original_battles&&original_ship_mask==15);
 assert(original_enemy_mask==((1<<ENEMY_SCOUT)|(1<<ENEMY_ASSAULT)|(1<<ENEMY_REBEL)|(1<<ENEMY_REBEL_FAT)));
 printf("PASS: six earned branching eight-sector campaigns, %d jumps and %d disk coldloads; both branches and all sector types visited; %d imported choices, %d trades, seen mask0x%x.\n",totaljumps,coldloads,imported_choices,imported_transactions,original_seen);printf("Original combat coverage: %d battles, %d hull wins, %d crew wins, %d escapes, ship mask0x%x enemy mask0x%x.\n",original_battles,original_hull_wins,original_crew_wins,original_escapes,original_ship_mask,original_enemy_mask);reward_evidence();return 0;
}
