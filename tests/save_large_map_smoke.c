#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/large-save.sav",*backup="tests/large-save.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static uint32_t get32(const unsigned char*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,v);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void reject(size_t n,size_t off,uint32_t value){memcpy(bytes,valid,n);put32(bytes+off,value);header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
static void same_map(const Campaign*a,const Campaign*b){assert(a->node_count==b->node_count&&a->sector_count==b->sector_count&&a->current==b->current&&a->selected==b->selected&&a->sector==b->sector&&a->jumps==b->jumps&&a->sector_jumps==b->sector_jumps);assert(!memcmp(a->links,b->links,sizeof a->links)&&!memcmp(a->visited,b->visited,sizeof a->visited)&&!memcmp(a->node_kind,b->node_kind,sizeof a->node_kind)&&!memcmp(a->node_y,b->node_y,sizeof a->node_y)&&!memcmp(a->stock,b->stock,sizeof a->stock));}
int main(void){size_t n,append,old;
 campaign_init_seed(&c,2304);assert(c.node_count==23&&c.sector_count==8);
 /* Jump to a reachable supply beacon, then save resolved campaign state. */
 assert(campaign_can_jump(&c,2)&&campaign_jump(&c,2));
 if(c.mode==CAM_EVENT&&!c.event_resolved)assert(campaign_choice(&c,0));
 if(c.mode==CAM_EVENT&&c.event_resolved)campaign_continue(&c);
 assert(c.mode==CAM_MAP&&c.jumps==1&&c.sector_jumps==1);
 assert(campaign_save(&c,path)&&campaign_load(&d,path));same_map(&c,&d);
 n=readfile(path);assert(bytes[8]==32&&n<8192);memcpy(valid,bytes,n);append=n-(980+464);
 assert(get32(bytes+append)==23&&get32(bytes+append+4)==8);
 reject(n,append,24);reject(n,append+4,6); /* invalid policy pair */
 reject(n,20+5*4,8);reject(n,20+6*4,23);reject(n,20+7*4,23); /* sector/current/selected */
 reject(n,append+8+4,get32(valid+append+8+4)^1u); /* asymmetric edge */
 reject(n,20+11*4,0); /* two visited beacons require at least one jump */
 reject(n,append+8+4,1u<<12); /* extra beacon self edge */
 reject(n,append+8+4,get32(valid+append+8+4)|(1u<<23)); /* outside active map */
 reject(n,append+8+11*76+16,1); /* inactive slot stock must be zero */
 reject(n,append+8+11*76+12,90); /* inactive slot position must be zero */
 reject(n,append+8+10*76,1); /* visited exit disconnected from visited set */
 reject(n,20+13*4+2*48,0); /* current node must be visited */
 /* Re-labeling a large map as legacy cannot silently expand the old policy. */
 memcpy(bytes,valid,n);old=n-(980+464);header(old,15);writefile(old);assert(!campaign_load(&d,path));assert(!remove(path));
 campaign_init_legacy_seed(&c,2305);assert(campaign_save(&c,path));n=readfile(path);old=n-(980+464);header(old,15);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&d.node_count==12&&d.sector_count==6);same_map(&c,&d);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 assert(campaign_load(&c,path)&&c.node_count==12&&c.sector_count==6);same_map(&c,&d);
 assert(!remove(path)&&!remove(backup));
 printf("Large saves: 23-beacon/8-sector roundtrip, jump/fleet counters, malformed graphs/config/inactive state, legacy15 policy and exact backup passed (legacy payload %u, v16 appendix 920)\n",(unsigned)(old-20));return 0;
}
