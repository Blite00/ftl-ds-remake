/* Save integration fixtures: sector/charge/transient edits isolate boundaries.
 * These do not claim earned campaign victories or resources. */
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[17000],original[17000];
static const char*path="tests/enemy-loadout.sav",*backup="tests/enemy-loadout.sav.bak";
static void clean(void){remove(path);remove(backup);remove("tests/enemy-loadout.sav.tmp");remove("tests/enemy-loadout.sav.bad");}
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void stage(unsigned seed,int policy){campaign_init_seed(&c,seed);c.event_policy=policy;c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0)&&c.mode==CAM_MAP&&c.sector==1);}
static void cleared(const Game*g){WeaponState empty={0};assert(!g->enemy_weapon_count&&!memcmp(&g->enemy_weapon_secondary,&empty,sizeof empty)&&!g->enemy_weapon.charge_ticks&&!g->enemy_weapon.pending_shots&&!g->enemy_fire_ticks&&!g->enemy_pending_shots&&!g->enemy_shot_ticks);for(int i=0;i<GAME_MAX_EFFECTS;i++)assert(!g->effects[i].active);}
static void same_profile(const Campaign*a,const Campaign*b){assert(a->rng==b->rng&&a->event_seen==b->event_seen&&a->game.enemy_kind==b->game.enemy_kind&&a->game.enemy_weapon_count==b->game.enemy_weapon_count&&a->game.enemy_missiles==b->game.enemy_missiles);for(int i=0;i<game_enemy_weapon_slots(&a->game);i++){const WeaponState*x=game_enemy_weapon_at(&a->game,i),*y=game_enemy_weapon_at(&b->game,i);assert(x->catalog_id==y->catalog_id&&x->power==y->power&&x->cooldown_ticks==y->cooldown_ticks&&x->damage==y->damage&&x->charge_ticks==y->charge_ticks);}}
int main(void){int configured=0;
 for(unsigned seed=1;seed<=32;seed++){
  clean();stage(seed,4);assert(campaign_save(&c,path));size_t n=readfile(path);memcpy(original,bytes,n);assert(campaign_load(&d,path));cleared(&d.game);
  assert(campaign_jump(&c,1)&&campaign_jump(&d,1));same_profile(&c,&d);if(c.game.enemy_weapon_count)configured++;
  for(int t=0;t<10;t++){campaign_step(&c);campaign_step(&d);}same_profile(&c,&d);
  before=c;assert(!campaign_save(&c,path)&&!memcmp(&before,&c,sizeof c));assert(readfile(path)==n&&!memcmp(bytes,original,n));FILE*f=fopen(backup,"rb");assert(!f);
  c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;assert(campaign_retreat(&c,0,0));campaign_continue(&c);assert(c.mode==CAM_MAP&&campaign_save(&c,path));memset(&d,0xa5,sizeof d);assert(campaign_load(&d,path));cleared(&d.game);assert(d.current==c.current&&d.jumps==c.jumps&&d.fuel==c.fuel&&d.game.player.hull==c.game.player.hull);
 }
 assert(configured>0);
 /* Safe map serialization contains no encounter loadout, even with leftover
  * transient charge/queue values; profile choice is recomputed on arrival. */
 clean();stage(7,4);int ids[]={2,3};assert(game_enemy_set_loadout(&c.game,ids,2,7));c.game.enemy_weapon.charge_ticks=17;c.game.enemy_fire_ticks=17;c.game.enemy_weapon_secondary.charge_ticks=31;c.game.enemy_weapon_secondary.pending_shots=1;assert(campaign_save(&c,path)&&campaign_load(&d,path));cleared(&d.game);assert(d.game.missiles==c.game.missiles&&d.game.weapon_count==c.game.weapon_count);
 /* Original station engagements also derive the new profile after cold load,
  * while historical policies retain their legacy enemy path. */
 for(int policy=0;policy<=4;policy++){
  clean();stage(19,policy);assert(campaign_save(&c,path)&&campaign_load(&d,path)&&d.event_policy==policy);cleared(&d.game);
  if(policy>=2){assert(campaign_imported_begin(&c,6)&&campaign_imported_begin(&d,6));assert(campaign_choice(&c,0)&&campaign_choice(&d,0)&&campaign_choice(&c,0)&&campaign_choice(&d,0));}
  else assert(campaign_jump(&c,1)&&campaign_jump(&d,1));
  same_profile(&c,&d);if(policy<4)assert(!c.game.enemy_weapon_count&&!d.game.enemy_weapon_count);else assert(c.game.enemy_weapon_count>0);
 }
 clean();puts("Enemy loadout saves: active refusal preserves exact checkpoint, authored/original profile continuation, retreat coldload and map transient normalization, historical policy0-3 retained passed (explicit fixtures).");
}
