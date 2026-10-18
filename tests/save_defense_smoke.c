#include "save.h"
#include "drones.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char*path="tests/defense-save.sav",*backup="tests/defense-save.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,v);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,old;campaign_init_legacy_seed(&c,2201);c.mode=CAM_STORE;c.scrap=200;
 assert(campaign_purchase(&c,11)&&campaign_purchase(&c,14)&&c.scrap==65&&c.drone_cargo[0]==1);
 before=c;assert(!campaign_purchase(&c,14)&&!memcmp(&before,&c,sizeof c));
 assert(campaign_drone_swap(&c,0,0)&&c.drone_cargo[0]==0&&c.game.drones.slots[0].catalog_id==1);
 c.mode=CAM_MAP;assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(d.game.drones.slots[0].catalog_id==1&&d.drone_cargo[0]==0&&!d.stock[0][14]&&d.scrap==65);
 assert(drones_power(1)==2&&drones_price(1)==50);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);
 memcpy(bytes,valid,n);put32(bytes+n-(1028+464),2);header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));
 /* Re-labeling current Defense ownership as old format14 must not enable it. */
 memcpy(bytes,valid,n);old=n-(1028+464);header(old,14);writefile(old);assert(!campaign_load(&d,path));assert(!remove(path));
 campaign_init_legacy_seed(&c,2202);c.mode=CAM_STORE;c.scrap=100;assert(campaign_purchase(&c,11));c.mode=CAM_MAP;assert(campaign_save(&c,path));n=readfile(path);old=n-(1028+464);header(old,14);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&d.game.drones.slots[0].catalog_id==0&&d.scrap==15);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)assert(d.stock[i][14]==1);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));assert(!remove(path)&&!remove(backup));
 puts("Defense saves: paid purchase/equip, ownership/stock, invalid stock and legacy catalog rejection, format14 migration/backup passed");return 0;
}
