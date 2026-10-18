#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned char bytes[9000],legacy[9000];
static const char *path="tests/crew-state.sav",*backup="tests/crew-state.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!ferror(f));assert(!fclose(f));return n;}
static void writefile(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(i*8));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,(unsigned)version);put32(bytes+12,(unsigned)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static int exists(const char*p){FILE*f=fopen(p,"rb");if(!f)return 0;fclose(f);return 1;}
int main(void){Campaign c,d,unchanged;size_t n,old;assert(!exists(path)&&!exists(backup));
 campaign_init_legacy_seed(&c,777);assert(game_rename_crew(&c.game,0,"  O'NEIL-7  "));assert(!strcmp(c.game.crew[0].name,"O'NEIL-7"));
 assert(!game_rename_crew(&c.game,0,"   "));assert(!game_rename_crew(&c.game,0,"ILLEGAL@"));assert(!game_rename_crew(&c.game,0,"TOO LONG A NAME"));
 game_save_stations(&c.game);unsigned id=c.game.crew[0].id;int station=c.game.crew[0].station;
 assert(game_order_crew(&c.game,0,10));c.game.crew[0].health=37;
 assert(campaign_save(&c,path));assert(campaign_load(&d,path));assert(d.game.crew[0].id==id&&d.game.crew[0].station==station&&d.game.crew[0].destination==10&&d.game.crew[0].health==37&&!strcmp(d.game.crew[0].name,"O'NEIL-7"));assert(d.game.next_crew_id==c.game.next_crew_id);
 n=readfile(path);old=n-(1448+464)-(size_t)c.game.crew_count*12-(size_t)c.game.player.room_count*4-24-(size_t)c.game.door_count*12-(size_t)c.game.player.room_count*8-24-(size_t)c.game.crew_count*8-4-(size_t)c.game.crew_count*(3+GAME_CREW_NAME_LEN)*4;
 /* Duplicate identities, invalid station and nonterminated names fail despite correct checksums. */
 size_t first=old+4;
 put32(bytes+first+4,(uint32_t)99);header(n,27);writefile(path,n);unchanged=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&unchanged,sizeof d));
 assert(!remove(path));assert(campaign_save(&c,path));n=readfile(path);
 put32(bytes+first+(3+GAME_CREW_NAME_LEN)*4,id);header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 assert(!remove(path));assert(campaign_save(&c,path));n=readfile(path);
 put32(bytes+first+(3+GAME_CREW_NAME_LEN-1)*4,'X');header(n,27);writefile(path,n);assert(!campaign_load(&d,path));
 assert(!remove(path));assert(campaign_save(&c,path));n=readfile(path);
 header(old,3);memcpy(legacy,bytes,old);writefile(path,old);assert(campaign_load(&d,path));assert(d.game.crew[0].health==37&&d.game.crew[0].id==1&&d.game.crew[0].station==d.game.crew[0].room&&!strcmp(d.game.crew[0].name,"CREW 1"));
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));
 assert(!remove(backup));n=readfile(path);header(n,99);memcpy(legacy,bytes,n);writefile(path,n);assert(!campaign_save(&d,path));assert(readfile(path)==n&&!memcmp(bytes,legacy,n));assert(!remove(path));
 /* Replacing a dead recruit receives a fresh identity, never an alias of a survivor. */
 c.mode=CAM_STORE;c.scrap=100;c.game.crew[1].health=0;unsigned dead=c.game.crew[1].id;assert(campaign_purchase(&c,3));assert(c.game.crew[1].id!=dead&&c.game.crew[1].id!=c.game.crew[0].id&&c.game.crew[1].health==species_max_health(c.game.crew[1].species));
 for(int i=c.game.crew_count;i<GAME_MAX_CREW;i++){game_init_crew(&c.game,0,i,c.game.pilot_room);c.game.crew_count++;}
 assert(game_rename_crew(&c.game,5,"LAST-5"));c.game.crew[5].health=0;c.game.crew[5].station=c.game.player.systems[SYS_MEDBAY].room;c.mode=CAM_MAP;
 assert(campaign_save(&c,path));assert(campaign_load(&d,path));assert(d.game.crew_count==GAME_MAX_CREW&&d.game.crew[5].id==c.game.crew[5].id&&d.game.crew[5].station==c.game.crew[5].station&&!strcmp(d.game.crew[5].name,"LAST-5")&&!d.game.crew[5].health);
 unsigned next=d.game.next_crew_id;d.mode=CAM_STORE;d.stock[d.current][3]=1;d.scrap=100;assert(campaign_purchase(&d,3));assert(d.game.crew[5].id==next&&d.game.next_crew_id==next+1);assert(!remove(path));
 puts("Crew saves: identity/name/station roundtrip, v3 migration, malformed rejection, future version protection and replacement identity passed");return 0;
}
