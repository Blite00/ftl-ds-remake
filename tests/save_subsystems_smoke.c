/* Disposable fixture files; all writes stay in the test run directory. */
#include "save.h"
#include "subsystems.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/subsystem-state.sav",*backup="tests/subsystem-state.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,old;campaign_init_legacy_seed(&c,1501);c.scrap=200;
 assert(campaign_subsystem_upgrade_price(&c,SUB_DOORS)==35&&campaign_subsystem_upgrade_price(&c,SUB_SENSORS)==25);
 c.mode=CAM_COMBAT;assert(!campaign_upgrade_subsystem(&c,SUB_DOORS)&&c.scrap==200);c.mode=CAM_MAP;
 int power=game_used_power(&c.game);assert(campaign_upgrade_subsystem(&c,SUB_DOORS)&&c.scrap==165);assert(campaign_upgrade_subsystem(&c,SUB_DOORS)&&c.scrap==115);assert(!campaign_upgrade_subsystem(&c,SUB_DOORS));
 assert(campaign_upgrade_subsystem(&c,SUB_SENSORS)&&c.scrap==90);assert(campaign_upgrade_subsystem(&c,SUB_SENSORS)&&c.scrap==50);assert(game_used_power(&c.game)==power);
 c.game.subsystems[0][SUB_DOORS].health=2;c.game.subsystems[0][SUB_SENSORS].health=0;
 c.game.doors[0].force_ticks=100;c.game.doors[0].force_idle_ticks=12;
 c.game.doors[1].open=1;c.game.doors[1].jam_ticks=149;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(!memcmp(c.game.subsystems[0],d.game.subsystems[0],sizeof c.game.subsystems[0]));
 assert(d.game.doors[0].force_ticks==100&&d.game.doors[0].force_idle_ticks==12&&d.game.doors[1].jam_ticks==149&&d.game.doors[1].open);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);old=n-(1448+464)-(size_t)c.game.crew_count*12-(size_t)c.game.player.room_count*4-24-(size_t)c.game.door_count*12;
 for(int k=0;k<7;k++){
  memcpy(bytes,valid,n);
  if(k==0)put32(bytes+old,99);
  if(k==1)put32(bytes+old+4,4);
  if(k==2)put32(bytes+old+8,4);
  if(k==3)put32(bytes+old+24,4*GAME_HZ);
  if(k==4)put32(bytes+old+28,GAME_HZ);
  if(k==5)put32(bytes+old+32,5*GAME_HZ+1);
  if(k==6)put32(bytes+old+32,1); /* first door closed, cannot be jammed open */
  header(n,27);writefile(path,n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 }
 memcpy(bytes,valid,n);header(old,6);memcpy(legacy,bytes,old);writefile(path,old);
 assert(campaign_load(&d,path)&&d.seed==1501&&d.scrap==50);
 for(int i=0;i<SUB_COUNT;i++)assert(d.game.subsystems[0][i].capacity==1&&d.game.subsystems[0][i].health==1);
 for(int i=0;i<d.game.door_count;i++)assert(!d.game.doors[i].force_ticks&&!d.game.doors[i].force_idle_ticks&&!d.game.doors[i].jam_ticks);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 assert(!remove(path)&&!remove(backup));
 puts("Subsystems: original upgrade costs, mode/cap/reactor guards, damaged state/door timers, malformed rejection and exact v6 migration backup passed");return 0;
}
