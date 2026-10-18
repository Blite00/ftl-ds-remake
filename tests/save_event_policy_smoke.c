#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/event-policy.sav",*backup="tests/event-policy.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void same(const Campaign*a,const Campaign*b){assert(a->seed==b->seed&&a->rng==b->rng&&a->sector==b->sector&&a->route_policy==b->route_policy&&a->fuel==b->fuel&&a->scrap==b->scrap);assert(!memcmp(a->links,b->links,sizeof a->links)&&!memcmp(a->node_kind,b->node_kind,sizeof a->node_kind)&&!memcmp(a->stock,b->stock,sizeof a->stock)&&!memcmp(a->sector_types,b->sector_types,sizeof a->sector_types)&&!memcmp(a->next_sector,b->next_sector,sizeof a->next_sector));}
int main(void){
 campaign_init_seed(&c,250025);c.event_policy=1;assert(c.event_policy==1&&c.imported_node==-1);c.event_seen=37;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));same(&c,&d);assert(d.event_policy==1&&d.event_seen==37&&d.imported_node==-1);
 size_t n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);size_t full=n;n-=464;
 for(int k=0;k<2;k++){memcpy(bytes,valid,n);put32(bytes+n-16,k?5:UINT32_MAX);header(n,23);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));}
 memcpy(bytes,valid,n);put32(bytes+n-12,64);header(n,23);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));
 memcpy(bytes,valid,n);size_t old=n-16;header(old,17);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path));same(&c,&d);assert(!d.event_policy&&!d.event_seen&&d.imported_node==-1);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));assert(readfile(path)==full&&!memcmp(bytes+20,legacy+20,old-20));
 assert(campaign_load(&c,path)&&!c.event_policy&&c.imported_node==-1);same(&c,&d);
 assert(!remove(path)&&!remove(backup));
 campaign_init_legacy(&c);c.event_policy=1;assert(!campaign_save(&c,path));
 campaign_init_seed(&c,250026);c.route_policy=0;memset(c.sector_types,0,sizeof c.sector_types);memset(c.next_sector,0,sizeof c.next_sector);assert(!campaign_save(&c,path));
 c.event_policy=0;assert(campaign_save(&c,path)&&campaign_load(&d,path)&&!d.event_policy);assert(!remove(path));
 campaign_init_seed(&c,250027);c.event_policy=7;assert(!campaign_save(&c,path));
 c.event_policy=0;c.event_seen=1;assert(!campaign_save(&c,path));
 c.event_policy=1;c.event_seen=64;assert(!campaign_save(&c,path));
 c.event_seen=0;c.mode=CAM_EVENT;assert(!campaign_save(&c,path));
 /* Real imported actions preserve uniqueness and future random outcomes across disk. */
 campaign_init_seed(&c,250028);c.event_policy=1;assert(campaign_imported_begin(&c,0)&&campaign_choice(&c,0));campaign_continue(&c);
 assert(campaign_save(&c,path)&&campaign_load(&d,path));before=d;
 assert(!campaign_imported_begin(&d,0)&&!memcmp(&d,&before,sizeof d));
 assert(campaign_imported_begin(&c,4)&&campaign_imported_begin(&d,4));
 assert(c.imported_node==d.imported_node&&c.rng==d.rng&&!strcmp(campaign_text(&c),campaign_text(&d)));
 assert(campaign_choice(&c,0)&&campaign_choice(&d,0));same(&c,&d);assert(c.game.drone_parts==d.game.drone_parts);
 assert(!remove(path));
 puts("Event save policy: v23 roundtrip, v17 exact prefix/backup with policy0, transient reset, malformed metadata atomic rejection and policy/map-only encode guards passed");return 0;
}
