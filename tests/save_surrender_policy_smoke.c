#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[16384],valid[16384],old[16384];
static const char *path="tests/surrender-policy.sav",*backup="tests/surrender-policy.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){
 campaign_init_seed(&c,320032);assert(c.event_policy==6);c.event_policy=5;c.event_seen=4095;assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(d.event_policy==5&&d.event_seen==4095&&d.imported_surrender_threshold==-1&&!d.imported_escape_warned);
 size_t n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);
 /* V25-27 append 36 final-sector words (396 bytes); older layouts end before them. */
 size_t full=n;n-=464;
 /* Same byte layout is intentional, but older headers retain their old bounds. */
 header(n,23);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 memcpy(bytes,valid,n);put32(bytes+n-16,4);put32(bytes+n-12,2048);header(n,23);writefile(n);assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 memcpy(bytes,valid,n);put32(bytes+n-16,4);put32(bytes+n-12,2047);header(n,23);memcpy(old,bytes,n);writefile(n);assert(campaign_load(&d,path)&&d.event_policy==4&&d.event_seen==2047);
 assert(campaign_save(&d,path));assert(readfile(backup)==n&&!memcmp(bytes,old,n));assert(readfile(path)==full&&bytes[8]==32&&!memcmp(bytes+20,old+20,n-20));assert(!remove(path)&&!remove(backup));
 c.event_seen=4096;assert(!campaign_save(&c,path));c.event_seen=0;c.event_policy=7;assert(!campaign_save(&c,path));
 campaign_init_seed(&c,320033);assert(campaign_imported_begin(&c,11));assert(!campaign_save(&c,path));
 puts("Save24 surrender policy: full12-root mask, transient reset, older header bounds, exact v23-layout migration/backup and invalid metadata rejection passed.");return 0;
}
