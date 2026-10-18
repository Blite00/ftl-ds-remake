/* All fixtures belong to the disposable test working directory. */
#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned char bytes[9000],legacy[9000];
static const char*path="tests/save-fire-check.sav";
static const char*backup="tests/save-fire-check.sav.bak";
static int exists(const char*p){FILE*f=fopen(p,"rb");if(!f)return 0;fclose(f);return 1;}
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(i*8));}
static uint32_t get32(const unsigned char*p){uint32_t v=0;for(int i=0;i<4;i++)v|=(uint32_t)p[i]<<(i*8);return v;}
static void rehash(size_t n){uint32_t h=2166136261u;for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){
 Campaign c,d;assert(!exists(path)&&!exists(backup)&&!exists("tests/save-fire-check.sav.tmp")&&!exists("tests/save-fire-check.sav.bad"));
 campaign_init_legacy_seed(&c,123);c.scrap=73;c.game.player.rooms[0].fires=1;c.game.player.rooms[0].fire_ticks=17;c.game.player.rooms[0].firefight_ticks=25;
 assert(campaign_save(&c,path));assert(campaign_load(&d,path));
 assert(d.game.player.rooms[0].fires==1&&d.game.player.rooms[0].fire_ticks==17&&d.game.player.rooms[0].firefight_ticks==25);
 size_t n=readfile(path);assert(get32(bytes+8)==32);size_t metadata=(1448+464)+(size_t)c.game.crew_count*12+(size_t)c.game.player.room_count*4+24+(size_t)c.game.door_count*12+(size_t)c.game.player.room_count*8+24+(size_t)c.game.crew_count*8+4+(size_t)c.game.crew_count*(3+GAME_CREW_NAME_LEN)*4;size_t old_n=n-(size_t)c.game.player.room_count*12-metadata;
 put32(bytes+8,2);put32(bytes+12,(uint32_t)old_n-20);rehash(old_n);memcpy(legacy,bytes,old_n);writefile(path,old_n);
 assert(campaign_load(&d,path));assert(d.scrap==73&&d.seed==123);
 for(int i=0;i<d.game.player.room_count;i++)assert(!d.game.player.rooms[i].fires&&!d.game.player.rooms[i].fire_ticks&&!d.game.player.rooms[i].firefight_ticks);
 assert(campaign_save(&d,path));assert(readfile(backup)==old_n&&!memcmp(bytes,legacy,old_n));
 n=readfile(path);assert(get32(bytes+8)==32&&n==old_n+(size_t)d.game.player.room_count*12+metadata);
 /* A valid checksum must not bypass bounded fire-state validation. */
 put32(bytes+old_n,(uint32_t)d.game.player.rooms[0].w*d.game.player.rooms[0].h+1);rehash(n);writefile(path,n);
 assert(!remove(backup));Campaign before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));assert(!remove(path));
 puts("Fire saves: v20 roundtrip, v2 zero-default migration, byte-preserved legacy backup, malformed fire rejection passed");
}
