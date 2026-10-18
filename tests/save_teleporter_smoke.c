/* Files are fixtures in the disposable host-test working directory. */
#include "save.h"
#include "boarding.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],legacy[9000],valid[9000];
static const char *path="tests/teleporter-state.sav",*backup="tests/teleporter-state.sav.bak";
static int exists(const char*p){FILE*f=fopen(p,"rb");if(!f)return 0;fclose(f);return 1;}
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(i*8));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,(unsigned)version);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,old;assert(!exists(path)&&!exists(backup));
 campaign_init_legacy_seed(&c,12345);c.mode=CAM_STORE;c.scrap=200;assert(campaign_install_teleporter(&c));assert(campaign_upgrade_teleporter(&c));campaign_continue(&c);
 c.game.player.reactor=20;assert(boarding_power(&c.game,0,1));c.game.teleporter[0].health=1;c.game.teleporter[0].cooldown_ticks=257;
 c.game.crew[0].combat_ticks=17;c.game.crew[0].sabotage_ticks=71;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(!memcmp(&c.game.teleporter[0],&d.game.teleporter[0],sizeof(Teleporter)));
 assert(d.game.crew[0].combat_ticks==17&&d.game.crew[0].sabotage_ticks==71&&d.scrap==40);
 n=readfile(path);memcpy(valid,bytes,n);old=n-(1448+464)-(size_t)c.game.crew_count*12-(size_t)c.game.player.room_count*4-24-(size_t)c.game.door_count*12-(size_t)c.game.player.room_count*8-24-(size_t)c.game.crew_count*8;assert(bytes[8]==32);
 /* Recomputed checksums cannot bypass teleporter identity/timer bounds. */
 put32(bytes+old+4,99);header(n,27);writefile(path,n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 memcpy(bytes,valid,n);put32(bytes+old+20,BOARDING_COOLDOWN+1);header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 memcpy(bytes,valid,n);put32(bytes+old,0);header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 memcpy(bytes,valid,n);put32(bytes+old+24,GAME_HZ);header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 memcpy(bytes,valid,n);put32(bytes+old+28,5*GAME_HZ);header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 /* Reject an overallocated live state before replacing any existing save. */
 memcpy(bytes,valid,n);writefile(path,n);c.game.player.reactor=8;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)c.game.player.systems[i].power=0;
 c.game.player.systems[SYS_WEAPONS].power=3;c.game.player.systems[SYS_SHIELDS].power=2;c.game.player.systems[SYS_ENGINES].power=2;c.game.player.systems[SYS_OXYGEN].power=1;
 assert(game_used_power(&c.game)==9&&!campaign_save(&c,path));assert(readfile(path)==n&&!memcmp(bytes,valid,n));
 /* V4 has no teleporter extension; upgrading preserves the exact source backup. */
 memcpy(bytes,valid,n);header(old,4);memcpy(legacy,bytes,old);writefile(path,old);
 assert(campaign_load(&d,path)&&!d.game.teleporter[0].installed&&!d.game.crew[0].combat_ticks&&!d.game.crew[0].sabotage_ticks);
 assert(d.seed==12345&&d.scrap==40);assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 assert(!remove(backup));assert(!remove(path));
 puts("Teleporter saves: damaged/powered/cooldown roundtrip, bounded timers, overreactor rejection, v4 migration and exact backup preservation passed");return 0;
}
