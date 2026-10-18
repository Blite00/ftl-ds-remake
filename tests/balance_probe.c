/* Reproducible balance measurement, not a pass/fail skill test.
 * First-contact rows use a real fresh campaign jump. Other rows deliberately
 * select a sector-two matchup, retaining the untouched starting Kestrel.
 * No injected damage, victories, supplies, charge, health, or upgrades. */
#include "campaign.h"
#include <assert.h>
#include <stdio.h>
static Campaign c;
static unsigned pool_seen;
enum { QUICK_WEAPONS, QUICK_SHIELDS, QUICK_HULL, DELAY_WEAPONS, DELAY_HULL, PASSIVE, POLICIES };
static const char*names[]={"ready-weapons","ready-shields","ready-empty-room","5s-check-weapons","5s-check-empty-room","no-input"};
static int target(const Game*g,int p){
 int sys=(p==QUICK_SHIELDS)?SYS_SHIELDS:SYS_WEAPONS;
 if(p==QUICK_HULL||p==DELAY_HULL){for(int i=0;i<g->enemy.room_count;i++){const GameRoom*r=&g->enemy.rooms[i];if(r->system<0&&r->id!=g->enemy_pilot_room&&r->id!=g->enemy_drone.room)return r->id;}}
 if(g->enemy.systems[sys].capacity>0)return g->enemy.systems[sys].room;
 return g->enemy.systems[SYS_WEAPONS].room;
}
static void run(int kind,int p,int count){int wins=0,losses=0,timeouts=0,minloss=100,maxloss=0,damage=0,missiles=0,ticks=0,crew=0,crew_losses=0,boarder_holds=0;
 for(unsigned seed=1;seed<=(unsigned)count;seed++){
  campaign_init_seed(&c,seed);
  if(kind==-2){
   /* Select a later authored fight through the real campaign dispatcher.
    * Only this path/node fixture is artificial; combat/loadout are untouched. */
   c.links[0]|=1<<4;c.links[4]|=1;c.node_kind[4]=NODE_FIGHT;
   assert(campaign_jump(&c,4)&&c.mode==CAM_COMBAT);
   assert(c.game.enemy_kind==ENEMY_INTERCEPTOR||c.game.enemy_kind==ENEMY_ARMORED);pool_seen|=1u<<c.game.enemy_kind;
  }else if(kind<0){assert(campaign_jump(&c,1)&&c.mode==CAM_COMBAT);}else assert(game_begin_encounter(&c.game,kind,1,0));
  Game*g=&c.game;int hull=g->player.hull,ammo=g->missiles,t=0,room=target(g,p);
  for(;t<120*GAME_HZ&&!g->outcome;t++){
   if(p!=PASSIVE&&(p<DELAY_WEAPONS||(t>=8*GAME_HZ&&t%(5*GAME_HZ)==0))){
    /* Inspecting/issuing orders while paused costs no simulation time. These
     * are ordinary ready-fire actions, not instant charge or extra damage. */
    if(p<DELAY_WEAPONS)game_toggle_pause(g);
    for(int i=0;i<g->weapon_count;i++)if(game_weapon_powered(g,i)&&game_weapons_manned(g,0)&&g->weapons[i].charge_ticks>=g->weapons[i].cooldown_ticks&&!g->weapons[i].pending_shots&&g->missiles>=g->weapons[i].missiles)assert(game_fire(g,i,room));
    if(p<DELAY_WEAPONS)game_toggle_pause(g);
   }
   game_step(g);
  }
  crew+=game_alive_crew(g);crew_losses+=g->result_reason==RESULT_PLAYER_CREW;boarder_holds+=!g->outcome&&g->enemy.hull<=0;
  int lost=hull-g->player.hull;damage+=lost;if(lost<minloss)minloss=lost;if(lost>maxloss)maxloss=lost;missiles+=ammo-g->missiles;ticks+=t;
  if(g->outcome==GAME_VICTORY)wins++;else if(g->outcome==GAME_DEFEAT)losses++;else timeouts++;
 }
 printf("%s,%s,%d,%d,%d,%d,%.2f,%d,%d,%.2f,%.2f,%.2f,%d,%d\n",kind==-2?"later-opening-pool":kind<0?"first-contact":game_enemy_name(&c.game),names[p],count,wins,losses,timeouts,(double)damage/count,minloss,maxloss,(double)missiles/count,(double)ticks/count/GAME_HZ,(double)crew/count,crew_losses,boarder_holds);
}
static void historical_pool(void){
 for(int policy=0;policy<=4;policy++)for(unsigned seed=1;seed<=16;seed++){
  campaign_init_seed(&c,seed);c.event_policy=policy;c.links[0]|=1<<4;c.links[4]|=1;c.node_kind[4]=NODE_FIGHT;unsigned rng=c.rng;
  assert(campaign_jump(&c,4));
  if(policy<4)assert(c.game.enemy_kind==ENEMY_SCOUT&&c.rng==rng);
  else assert(c.game.enemy_kind!=ENEMY_SCOUT&&c.rng!=rng);
  campaign_init_legacy_seed(&c,seed);rng=c.rng;c.node_kind[2]=NODE_FIGHT;assert(campaign_jump(&c,2)&&c.game.enemy_kind==ENEMY_SCOUT&&c.rng==rng);
 }
}
int main(void){
 historical_pool();
 puts("matchup,policy,seeds,wins,losses,timeouts,mean_hull_loss,min_hull_loss,max_hull_loss,mean_missiles,mean_seconds,mean_living_crew,crew_loss_defeats,boarder_cleanup_timeouts");
 for(int p=0;p<POLICIES;p++)run(-1,p,128);
 for(int p=0;p<POLICIES;p++)run(-2,p,128);
 assert(pool_seen==((1u<<ENEMY_INTERCEPTOR)|(1u<<ENEMY_ARMORED)));
 int kinds[]={ENEMY_SCOUT,ENEMY_INTERCEPTOR,ENEMY_ARMORED,ENEMY_ASSAULT,ENEMY_REBEL,ENEMY_REBEL_BOARDER};
 for(unsigned k=0;k<sizeof kinds/sizeof kinds[0];k++)for(int p=0;p<POLICIES;p++)run(kinds[k],p,32);
}
