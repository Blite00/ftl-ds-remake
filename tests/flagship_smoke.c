/* V42 authentic Rebel flagship: imported BOSS_1..3_NORMAL stages, artillery,
 * cloaking, StartStage crew rules, AI takeover, crew persistence, save26 and
 * winnability with a strong but attainable DS build (seeded statistics). */
#include "campaign.h"
#include "save.h"
#include "boarding.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g;
static Campaign c,d;
static void stage(int phase){game_init(&g);g.flagship_authentic=1;assert(game_begin_encounter(&g,ENEMY_FLAGSHIP,7,phase));assert(game_is_flagship(&g)&&g.flagship_stage==phase);}
static int alive_in(int room){int n=0;for(int i=0;i<g.enemy_crew_count&&i<GAME_MAX_ENEMY_CREW;i++)n+=g.enemy_crew[i].health>0&&g.enemy_crew[i].ship==1&&g.enemy_crew[i].room==room;return n;}
static int artillery_in(int room){for(int i=0;i<g.artillery_count;i++)if(g.artillery[i].room==room)return i;return -1;}
static int effects_kind(int kind){int n=0;for(int i=0;i<GAME_MAX_EFFECTS;i++)n+=g.effects[i].active&&g.effects[i].side==1&&g.effects[i].kind==kind;return n;}
/* Strong attainable build: reactor 18, 3 shield layers, engines 4, weapons 8
 * (Artemis, Burst II x2, Heavy Laser), six crew; fires whole volleys at the
 * missile room, then the laser room, then shields. */
static int strong_fight(unsigned seed){
 stage(0);g.rng^=seed*2654435761u;
 for(int i=g.crew_count;i<6;i++){game_init_crew(&g,0,i,g.player.rooms[0].id);g.crew_count++;}
 game_equip(&g,0,0);game_equip(&g,1,1);game_equip(&g,2,1);game_equip(&g,3,3);
 g.player.reactor=18;g.player.systems[SYS_SHIELDS].capacity=g.player.systems[SYS_SHIELDS].health=6;
 g.player.systems[SYS_ENGINES].capacity=g.player.systems[SYS_ENGINES].health=4;g.player.systems[SYS_WEAPONS].capacity=g.player.systems[SYS_WEAPONS].health=8;g.missiles=14;
 for(int k=0;k<GAME_SYSTEM_COUNT;k++)while(game_adjust_power(&g,k,-1)){}
 game_adjust_power(&g,SYS_OXYGEN,1);while(game_adjust_power(&g,SYS_WEAPONS,1)){}while(game_adjust_power(&g,SYS_SHIELDS,1)){}while(game_adjust_power(&g,SYS_ENGINES,1)){}
 for(int t=0;t<240*GAME_HZ&&g.outcome==GAME_PLAYING;t++){
  int aim=-1,ready=1;static const int order[]={1,0};
  for(int i=0;i<2&&aim<0;i++)for(int k=0;k<g.artillery_count;k++)if(g.artillery[k].weapon==order[i]&&g.artillery[k].health>0)aim=g.artillery[k].room;
  if(aim<0)aim=g.enemy.systems[SYS_SHIELDS].room;
  for(int w=0;w<g.weapon_count;w++)if(game_weapon_powered(&g,w)&&g.weapons[w].charge_ticks<g.weapons[w].cooldown_ticks)ready=0;
  if(ready)for(int w=0;w<g.weapon_count;w++)game_fire(&g,w,aim);
  /* Same crew handling as the campaign fixture: heal below 40 HP (powering the
   * medbay from engines/weapons if needed), one crew member repairs oxygen,
   * then shields, then weapons. */
  int repair=-1,healing=0,med=g.player.systems[SYS_MEDBAY].room;
  for(int i=0;i<g.crew_count;i++){Crew*cr=&g.crew[i];if(cr->health<=0)continue;int target=-1;
   if(cr->health<40||(cr->destination==med&&cr->health<95)){target=med;healing=1;}else if(repair<0)repair=i;
   if(target>=0&&cr->destination!=target)game_order_crew(&g,i,target);}
  if(healing&&!g.player.systems[SYS_MEDBAY].power){if(game_used_power(&g)>=g.player.reactor)game_adjust_power(&g,SYS_ENGINES,-1);if(game_used_power(&g)>=g.player.reactor)game_adjust_power(&g,SYS_WEAPONS,-1);game_adjust_power(&g,SYS_MEDBAY,1);}
  if(!healing&&g.player.systems[SYS_MEDBAY].power){game_adjust_power(&g,SYS_MEDBAY,-1);while(game_adjust_power(&g,SYS_WEAPONS,1)){}while(game_adjust_power(&g,SYS_ENGINES,1)){}}
  if(repair>=0){int target=-1;const GameSystemState*sy=g.player.systems;
   if(sy[SYS_OXYGEN].health<sy[SYS_OXYGEN].capacity)target=sy[SYS_OXYGEN].room;else if(sy[SYS_SHIELDS].health<sy[SYS_SHIELDS].capacity)target=sy[SYS_SHIELDS].room;else if(sy[SYS_WEAPONS].health<sy[SYS_WEAPONS].capacity)target=sy[SYS_WEAPONS].room;
   if(target>=0&&g.crew[repair].destination!=target)game_order_crew(&g,repair,target);}
  if(g.player.systems[SYS_OXYGEN].health>0&&!g.player.systems[SYS_OXYGEN].power)game_adjust_power(&g,SYS_OXYGEN,1);
  while(game_adjust_power(&g,SYS_SHIELDS,1)){}
  game_step(&g);
 }
 return g.outcome==GAME_VICTORY;
}
int main(void){
 /* Stage geometry and systems come from the imported normal blueprints. */
 static const int rooms[3]={19,15,11},hulls[3]={20,22,20},arts[3]={4,3,2};
 for(int p=0;p<3;p++){stage(p);assert(g.enemy.room_count==rooms[p]&&g.enemy.hull==hulls[p]&&g.enemy.max_hull==hulls[p]&&g.artillery_count==arts[p]);
  assert(g.enemy.systems[SYS_SHIELDS].capacity==8&&g.enemy.shields==4&&!g.enemy.automated&&g.enemy_pilot_room==0&&g.enemy.systems[SYS_WEAPONS].capacity==0);
  assert(artillery_in(9)>=0&&artillery_in(10)>=0&&g.artillery[artillery_in(9)].w.shots==3&&g.artillery[artillery_in(10)].w.missiles&&g.artillery[artillery_in(10)].w.shield_piercing==5);}
 stage(0);assert(g.enemy_cloak.capacity==2&&g.enemy_cloak.room==15&&g.subsystems[1][SUB_DOORS].capacity==3&&!g.teleporter[1].installed);
 /* Observed level 3 for the stage-1 laser/missile (blueprint 4; FTL wiki charge table). */
 assert(g.artillery[artillery_in(14)].w.beam_length>0&&g.artillery[artillery_in(18)].w.ion_damage==1&&g.artillery[artillery_in(9)].capacity==3&&g.artillery[artillery_in(18)].capacity==3);
 stage(2);assert(g.artillery[artillery_in(9)].capacity==4);stage(0);
 stage(1);assert(!g.enemy_cloak.capacity&&artillery_in(18)<0&&artillery_in(14)>=0);
 stage(2);assert(g.teleporter[1].installed&&g.teleporter[1].room==3&&g.enemy_board_mode==2&&artillery_in(14)<0&&g.enemy.systems[SYS_ENGINES].capacity==6);
 /* StartStage(1): one human in rooms 0..6, 9, 10, 14 and 18; gunners are isolated. */
 stage(0);assert(game_alive_enemy_crew(&g)==11);
 for(int r=0;r<19;r++)assert(alive_in(r)==((r<=6||r==9||r==10||r==14||r==18)?1:0));
 for(int i=0;i<11;i++)assert(g.enemy_crew[i].species==SPEC_HUMAN);
 assert(game_next_room(&g,1,9,0)<0&&game_next_room(&g,1,0,9)<0&&game_next_room(&g,1,0,5)>=0);
 /* Later stages reuse survivors per room; rooms past the smaller hull lose theirs. */
 {int counts[GAME_FLAGSHIP_ROOMS];game_flagship_crew_counts(&g,counts);counts[2]=0;counts[18]=1;stage(1);assert(game_flagship_set_crew(&g,counts)==9&&!alive_in(2)&&alive_in(9)==1);
  memset(counts,0,sizeof counts);assert(game_flagship_set_crew(&g,counts)==0&&g.enemy.automated);counts[0]=5;assert(game_flagship_set_crew(&g,counts)<0);}
 /* Artillery: cooldown x(1.5-0.25*power); the level-3 laser fires after 15 s. */
 stage(0);g.enemy_cloak.capacity=0;{int a=artillery_in(9),t=0;g.artillery[a].charge=0;
  while(!g.artillery[a].w.pending_shots&&!g.artillery[a].w.queued_target_count&&t<40*GAME_HZ){game_step(&g);t++;}
  assert(t>=15*GAME_HZ-2&&t<=15*GAME_HZ+2);
  int plan[GAME_MAX_VOLLEY];memcpy(plan,g.artillery[a].w.queued_targets,sizeof plan);assert(g.artillery[a].w.queued_target_count==3&&plan[0]!=plan[1]&&plan[1]!=plan[2]&&plan[0]!=plan[2]);}
 stage(0);g.enemy_cloak.capacity=0;{int a=artillery_in(9),t=0;g.artillery[a].capacity=g.artillery[a].health=2;g.artillery[a].power=2; /* A full 2-bar room: the gunner has nothing to repair. */
 
  while(!g.artillery[a].w.queued_target_count&&t<60*GAME_HZ){game_step(&g);t++;}assert(t>=20*GAME_HZ-2&&t<=20*GAME_HZ+2);}
 stage(0);g.enemy_cloak.capacity=0;{int seen_ion=0,seen_beam=0,seen_missile=0;for(int t=0;t<40*GAME_HZ;t++){game_step(&g);seen_ion|=effects_kind(EFFECT_ION)>0;seen_beam|=effects_kind(EFFECT_BEAM)>0;seen_missile|=effects_kind(EFFECT_MISSILE)>0;}assert(seen_ion&&seen_beam&&seen_missile);}
 /* Damage, ion and sabotage reach artillery rooms. */
 stage(0);{int a=artillery_in(10);GameDamage hit={0,2,0,0,0,0,0,0};g.enemy.shields=0;g.enemy.systems[SYS_SHIELDS].power=0;g.enemy_cloak.capacity=0;
  assert(game_launch_effect(&g,0,10,EFFECT_MISSILE,&hit,5));for(int t=0;t<20;t++){game_step(&g);}assert(g.artillery[a].health<=2);
  assert(game_launch_ion(&g,0,9,2));for(int t=0;t<20;t++){game_step(&g);}assert(g.artillery[artillery_in(9)].ion_bars>0||g.enemy.shields>0);}
 /* Cloak: 10 s at two bars, +60 evasion, player charging paused and firing refused; then 20 s lockout. */
 stage(0);game_step(&g);assert(game_enemy_cloaked(&g)&&g.enemy_cloak.active_ticks>9*GAME_HZ);
 {int charge=g.weapons[0].charge_ticks;for(int t=0;t<GAME_HZ;t++){game_step(&g);}assert(g.weapons[0].charge_ticks==charge);g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;assert(!game_fire(&g,0,0));
  int misses=0;g.enemy.shields=0;for(int i=0;i<40;i++){GameDamage hit={0,0,0,0,0,0,0,0};assert(game_launch_effect(&g,0,0,EFFECT_LASER,&hit,0));for(int t=0;t<12;t++)game_step(&g);for(int k=0;k<GAME_MAX_EFFECTS;k++)if(g.effects[k].side==0&&g.effects[k].result==EFFECT_MISS){misses++;g.effects[k].active=0;}if(!game_enemy_cloaked(&g))break;}
  assert(misses>0);}
 while(game_enemy_cloaked(&g)){game_step(&g);}
 assert(g.enemy_cloak.ion_bars==4);for(int t=0;t<15*GAME_HZ;t++){game_step(&g);assert(!game_enemy_cloaked(&g));}
 for(int t=0;t<6*GAME_HZ&&!game_enemy_cloaked(&g);t++){game_step(&g);}
 assert(game_enemy_cloaked(&g));
 /* All crew dead: the AI takes control; no crew victory, and repairs follow. */
 stage(0);g.enemy_cloak.capacity=0;for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)g.enemy_crew[i].health=0;g.artillery[0].health=1;game_step(&g);
 assert(g.outcome==GAME_PLAYING&&g.enemy.automated&&strstr(g.message,"AI"));for(int t=0;t<13*GAME_HZ;t++){game_step(&g);}
 assert(g.artillery[0].health==2&&g.outcome==GAME_PLAYING);
 /* Campaign: stage reward, crew persistence and save26. */
 campaign_init_seed(&c,77);for(int i=1;i<c.sector_count-1;i++)c.sector_types[i]=SECTOR_REBEL;c.sector=c.sector_count-2;c.next_sector[0]=SECTOR_FINAL;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;
 assert(campaign_choice(&c,0)&&campaign_finale_active(&c));
 {int boss=campaign_boss_node(&c),from=-1;for(int i=0;i<c.node_count&&from<0;i++)if(campaign_linked(&c,i,boss)&&i!=c.node_count-1)from=i;
  int prev[CAM_NODE_COUNT],q[CAM_NODE_COUNT],h=0,t=0;for(int i=0;i<c.node_count;i++)prev[i]=-2;prev[0]=-1;q[t++]=0;while(h<t){int v=q[h++];for(int u=0;u<c.node_count;u++)if(prev[u]==-2&&campaign_linked(&c,v,u)){prev[u]=v;q[t++]=u;}}
  for(int v=from;v>=0;v=prev[v]){c.visited[v]=1;c.sector_jumps++;}
  c.current=c.selected=from;c.fuel=20;c.boss_move=0;c.fleet_nodes&=~(1u<<boss);
  assert(campaign_jump(&c,boss)&&c.mode==CAM_COMBAT&&game_is_flagship(&c.game)&&game_alive_enemy_crew(&c.game)==11);
  for(int i=0;i<GAME_MAX_ENEMY_CREW;i++)if(c.game.enemy_crew[i].room==2||c.game.enemy_crew[i].room==18)c.game.enemy_crew[i].health=0;
  for(int k=0;k<5;k++){campaign_step(&c);} /* Counts refresh while the hull holds. */
  int scrap=c.scrap;c.game.enemy.hull=0;for(int k=0;k<3000&&c.mode==CAM_COMBAT;k++){campaign_step(&c);}
  if(c.mode!=CAM_EVENT)fprintf(stderr,"mode%d out%d valid%d c0=%d c2=%d c18=%d scrap%d/%d",c.mode,c.game.outcome,c.boss_crew_valid,c.boss_crew[0],c.boss_crew[2],c.boss_crew[18],c.scrap,scrap);
  {int sum=0;for(int r=0;r<GAME_FLAGSHIP_ROOMS;r++)sum+=c.boss_crew[r];
   if(!(c.mode==CAM_EVENT&&c.boss_crew_valid&&sum==9&&!c.boss_crew[18]&&c.boss_crew[9]==1&&c.scrap>=scrap+30&&c.scrap<=scrap+40))fprintf(stderr,"mode%d valid%d sum%d r18=%d r9=%d scrap%d/%d",c.mode,c.boss_crew_valid,sum,c.boss_crew[18],c.boss_crew[9],c.scrap,scrap);
   /* Isolated gunners stay in their rooms; the killed ion gunner is not replaced. */
   assert(c.mode==CAM_EVENT&&c.boss_crew_valid&&sum==9&&!c.boss_crew[18]&&c.boss_crew[9]==1&&c.scrap>=scrap+30&&c.scrap<=scrap+40);}
  campaign_continue(&c);assert(c.mode==CAM_MAP);
  assert(campaign_save(&c,"tests/flagship.sav")&&campaign_load(&d,"tests/flagship.sav"));assert(d.boss_crew_valid&&!memcmp(d.boss_crew,c.boss_crew,sizeof c.boss_crew)&&d.boss_resume_phase==1);
  {FILE*f=fopen("tests/flagship.sav","rb");unsigned char hd[12];assert(f&&fread(hd,1,12,f)==12&&!fclose(f));assert(hd[8]==32);}
  remove("tests/flagship.sav");remove("tests/flagship.sav.bak");
  /* Stage 2 starts with the stage-1 survivors (the ion room no longer exists). */
  int guard=0;while(c.mode==CAM_MAP&&guard++<40){if(campaign_boss_node(&c)==c.current){campaign_wait(&c);continue;}campaign_wait(&c);}
  if(c.mode==CAM_COMBAT){assert(c.boss_phase==1&&game_alive_enemy_crew(&c.game)==9);}}
 /* Winnability: the strong attainable build beats stage 1 in most seeded fights. */
 {int wins=0;for(unsigned s=1;s<=20;s++)wins+=strong_fight(s);fprintf(stderr,"Strong DS build vs authentic stage 1: %d/20 wins.\n",wins);assert(wins>=14);}
 puts("PASS: V42 flagship stages, artillery scaling/targets, cloak, crew rules/isolation, AI takeover, persistence, stage reward, save26 and winnability.");return 0;
}
