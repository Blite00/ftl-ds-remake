#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/sector-routes.sav",*backup="tests/sector-routes.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void reject(size_t n,int field,int value){memcpy(bytes,valid,n);put32(bytes+n-524+field*4,value);header(n,28);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
static void same(const Campaign*a,const Campaign*b){assert(a->route_policy==b->route_policy&&a->sector==b->sector&&a->rng==b->rng&&a->fuel==b->fuel&&a->scrap==b->scrap);assert(!memcmp(a->sector_types,b->sector_types,sizeof a->sector_types)&&!memcmp(a->next_sector,b->next_sector,sizeof a->next_sector)&&!memcmp(a->links,b->links,sizeof a->links)&&!memcmp(a->node_kind,b->node_kind,sizeof a->node_kind)&&!memcmp(a->stock,b->stock,sizeof a->stock));}
/* Isolate sector transition/save behavior; this is not an earned campaign. */
static void exit_event(Campaign*a){a->mode=CAM_EVENT;a->event_id=EVENT_EXIT;a->event_resolved=0;a->current=a->node_count-1;a->visited[a->current]=1;}
int main(void){
 size_t n,old;
 campaign_init_seed(&c,24017);assert(c.route_policy==1&&c.sector_types[0]==SECTOR_CIVILIAN);
 assert(campaign_save(&c,path)&&campaign_load(&d,path));same(&c,&d);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);
 reject(n,0,2);reject(n,0,0);reject(n,1,SECTOR_ENGI);reject(n,2,SECTOR_REBEL);reject(n,1,SECTOR_TYPE_COUNT);
 reject(n,9,0);reject(n,10,SECTOR_FINAL);reject(n,10,c.next_sector[0]);
 memcpy(bytes,valid,n);writefile(n);
 /* Both pending branches survive disk, and chosen routes produce identical maps. */
 for(int sector=0;sector<7;sector++){
  exit_event(&c);exit_event(&d);int choice=sector<6?sector%2:0;
  assert(campaign_choice(&c,choice)&&campaign_choice(&d,choice));same(&c,&d);
  assert(c.mode==CAM_MAP&&c.sector==sector+1&&campaign_save(&c,path)&&campaign_load(&d,path));same(&c,&d);
 }
 assert(c.sector_types[7]==SECTOR_FINAL&&!c.next_sector[0]&&!c.next_sector[1]);
 assert(!remove(backup));n=readfile(path);memcpy(valid,bytes,n);reject(n,8,SECTOR_REBEL);reject(n,9,SECTOR_CIVILIAN);
 /* Sector six has exactly one final destination. */
 campaign_init_seed(&c,24018);for(int i=0;i<6;i++){exit_event(&c);assert(campaign_choice(&c,i%2));}
 assert(!remove(path));assert(campaign_save(&c,path));n=readfile(path);memcpy(valid,bytes,n);reject(n,9,SECTOR_CIVILIAN);reject(n,10,SECTOR_FINAL);
 /* A true format16 prefix migrates without rerolling its graph or its RNG. */
 campaign_init_seed(&c,24019);assert(!remove(path));assert(campaign_save(&c,path));n=readfile(path);old=n-524;header(old,16);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&!d.route_policy&&d.node_count==23&&d.sector_count==8);
 c.route_policy=0;memset(c.sector_types,0,sizeof c.sector_types);memset(c.next_sector,0,sizeof c.next_sector);same(&c,&d);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 assert(campaign_load(&c,path));same(&c,&d);exit_event(&c);assert(campaign_choice_count(&c)==1&&campaign_choice(&c,0)&&!c.route_policy);
 assert(!remove(path)&&!remove(backup));
 campaign_init_legacy_seed(&c,24020);c.route_policy=1;c.sector_types[0]=SECTOR_CIVILIAN;c.next_sector[0]=SECTOR_CIVILIAN;c.next_sector[1]=SECTOR_ENGI;assert(!campaign_save(&c,path));
 puts("Sector saves: v17 branches/history, alternating route continuation through final sector, malformed metadata atomic rejection, v16 unchanged map/RNG migration and exact backup passed");return 0;
}
