#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/species.sav",*backup="tests/species.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,base;
 campaign_init_legacy_seed(&c,1601);
 for(int i=3;i<GAME_MAX_CREW;i++)game_init_crew(&c.game,0,i,c.game.player.rooms[i].id);
 c.game.crew_count=GAME_MAX_CREW;
 for(int i=0;i<GAME_MAX_CREW;i++)assert(game_set_crew_species(&c.game,0,i,i%SPEC_COUNT));
 c.game.crew[SPEC_ROCK].move_ticks=59;
 c.game.crew[SPEC_CRYSTAL].ability_ticks=CRYSTAL_COOLDOWN_TICKS;
 c.game.player.rooms[0].lockdown_ticks=CRYSTAL_LOCKDOWN_TICKS;
 c.game.crew[SPEC_ZOLTAN].room=c.game.crew[SPEC_ZOLTAN].destination=c.game.player.systems[SYS_SHIELDS].room;
 species_refresh_power(&c.game);
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.game.crew_count==8&&d.game.next_crew_id==c.game.next_crew_id);
 for(int i=0;i<8;i++){assert(d.game.crew[i].species==c.game.crew[i].species);assert(d.game.crew[i].health==species_max_health(i%SPEC_COUNT));assert(d.game.crew[i].id==c.game.crew[i].id);}
 assert(d.game.crew[SPEC_ROCK].move_ticks==59&&d.game.crew[SPEC_CRYSTAL].ability_ticks==CRYSTAL_COOLDOWN_TICKS);
 assert(d.game.player.rooms[0].lockdown_ticks==CRYSTAL_LOCKDOWN_TICKS);
 assert(d.game.player.systems[SYS_SHIELDS].bonus_power==1&&game_used_power(&c.game)==game_used_power(&d.game));
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);base=n-(1448+464)-8*12-(size_t)c.game.player.room_count*4;
 for(int k=0;k<7;k++){
  memcpy(bytes,valid,n);
  if(k==0)put32(bytes+base,SPEC_COUNT);
  if(k==1)put32(bytes+base+SPEC_ROCK*12,SPEC_ZOLTAN);
  if(k==2)put32(bytes+base+4,1);
  if(k==3)put32(bytes+base+8,1);
  if(k==4)put32(bytes+base+SPEC_CRYSTAL*12+4,CRYSTAL_COOLDOWN_TICKS+1);
  if(k==5)put32(bytes+base+8*12,CRYSTAL_LOCKDOWN_TICKS+1);
  if(k==6)put32(bytes+base+8,2);
  header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 }
 assert(!remove(path));
 /* Legacy format7 has no species appendix and at most six Human crew. */
 campaign_init_legacy_seed(&c,1602);assert(campaign_save(&c,path));n=readfile(path);base=n-(1448+464)-(size_t)c.game.crew_count*12-(size_t)c.game.player.room_count*4;
 header(base,7);memcpy(legacy,bytes,base);writefile(base);assert(campaign_load(&d,path));
 for(int i=0;i<d.game.crew_count;i++)assert(d.game.crew[i].species==SPEC_HUMAN&&!d.game.crew[i].ability_ticks);
 assert(campaign_save(&d,path));assert(readfile(backup)==base&&!memcmp(bytes,legacy,base));
 assert(!remove(path)&&!remove(backup));
 /* Six-species store offers retain price, identity and bounded roster rules. */
 for(int s=0;s<6;s++){
  static const int price[]={45,50,55,55,45,60};campaign_init_legacy_seed(&c,(unsigned)(6+s));c.mode=CAM_STORE;c.scrap=1000;
  assert(campaign_species_offer(&c)==s&&campaign_offer_price(&c,3)==price[s]);
  while(c.game.crew_count<8){c.stock[c.current][3]=1;assert(campaign_purchase(&c,3));assert(c.game.crew[c.game.crew_count-1].species==s);}
  c.stock[c.current][3]=1;int scrap=c.scrap;assert(!campaign_purchase(&c,3)&&c.scrap==scrap);
  unsigned id=c.game.crew[4].id,next=c.game.next_crew_id;
  c.mode=CAM_COMBAT;assert(!campaign_dismiss_crew(&c,3));c.mode=CAM_MAP;
  c.game.crew[3].ship=1;assert(!campaign_dismiss_crew(&c,3));c.game.crew[3].ship=0;
  assert(campaign_dismiss_crew(&c,3)&&c.game.crew_count==7&&c.game.crew[3].id==id&&c.game.next_crew_id==next);
  while(c.game.crew_count>1)assert(campaign_dismiss_crew(&c,0));
  assert(!campaign_dismiss_crew(&c,0));
 }
 puts("Species save: eight mixed crew, derived power, timers, corruption rejection, v7 exact backup, recruitment/dismissal passed");return 0;
}
