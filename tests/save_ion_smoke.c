#include "save.h"
#include "boarding.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/ion.sav",*backup="tests/ion.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,base;campaign_init_legacy_seed(&c,1701);c.mode=CAM_STORE;c.scrap=100;
 assert(campaign_offer_price(&c,8)==30&&campaign_purchase(&c,8)&&c.scrap==70&&c.cargo[0]==4);
 assert(!campaign_purchase(&c,8)&&c.scrap==70);assert(campaign_swap(&c,0,2));c.mode=CAM_MAP;
 assert(boarding_install(&c.game,0));
 c.game.player.systems[SYS_WEAPONS].ion_bars=2;c.game.player.systems[SYS_WEAPONS].ion_ticks=299;
 c.game.subsystems[0][SUB_SENSORS].ion_bars=1;c.game.subsystems[0][SUB_SENSORS].ion_ticks=77;
 c.game.teleporter[0].ion_bars=1;c.game.teleporter[0].ion_ticks=44;c.game.pilot_ion_ticks[0]=10;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.game.weapons[2].catalog_id==4&&d.stock[0][8]==0);
 assert(d.game.player.systems[SYS_WEAPONS].ion_bars==2&&d.game.player.systems[SYS_WEAPONS].ion_ticks==299);
 assert(d.game.subsystems[0][SUB_SENSORS].ion_ticks==77&&d.game.teleporter[0].ion_ticks==44&&d.game.pilot_ion_ticks[0]==10);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);base=n-(1448+464);
 for(int k=0;k<8;k++){
  memcpy(bytes,valid,n);
  if(k==0)put32(bytes+base,9);
  if(k==1)put32(bytes+base+4,1201);
  if(k==2)put32(bytes+base+4,1); /* zero bars with a timer */
  if(k==3)put32(bytes+base+SYS_WEAPONS*8+4,0); /* bars without timer */
  if(k==4)put32(bytes+base+40+SUB_SENSORS*8,9);
  if(k==5)put32(bytes+base+56+4,1201);
  if(k==6)put32(bytes+base+64,1201);
  if(k==7)put32(bytes+base+68,2);
  header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 }
 assert(!remove(path));
 campaign_init_legacy_seed(&c,1702);assert(campaign_save(&c,path));n=readfile(path);base=n-(1448+464);
 header(base,8);memcpy(legacy,bytes,base);writefile(base);assert(campaign_load(&d,path));
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)assert(!d.game.player.systems[i].ion_ticks&&!d.game.player.systems[i].ion_bars);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)assert(d.stock[i][8]==1);
 assert(campaign_save(&d,path));assert(readfile(backup)==base&&!memcmp(bytes,legacy,base));
 assert(!remove(path)&&!remove(backup));
 puts("Ion saves: paid purchase/install, stock, lockouts, malformed rejection, v8 migration and byte-exact backup passed");return 0;
}
