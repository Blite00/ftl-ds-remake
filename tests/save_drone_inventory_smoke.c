#include "save.h"
#include "drones.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char*path="tests/drone-inventory.sav";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,v);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n,old;
 campaign_init_legacy_seed(&c,210014);c.mode=CAM_STORE;c.scrap=200;
 assert(campaign_purchase(&c,11)&&campaign_purchase(&c,12)&&campaign_purchase(&c,13));
 assert(c.scrap==41&&c.game.drone_parts==7&&c.drone_cargo[0]==0&&c.game.drones.slots[0].catalog_id==0);
 c.mode=CAM_MAP;assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.scrap==41&&d.game.drone_parts==7&&d.drone_cargo[0]==0&&d.drone_cargo[1]==-1);
 assert(!d.stock[0][11]&&!d.stock[0][12]&&d.stock[0][13]==2);
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);
 /* Bounded cargo and stock must be validated, independently of checksum. */
 for(int k=0;k<5;k++){memcpy(bytes,valid,n);size_t off=n-(1188+464)+(k<2?0:16+(k-2)*4);put32(bytes+off,k==0?-2:k==1?GAME_DRONE_CATALOG_COUNT:k==4?4:2);header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));}
 /* A real format13 payload retains installed control/parts, but gains no cargo. */
 memcpy(bytes,valid,n);old=n-(1188+464);header(old,13);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&d.game.drones.installed&&d.game.drone_parts==7&&d.game.drones.slots[0].catalog_id==0);
 for(int i=0;i<CAM_CARGO_COUNT;i++)assert(d.drone_cargo[i]==-1);
 for(int i=0;i<CAM_LEGACY_NODE_COUNT;i++)assert(d.stock[i][11]==1&&d.stock[i][12]==1&&d.stock[i][13]==3);
 assert(campaign_save(&d,path));assert(readfile("tests/drone-inventory.sav.bak")==old&&!memcmp(bytes,legacy,old));
 assert(!remove(path)&&!remove("tests/drone-inventory.sav.bak"));
 puts("Drone inventory saves: purchased ownership/stock, invalid cargo/stock rejection, format13 defaults and byte-exact backup passed");return 0;
}
