/* Real-action campaign evidence plus explicitly selected matchup fixtures.
 * The included fixture main is compiled but never invoked. No earned-run
 * wins, player health, money, ammunition, charge, or geometry are injected. */
#include "campaign.h"
static int profile_battles,missile_battles,enemy_ammo_spent,player_ammo_spent;
static void measured_step(Campaign*c){int before=c->game.enemy_missiles;campaign_step(c);if(before>c->game.enemy_missiles)enemy_ammo_spent+=before-c->game.enemy_missiles;}
static int measured_fire(Game*g,int weapon,int room){int before=g->missiles,result=game_fire(g,weapon,room);if(before>g->missiles)player_ammo_spent+=before-g->missiles;return result;}
#define game_fire measured_fire
#define campaign_step measured_step
#define main uncalled_legacy_fixture_main
#define settle legacy_settle
#include "campaign_v10_smoke.c"
#undef settle
#undef main
#undef campaign_step
#undef game_fire
static int choose(Campaign*c,int n){
 if(c->event_id==EVENT_EXIT&&campaign_route_count(c)==2)for(int i=0;i<2;i++)if(c->next_sector[i]==SECTOR_REBEL)n=i;
 if(c->event_id==EVENT_IMPORTED){n=c->imported_ship>=0?0:campaign_choice_count(c)-1;
  for(int i=0;i<campaign_choice_count(c);i++)if(campaign_choice_enabled(c,i)){
   const char*label=campaign_choice_name(c,i);
   if((c->fuel<12&&(strstr(label,"Buy 6 fuel")||strstr(label,"Trade some scrap")||strstr(label,"Accept it.")))||(c->game.player.hull<=20&&strstr(label,"Repair 10"))||(c->game.missiles>=11&&strstr(label,"Sell 5 missiles"))||(c->game.drone_parts>=3&&strstr(label,"Sell 3 drone")))n=i;
  }
 }
 if(!campaign_choice_enabled(c,n))for(int i=0;i<campaign_choice_count(c);i++)if(campaign_choice_enabled(c,i)){n=i;break;}
 return campaign_choice(c,n);
}
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
   if(c->game.enemy_weapon_count){profile_battles++;if(c->game.enemy_weapon_secondary.missiles)missile_battles++;}
   prepare_power(c);fight(c);
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
   assert(choose(c,n));
  }else{service(c);prepare_power(c);campaign_continue(c);}
 }
 invest(c); /* Shared late-game shield investment (campaign_v10_smoke.c). */
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
 if(c->sector>=1&&c->game.player.hull>=25){int fought=0;for(int i=0;i<c->node_count;i++)if(c->visited[i]&&c->node_kind[i]==NODE_FIGHT)fought=1;
  if(!fought)for(int i=0;i<c->node_count;i++)if(!c->visited[i]&&c->node_kind[i]==NODE_FIGHT&&campaign_node_x(c,i)>x&&!campaign_node_captured(c,i,1))return i;
 }
 if(c->game.player.hull<25)for(int i=0;i<c->node_count;i++)if(!c->visited[i]&&c->node_kind[i]==NODE_REPAIR&&campaign_node_x(c,i)>x&&!campaign_node_captured(c,i,1))return i;
 return c->node_count-1;
}

static void comparison(void){
 const int pair[3][2]={{2,3},{2,1},{2,GAME_ENEMY_ARTEMIS}};
 const char*labels[]={"legacy","basic-heavy","basic-burst2","basic-enemy-artemis"};
 puts("fixture,policy,seeds,victories,defeats,timeouts,mean_hull_loss,mean_player_ammo,mean_enemy_ammo");
 for(int profile=-1;profile<3;profile++)for(int passive=0;passive<2;passive++){
  int wins=0,losses=0,timeouts=0,hull=0,ammo=0,enemy_ammo=0;
  for(unsigned seed=1;seed<=32;seed++){
   Campaign c;campaign_init_seed(&c,seed);assert(game_begin_encounter(&c.game,ENEMY_SCOUT,1,0));
   /* Fixed matchup provisioning is explicit here, not an earned upgrade.
    * Starting player ship and resources remain untouched. */
   if(profile>=0){int power=profile?3:2;c.game.enemy.systems[SYS_WEAPONS].capacity=c.game.enemy.systems[SYS_WEAPONS].health=c.game.enemy.systems[SYS_WEAPONS].power=power;assert(game_enemy_set_loadout(&c.game,pair[profile],2,10));}
   int start_hull=c.game.player.hull,start_ammo=c.game.missiles,start_enemy=c.game.enemy_missiles;
   for(int t=0;t<120*GAME_HZ&&!c.game.outcome;t++){
    if(!passive)for(int w=0;w<c.game.weapon_count;w++)if(game_weapon_powered(&c.game,w)&&c.game.weapons[w].charge_ticks>=c.game.weapons[w].cooldown_ticks&&!c.game.weapons[w].pending_shots)game_fire(&c.game,w,c.game.enemy.systems[SYS_WEAPONS].room);
    game_step(&c.game);
   }
   hull+=start_hull-c.game.player.hull;ammo+=start_ammo-c.game.missiles;enemy_ammo+=start_enemy-c.game.enemy_missiles;
   wins+=c.game.outcome==GAME_VICTORY;losses+=c.game.outcome==GAME_DEFEAT;timeouts+=c.game.outcome==GAME_PLAYING;
  }
  printf("%s,%s,32,%d,%d,%d,%.2f,%.2f,%.2f\n",labels[profile+1],passive?"passive":"ready-weapons",wins,losses,timeouts,hull/32.0,ammo/32.0,enemy_ammo/32.0);
 }
}
static void earned(unsigned seed){Campaign c;campaign_init_seed(&c,seed);c.event_policy=4;/* Preserve V31 pilot comparison. */int start=profile_battles,enemy=enemy_ammo_spent,player=player_ammo_spent;
 flagship_last_defeat=0;for(int stop=0;stop<200&&c.mode!=CAM_COMPLETE&&c.mode!=CAM_DEFEAT;stop++){
  assert(c.mode==CAM_MAP);if(!c.fuel){assert(campaign_rescue(&c));settle(&c);if(c.mode==CAM_DEFEAT)break;}
  if(campaign_finale_active(&c)&&campaign_boss_node(&c)==c.current){assert(campaign_wait(&c));settle(&c);if(c.mode==CAM_DEFEAT)break;continue;}
  assert(campaign_jump(&c,next_step(&c,destination(&c))));settle(&c);if(c.mode==CAM_DEFEAT){if(c.sector==7)flagship_last_defeat=1;break;}
 }
 if(!(c.sector==7&&(c.mode==CAM_COMPLETE?c.game.player.hull>0:(c.mode==CAM_DEFEAT&&flagship_last_defeat))))fprintf(stderr,"Loadout run failure seed%u sector%d mode%d hull%d: %s\n",seed,c.sector,c.mode,c.game.player.hull,c.text);
 assert(c.sector==7&&(c.mode==CAM_COMPLETE?c.game.player.hull>0:(c.mode==CAM_DEFEAT&&flagship_last_defeat)));
 printf("Earned loadout campaign seed%u: jumps%d hull%d scrap%d profile_battles%d player_ammo_consumed%d enemy_ammo_consumed%d\n",seed,c.jumps,c.game.player.hull,c.scrap,profile_battles-start,player_ammo_spent-player,enemy_ammo_spent-enemy);
}
int main(void){setvbuf(stdout,NULL,_IONBF,0);comparison();earned(1);earned(42);earned(8675309);assert(profile_battles>0&&missile_battles>0&&player_ammo_spent>0&&enemy_ammo_spent>0);printf("Earned totals: profiles%d missile_profiles%d enemy_ammo%d player_ammo%d\n",profile_battles,missile_battles,enemy_ammo_spent,player_ammo_spent);return 0;}
