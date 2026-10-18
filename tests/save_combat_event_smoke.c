#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/combat-event.sav",*backup="tests/combat-event.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void invalid(size_t n,int version,int policy,int seen){memcpy(bytes,valid,n);put32(bytes+n-16,policy);put32(bytes+n-12,seen);size_t size=version<21?n-8:version<22?n-4:n;header(size,version);writefile(size);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
int main(void){
 campaign_init_seed(&c,260026);c.event_policy=2;assert(c.event_policy==2&&c.imported_ship==-1&&!c.imported_battle);c.event_seen=255;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(d.event_policy==2&&d.event_seen==255&&d.imported_ship==-1&&!d.imported_battle&&d.imported_node==-1);
 size_t n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);size_t full=n;n-=464;
 invalid(n,19,3,0);invalid(n,19,2,256);invalid(n,19,1,64);invalid(n,19,0,1);
 invalid(n,18,2,0);invalid(n,18,1,64); /* Historical format keeps historical policy bounds. */
 memcpy(bytes,valid,n);put32(bytes+n-16,1);put32(bytes+n-12,37);size_t old=n-8;header(old,18);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&d.event_policy==1&&d.event_seen==37&&d.imported_ship==-1&&!d.imported_battle);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));assert(readfile(path)==full&&!memcmp(bytes+20,legacy+20,old-20));
 assert(!remove(path)&&!remove(backup));
 for(int pending=1;pending<=3;pending++){campaign_init_seed(&c,260027);c.imported_battle=pending;assert(!campaign_save(&c,path));}
 campaign_init_seed(&c,260028);c.mode=CAM_COMBAT;assert(!campaign_save(&c,path));
 c.mode=CAM_EVENT;assert(!campaign_save(&c,path));
 campaign_init_legacy(&c);c.event_policy=2;assert(!campaign_save(&c,path));
 puts("Combat-event saves: policy2 eight-root mask, v18 historical bounds, unchanged payload/backup migration, pending battle/map-only rejection and transient resets passed");return 0;
}
