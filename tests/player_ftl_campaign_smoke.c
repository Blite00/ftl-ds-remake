#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char a[8192],b[8192];
static size_t read_file(const char *path,unsigned char *data){
 FILE *f=fopen(path,"rb");assert(f);size_t n=fread(data,1,8192,f);assert(feof(f)&&!fclose(f));return n;
}
int main(void){
 const char *path="tests/player-ftl-campaign.sav";
 const char *comparison="tests/player-ftl-comparison.sav";
 campaign_init_seed(&c,280028);
 /* No FTL charge from safe-beacon management ticks or read-only queries. */
 for(int i=0;i<2*GAME_HZ;i++)campaign_step(&c);
 assert(c.mode==CAM_MAP&&!c.game.player_ftl_charge);
 assert(campaign_save(&c,path));size_t n=read_file(path,a);
 /* Map-only format deliberately excludes charge, including stale encounter
    values. The two saves must encode identical persistent state. */
 c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;
 assert(campaign_save(&c,comparison));assert(n==read_file(comparison,b)&&!memcmp(a,b,n));
 memset(&d,0x5a,sizeof d);assert(campaign_load(&d,comparison));
 assert(!d.game.player_ftl_charge&&d.mode==CAM_MAP);
 assert(campaign_jump(&d,1)&&d.mode==CAM_COMBAT);
 assert(!d.game.player_ftl_charge);
 int hull=d.game.player.hull,fuel=d.fuel,jumps=d.jumps,scrap=d.scrap;
 /* These are actual simulation ticks with the starting ship/loadout; no
    resource grants or combat outcomes are injected. This is not a full run. */
 for(int i=0;i<GAME_HZ;i++)campaign_step(&d);
 assert(d.mode==CAM_COMBAT&&d.game.player_ftl_charge>0);
 assert(d.fuel==fuel&&d.jumps==jumps&&d.scrap==scrap&&d.game.player.hull<=hull);
 assert(!campaign_save(&d,path));assert(n==read_file(path,b)&&!memcmp(a,b,n));
 d.game.paused=1;before=d;for(int i=0;i<GAME_HZ;i++)campaign_step(&d);
 assert(!memcmp(&d,&before,sizeof d));
 /* Original escape warnings freeze both ships and reject checkpoint saving. */
 campaign_init_seed(&c,280029);assert(campaign_imported_begin(&c,8));
 assert(campaign_choice(&c,0)&&c.imported_battle==4&&c.mode==CAM_EVENT);
 before=c;for(int i=0;i<GAME_HZ;i++)campaign_step(&c);
 assert(!memcmp(&c,&before,sizeof c));assert(!campaign_save(&c,path));
 assert(campaign_choice(&c,0)&&c.mode==CAM_COMBAT);
 campaign_step(&c);assert(c.game.player_ftl_charge>0);
 assert(!remove(path)&&!remove(comparison));
 puts("Player FTL campaign: live combat charging, narrative/pause/service freeze, no travel/resource side effects, format21 byte identity, cold-load reset and active-save rejection passed");
 return 0;
}
