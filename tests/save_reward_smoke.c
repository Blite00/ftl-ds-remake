#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[17000],original[17000],legacy[17000];
static const char *path="tests/reward-save.sav",*backup="tests/reward-save.sav.bak";
static void clean(void){remove(path);remove(backup);remove("tests/reward-save.sav.bad");remove("tests/reward-save.sav.tmp");}
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(i*8));}
static void writefile(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,(uint32_t)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);FILE*f=fopen(path,"wb");assert(f&&fwrite(bytes,1,n,f)==n&&!fclose(f));}
int main(void){
 clean();campaign_init_seed(&c,292300);c.event_policy=4;c.event_seen=2047;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(d.event_policy==4&&d.event_seen==2047&&!d.reward.kind);size_t n=readfile(path);assert(bytes[8]==32);memcpy(original,bytes,n);
 /* An interrupted pending offer recovers the last safe checkpoint, with no offer or reserved ID leaked into it. */
 for(int crew=0;crew<2;crew++){
  c=d;assert(crew?rewards_offer_crew(&c.reward,&c.game,SPEC_ROCK):rewards_offer_weapon(&c.reward,1));before=c;
  assert(!campaign_save(&c,path)&&!memcmp(&before,&c,sizeof c));assert(readfile(path)==n&&!memcmp(bytes,original,n));FILE*f=fopen(backup,"rb");assert(!f);
  memset(&c,0xa5,sizeof c);assert(campaign_load(&c,path)&&!c.reward.kind&&c.event_policy==4&&c.event_seen==2047&&c.game.next_crew_id==d.game.next_crew_id&&c.scrap==d.scrap&&c.fuel==d.fuel);
 }
 /* Same-sized historical22 payload accepts only its historical policy bounds. */
 size_t h=n-464; /* V25-V27 append 60+80+4 bytes of final-sector state. */
 memcpy(bytes,original,n);writefile(h,22);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 memcpy(bytes,original,n);put32(bytes+h-16,3);put32(bytes+h-12,512);writefile(h,22);assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 memcpy(bytes,original,n);put32(bytes+h-16,3);put32(bytes+h-12,511);writefile(h,22);assert(readfile(path)==h);memcpy(legacy,bytes,h);
 assert(campaign_load(&d,path)&&d.event_policy==3&&d.event_seen==511&&d.boss_resume_phase==-1);assert(campaign_save(&d,path));assert(readfile(backup)==h&&!memcmp(bytes,legacy,h));assert(readfile(path)==n&&bytes[8]==32&&!memcmp(bytes+20,legacy+20,h-20));
 clean();c=d;c.event_policy=4;c.event_seen=2048;assert(!campaign_save(&c,path));c.event_policy=7;c.event_seen=0;assert(!campaign_save(&c,path));
 puts("Reward save23: pending weapon/recruit refusal leaves checkpoint byte-exact, safe coldload, policy4 mask2047 roundtrip, historical22 policy bounds and exact unchanged-payload/backup migration passed.");
}
