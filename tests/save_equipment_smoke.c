#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],legacy[9000];
static const char*path="tests/equipment.sav",*backup="tests/equipment.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int v){uint32_t h=2166136261u;put32(bytes+8,(unsigned)v);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
int main(void){size_t n;campaign_init_legacy_seed(&c,2001);
 c.mode=CAM_COMBAT;before=c;assert(!campaign_unequip(&c,0)&&!campaign_reorder_weapon(&c,0,1)&&!campaign_sell_weapon(&c,1,0)&&!memcmp(&c,&before,sizeof c));c.mode=CAM_MAP;
 for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=2;
 before=c;assert(!campaign_unequip(&c,0)&&!memcmp(&c,&before,sizeof c));
 for(int i=0;i<CAM_CARGO_COUNT;i++)c.cargo[i]=-1;
 assert(campaign_reorder_weapon(&c,1,0));assert(c.game.weapons[0].catalog_id==1&&c.game.weapons[1].catalog_id==0);
 c.game.weapons[0].enabled=0;assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(d.game.weapons[0].catalog_id==1&&!d.game.weapons[0].enabled);
 assert(!remove(path));
 assert(campaign_unequip(&c,1)&&c.cargo[0]==0);assert(campaign_unequip(&c,0)&&c.cargo[1]==1&&!c.game.weapon_count);
 assert(campaign_save(&c,path)&&campaign_load(&d,path)&&!d.game.weapon_count);
 for(int i=0;i<GAME_MAX_WEAPONS;i++){WeaponState zero={0};assert(!memcmp(&d.game.weapons[i],&zero,sizeof zero)&&!d.game.autofire[i]);}
 n=readfile(path);assert(bytes[8]==32);n-=(1236+464);header(n,11);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));assert(!remove(path));
 assert(campaign_swap(&d,1,0)&&d.game.weapon_count==1&&d.game.weapons[0].catalog_id==1&&d.cargo[1]==-1);
 d.mode=CAM_STORE;int scrap=d.scrap,price=campaign_weapon_sell_price(&d,0,0);assert(price==19&&campaign_sell_weapon(&d,0,0)&&d.scrap==scrap+19&&d.cargo[0]==-1);assert(!campaign_sell_weapon(&d,0,0));
 price=campaign_weapon_sell_price(&d,1,0);assert(price==32&&campaign_sell_weapon(&d,1,0)&&!d.game.weapon_count);before=d;assert(!campaign_purchase(&d,7)&&!memcmp(&d,&before,sizeof d));
 campaign_init_legacy_seed(&c,2002);c.game.weapons[0].enabled=0;assert(campaign_save(&c,path));n=readfile(path);n-=(1236+464);header(n,11);memcpy(legacy,bytes,n);writefile(n);
 assert(campaign_load(&d,path)&&d.game.weapons[0].catalog_id==1&&d.game.weapons[1].catalog_id==0&&!d.game.weapons[1].enabled);
 assert(campaign_save(&d,path));assert(readfile(backup)==n&&!memcmp(bytes,legacy,n));assert(!remove(path)&&!remove(backup));
 puts("Equipment saves: mode/full-cargo guards, zero loadout, re-equip, exact sale/price, order persistence and byte-exact v11 priority migration passed");return 0;
}
