#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[17000],valid[17000],legacy[17000];
static const char *path="tests/intruder-state.sav",*backup="tests/intruder-state.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void invalid(size_t n,size_t offset,uint32_t value){memcpy(bytes,valid,n);put32(bytes+offset,value);header(n,24);writefile(n);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
static Crew*add(int slot,int room){Crew*r=&c.game.enemy_crew[slot];memset(r,0,sizeof *r);r->id=c.game.next_crew_id++;r->ship=0;r->room=r->destination=room;r->station=31;r->health=100;r->species=SPEC_HUMAN;snprintf(r->name,sizeof r->name,"Intruder%02d",slot);if(c.game.enemy_crew_count<=slot)c.game.enemy_crew_count=slot+1;return r;}
static void full_roster(void){campaign_init_seed(&c,282021);int slot=GAME_MAX_ENEMY_CREW;for(int r=0;r<c.game.player.room_count;r++)for(int n=0;n<c.game.player.rooms[r].w*c.game.player.rooms[r].h;n++){
 Crew*x=add(slot++,c.game.player.rooms[r].id);x->move_ticks=3;x->repair_ticks=7;x->heal_accumulator=9;x->combat_ticks=11;x->sabotage_ticks=13;x->ability_ticks=15;x->species=SPEC_CRYSTAL;for(int k=0;k<4;k++)x->skill[k]=17+k;
 }assert(slot==GAME_MAX_HOSTILES);}
int main(void){
 full_roster();assert(campaign_save(&c,path)&&campaign_load(&d,path));
 assert(d.game.enemy_crew_count==GAME_MAX_HOSTILES&&!memcmp(c.game.enemy_crew,d.game.enemy_crew,sizeof c.game.enemy_crew));
 assert(d.game.next_crew_id==c.game.next_crew_id&&!d.game.enemy.hull&&d.game.enemy_departed&&strstr(d.text,"intruders"));
 size_t n=readfile(path);assert(bytes[8]==32&&n>8192);memcpy(valid,bytes,n);n-=464;size_t base=n-8-GAME_MAX_INTRUDERS*128,first=base+4;
 invalid(n,base,49);invalid(n,first,7);invalid(n,first,56);invalid(n,first+128,8);
 invalid(n,first+4,0);invalid(n,first+4,c.game.crew[0].id);invalid(n,first+4,c.game.next_crew_id);invalid(n,first+128+4,c.game.enemy_crew[GAME_MAX_ENEMY_CREW].id);
 invalid(n,first+8,31);invalid(n,first+12,31);invalid(n,first+16,32);invalid(n,first+20,1);invalid(n,first+24,SPEC_COUNT);invalid(n,first+24,SPEC_HUMAN);invalid(n,first+28,126);
 invalid(n,first+32,species_move_ticks(SPEC_CRYSTAL));invalid(n,first+36,5*GAME_HZ);invalid(n,first+40,GAME_HZ);invalid(n,first+44,GAME_HZ);invalid(n,first+48,5*GAME_HZ);invalid(n,first+52,CRYSTAL_COOLDOWN_TICKS+1);invalid(n,first+56,1);invalid(n,first+60,120*GAME_HZ+1);
 invalid(n,first+76,0);invalid(n,first+76,'!');invalid(n,first+124,'Z');
 /* Three live occupants in the first two-cell room exceeds physical capacity. */
 invalid(n,first+256+8,c.game.player.rooms[0].id);
 memcpy(bytes,valid,n);header(n-1,24);writefile(n-1);before=d;assert(!campaign_load(&d,path)&&!memcmp(&d,&before,sizeof d));
 assert(!remove(path));
 /* Rejected source states must not replace the valid primary or backup. */
 full_roster();assert(campaign_save(&c,path));n=readfile(path);memcpy(valid,bytes,n);
 c.game.enemy_crew[GAME_MAX_HOSTILES-1].ship=1;assert(!campaign_save(&c,path));assert(readfile(path)==n&&!memcmp(bytes,valid,n));c.game.enemy_crew[GAME_MAX_HOSTILES-1].ship=0;
 c.game.enemy_crew[0]=c.game.enemy_crew[GAME_MAX_HOSTILES-1];assert(!campaign_save(&c,path));c.game.enemy_crew[0].health=0;c.game.enemy_crew[0].death_processed=0;assert(!campaign_save(&c,path));
 assert(!remove(path));
 /* Pending carried death remains pending after load and triggers once at home. */
 campaign_init_seed(&c,282022);Crew*r=add(GAME_MAX_HOSTILES-1,c.game.pilot_room);r->species=SPEC_ZOLTAN;r->health=0;r->death_processed=0;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(!d.game.enemy_crew[GAME_MAX_HOSTILES-1].death_processed&&d.game.enemy_crew[GAME_MAX_HOSTILES-1].health==0);
 int health=d.game.crew[0].health;species_step(&d.game,1);assert(d.game.enemy_crew[GAME_MAX_HOSTILES-1].death_processed&&d.game.crew[0].health==health-15);species_step(&d.game,1);assert(d.game.crew[0].health==health-15);
 assert(!remove(path));
 /* Processed corpses are retired and do not create a false hostile warning. */
 campaign_init_seed(&c,282024);r=add(GAME_MAX_HOSTILES-1,c.game.pilot_room);r->health=0;r->death_processed=1;
 assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(!d.game.enemy_crew_count&&!d.game.enemy_departed&&!strstr(d.text,"intruders"));assert(!remove(path));
 /* A true format20 payload has no appendix; next save appends only count0. */
 campaign_init_seed(&c,282023);c.event_policy=3;assert(campaign_save(&c,path));n=readfile(path);assert(n>4);header(n-472,20);memcpy(legacy,bytes,n-472);writefile(n-472);
 assert(campaign_load(&d,path)&&!d.game.enemy_crew_count&&!d.game.enemy_departed);assert(campaign_save(&d,path));assert(readfile(backup)==n-472&&!memcmp(bytes,legacy,n-472));assert(readfile(path)==n&&!memcmp(bytes+20,legacy+20,n-492));
 assert(!remove(path)&&!remove(backup));
 puts("Intruder save23: full48/slot55 exact Crew roundtrip, bounded fields/IDs/names/occupancy, transactional malformed rejection, pending death once, no remote ship resurrection, explicit promotion guard and format20 prefix/backup migration passed");return 0;
}
