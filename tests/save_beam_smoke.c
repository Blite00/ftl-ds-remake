#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/beam.sav",*backup="tests/beam.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,base;campaign_init_legacy_seed(&c,1801);c.mode=CAM_STORE;c.scrap=200;
 for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=0;
 assert(!campaign_purchase(&c,9)&&c.scrap==200&&c.stock[0][9]==1);
 for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=-1;
 assert(campaign_offer_price(&c,9)==65&&campaign_purchase(&c,9)&&c.scrap==135&&c.cargo[0]==5);
 assert(!campaign_purchase(&c,9)&&c.scrap==135);assert(campaign_swap(&c,0,2));c.mode=CAM_MAP;
 c.game.weapons[2].beam_target_valid=1;c.game.weapons[2].beam_x0=32;c.game.weapons[2].beam_y0=32;c.game.weapons[2].beam_x1=60;c.game.weapons[2].beam_y1=60;
 c.game.weapons[2].charge_ticks=42;c.game.autofire[2]=1;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.game.weapons[2].catalog_id==5&&d.game.weapons[2].beam_length>0&&d.stock[0][9]==0);
 assert(!d.game.weapons[2].beam_target_valid&&!d.game.weapons[2].charge_ticks&&!d.game.autofire[2]);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);base=n-(1236+464)-CAM_LEGACY_NODE_COUNT*8;
 memcpy(bytes,valid,n);put32(bytes+base,2);header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 assert(!remove(path));
 campaign_init_legacy_seed(&c,1802);c.stock[0][8]=0;assert(campaign_save(&c,path));n=readfile(path);base=n-(1236+464)-CAM_LEGACY_NODE_COUNT*8;
 header(base,9);memcpy(legacy,bytes,base);writefile(base);assert(campaign_load(&d,path));
 assert(d.stock[0][8]==0);for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)assert(d.stock[i][9]==1);
 assert(campaign_save(&d,path));assert(readfile(backup)==base&&!memcmp(bytes,legacy,base));
 assert(!remove(path)&&!remove(backup));
 puts("Beam saves: paid purchase/install, catalog and stock, transient target reset, corruption rejection and exact v9 migration backup passed");return 0;
}
