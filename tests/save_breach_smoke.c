/* Disposable fixture files; never accesses user saves. */
#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],oldbytes[9000];
static const char *path="tests/breach-state.sav",*backup="tests/breach-state.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,old;campaign_init_legacy_seed(&c,1401);
 for(int i=0;i<c.game.player.room_count;i++){GameRoom*r=&c.game.player.rooms[i];r->breaches=r->w*r->h<4?r->w*r->h:4;r->breach_repair_ticks=149-i;}
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 for(int i=0;i<c.game.player.room_count;i++){assert(c.game.player.rooms[i].breaches==d.game.player.rooms[i].breaches);assert(c.game.player.rooms[i].breach_repair_ticks==d.game.player.rooms[i].breach_repair_ticks);}
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);old=n-(1448+464)-(size_t)c.game.crew_count*12-(size_t)c.game.player.room_count*4-24-(size_t)c.game.door_count*12-(size_t)c.game.player.room_count*8;
 /* Malformed but checksum-valid hazards cannot change the caller's state. */
 for(int k=0;k<4;k++){
  memcpy(bytes,valid,n);
  if(k==0)put32(bytes+old,5);
  if(k==1)put32(bytes+old+4,5*GAME_HZ);
  if(k==2)put32(bytes+old,0); /* repair timer without a breach */
  if(k==3)put32(bytes+old,UINT32_MAX);
  header(n,27);writefile(path,n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 }
 /* Format5 has no breach extension. It must migrate with zero defaults and exact backup. */
 memcpy(bytes,valid,n);header(old,5);memcpy(oldbytes,bytes,old);writefile(path,old);
 assert(campaign_load(&d,path)&&d.seed==1401);
 for(int i=0;i<d.game.player.room_count;i++)assert(!d.game.player.rooms[i].breaches&&!d.game.player.rooms[i].breach_repair_ticks);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,oldbytes,old));
 /* Invalid primary recovers that valid legacy backup. */
 n=readfile(path);bytes[n-1]^=1;writefile(path,n);assert(campaign_load(&d,path)&&d.seed==1401);
 assert(!remove(path)&&!remove(backup));
 puts("Breach saves: all-room count/progress roundtrip, bounded malformed rejection, v5 zero defaults, exact backup and recovery passed");return 0;
}
