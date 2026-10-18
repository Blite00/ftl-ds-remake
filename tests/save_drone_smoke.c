/* Fixtures live only in the disposable test working directory. */
#include "save.h"
#include "drones.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/drone-state.sav",*backup="tests/drone-state.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,(unsigned)version);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void invalid(size_t n,int field,int value){memcpy(bytes,valid,n);put32(bytes+n-(1236+464)+field*4,(uint32_t)value);header(n,27);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));}
int main(void){size_t n,old;campaign_init_legacy_seed(&c,210021);assert(drones_install(&c.game));
 c.game.drone_parts=17;c.game.player.reactor=20;
 DroneControl*dc=&c.game.drones;dc->capacity=5;dc->health=3;dc->power=2;dc->ion_bars=2;dc->ion_ticks=123;
 dc->slots[0].catalog_id=0;dc->slots[1].catalog_id=0;
 dc->slots[0].enabled=dc->slots[0].deployed=1;dc->slots[0].charge_ticks=27;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.game.drone_parts==17&&d.game.drones.installed&&d.game.drones.room==dc->room);
 assert(d.game.drones.capacity==5&&d.game.drones.health==3&&d.game.drones.power==2&&d.game.drones.ion_bars==2&&d.game.drones.ion_ticks==123);
 assert(d.game.drones.slot_count==2);
 for(int i=0;i<3;i++){assert(d.game.drones.slots[i].catalog_id==(i<2?0:-1));assert(!d.game.drones.slots[i].enabled&&!d.game.drones.slots[i].deployed&&!d.game.drones.slots[i].charge_ticks);}
 n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);
 /* A valid checksum does not excuse invalid capacity, room, power, catalog or state relationships. */
 invalid(n,0,-1);invalid(n,0,1000001);invalid(n,1,2);invalid(n,1,0);invalid(n,2,99);
 invalid(n,3,1);invalid(n,3,9);invalid(n,4,6);invalid(n,5,4);invalid(n,6,0);invalid(n,6,9);invalid(n,7,0);invalid(n,7,40*GAME_HZ+1);
 invalid(n,8,0);invalid(n,8,3);invalid(n,9,GAME_DRONE_CATALOG_COUNT);invalid(n,10,-2);invalid(n,11,0);
 memcpy(bytes,valid,n);writefile(n);c.game.player.reactor=8;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)c.game.player.systems[i].power=0;
 c.game.player.systems[SYS_WEAPONS].power=3;c.game.player.systems[SYS_SHIELDS].power=2;c.game.player.systems[SYS_ENGINES].power=2;c.game.player.systems[SYS_OXYGEN].power=1;
 assert(!campaign_save(&c,path));assert(readfile(path)==n&&!memcmp(bytes,valid,n));
 /* Format12 has no drone fields; new control defaults do not grant parts or drones. */
 memcpy(bytes,valid,n);old=n-(1236+464);header(old,12);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&!d.game.drones.installed&&!d.game.drone_parts);
 for(int i=0;i<3;i++)assert(d.game.drones.slots[i].catalog_id==-1);
 assert(d.seed==210021&&campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 /* Corrupt primary recovers the actual old-format backup. */
 n=readfile(path);bytes[20]^=1;writefile(n);assert(campaign_load(&d,path)&&!d.game.drones.installed&&!d.game.drone_parts);
 assert(!remove(path)&&!remove(backup));
 puts("Drone saves: control/parts/catalog persistence, launch reset, bounds/room/reactor rejection, v12 migration and exact backup recovery passed");return 0;
}
