/* Disposable host fixtures; assert every path is absent before creating any. */
#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static const char *path="tests/save-v10-check.sav";
static const char *paths[]={"tests/save-v10-check.sav","tests/save-v10-check.sav.bak","tests/save-v10-check.sav.tmp","tests/save-v10-check.sav.bad","tests/save-v10-invalid.sav","tests/save-v10-invalid.sav.bak","tests/save-v10-invalid.sav.tmp","tests/save-v10-invalid.sav.bad"};
static unsigned char bytes[9000];
static int exists(const char *p){FILE *f=fopen(p,"rb");if(!f)return 0;assert(!fclose(f));return 1;}
static size_t read_bytes(const char *p){FILE*f=fopen(p,"rb");size_t n;assert(f);n=fread(bytes,1,sizeof bytes,f);assert(!ferror(f));assert(feof(f));assert(!fclose(f));return n;}
static void write_bytes(const char*p,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));}
static void damage(const char*p){size_t n=read_bytes(p);assert(n>24);bytes[24]^=64;write_bytes(p,n);}
static void put32(unsigned char*p,uint32_t v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static void malformed_field(const char*p,unsigned word,uint32_t val){size_t n=read_bytes(p);uint32_t h=2166136261u;assert(20+4*word+4<=n);put32(bytes+20+4*word,val);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);write_bytes(p,n);}
static void foreign(const char *p,const char *magic,size_t n){memcpy(bytes,magic,n);write_bytes(p,n);}
static void compare(const Campaign*a,const Campaign*b){
 assert(a->seed==b->seed&&a->rng==b->rng&&a->sector==b->sector&&a->current==b->current&&a->selected==b->selected);
 assert(a->fuel==b->fuel&&a->scrap==b->scrap&&a->jumps==b->jumps&&a->sector_jumps==b->sector_jumps&&a->quest==b->quest);
 assert(!memcmp(a->visited,b->visited,sizeof a->visited));assert(!memcmp(a->links,b->links,sizeof a->links));assert(!memcmp(a->node_y,b->node_y,sizeof a->node_y));assert(!memcmp(a->node_kind,b->node_kind,sizeof a->node_kind));assert(!memcmp(a->stock,b->stock,sizeof a->stock));assert(!memcmp(a->cargo,b->cargo,sizeof a->cargo));
 assert(!memcmp(&a->game.player,&b->game.player,sizeof a->game.player));assert(!memcmp(a->game.doors,b->game.doors,sizeof a->game.doors));
 assert(a->game.crew_count==b->game.crew_count&&a->game.weapon_count==b->game.weapon_count);
 assert(!memcmp(a->game.crew,b->game.crew,sizeof(Crew)*(size_t)a->game.crew_count));
 for(int i=0;i<a->game.weapon_count;i++){assert(a->game.weapons[i].catalog_id==b->game.weapons[i].catalog_id);assert(a->game.weapons[i].enabled==b->game.weapons[i].enabled);assert(!strcmp(a->game.weapons[i].name,b->game.weapons[i].name));assert(!b->game.autofire[i]);}
 assert(a->game.rng==b->game.rng&&a->game.ticks==b->game.ticks&&a->game.missiles==b->game.missiles&&a->game.paused==b->game.paused&&a->game.pilot_health==b->game.pilot_health);
}
int main(void){
 Campaign c,d,before,valid;size_t n;int i;for(i=0;i<8;i++)assert(!exists(paths[i]));
 campaign_init_legacy_seed(&c,0xfedcba98u);
 /* Transition API regenerates the map and preserves run progression. */
 c.mode=CAM_EVENT;c.event_id=EVENT_EXIT;c.event_resolved=0;assert(campaign_choice(&c,0));assert(c.sector==1);
 assert(campaign_jump(&c,2));assert(campaign_choice(&c,0));campaign_continue(&c);assert(campaign_jump(&c,0));campaign_continue(&c);
 assert(campaign_jump(&c,2));campaign_continue(&c);assert(c.visited[2]&&c.current==2&&c.jumps==3);
 c.scrap=1000;c.mode=CAM_STORE; /* Purchase actions fill a later-sector persistent stock. */
 assert(campaign_purchase(&c,3));assert(campaign_purchase(&c,4));assert(campaign_swap(&c,0,2));assert(campaign_purchase(&c,5));assert(campaign_purchase(&c,6));
 assert(campaign_upgrade(&c,-1));assert(campaign_upgrade(&c,SYS_WEAPONS));assert(campaign_upgrade(&c,SYS_SHIELDS));campaign_continue(&c);
 c.quest=1;c.game.player.hull=19;c.game.crew[0].health=47;c.game.crew[0].skill[0]=3100;c.game.crew[1].skill[1]=1000;
 c.game.crew[0].move_ticks=7;c.game.crew[0].repair_ticks=13;c.game.crew[0].heal_accumulator=4;
 c.game.player.systems[SYS_WEAPONS].health=1;c.game.player.systems[SYS_WEAPONS].power=1;
 c.game.player.rooms[0].oxygen=270;c.game.player.rooms[0].exposed=1;c.game.doors[0].open=1;c.game.doors[0].transit_ticks=9;
 c.game.missiles=3;c.game.rng=0xf0e1d2c3u;c.game.ticks=456;c.game.weapons[0].enabled=0;c.game.autofire[1]=1;
 assert(campaign_save(&c,path));assert(campaign_load(&d,path));compare(&c,&d);valid=c;
 c.scrap+=5;assert(campaign_save(&c,path));assert(campaign_load(&d,path));compare(&c,&d);
 damage(path);assert(campaign_load(&d,path));compare(&valid,&d);assert(strstr(d.text,"backup"));
 /* Saving after recovery retains the valid backup and quarantines corrupt primary. */
 d.scrap+=7;assert(campaign_save(&d,path));assert(exists(paths[3]));assert(campaign_load(&before,paths[1]));compare(&valid,&before);
 assert(campaign_load(&before,path));compare(&d,&before);
 damage(path);damage(paths[1]);before=d;assert(!campaign_load(&d,path));assert(!memcmp(&before,&d,sizeof d));
 assert(!remove(path));assert(!remove(paths[1]));assert(!remove(paths[3]));
 /* A genuine rescue can advance pursuit beyond real jump count. */
 campaign_init_legacy(&c);c.fuel=0;assert(campaign_rescue(&c));assert(c.sector_jumps>c.jumps);assert(campaign_save(&c,path));assert(campaign_load(&d,path));compare(&c,&d);assert(!remove(path));
 /* Invalid field and graph payloads with correct checksums must still fail. */
 c=valid;assert(campaign_save(&c,paths[4]));before=d;malformed_field(paths[4],5,CAM_LEGACY_SECTOR_COUNT);assert(!campaign_load(&d,paths[4]));assert(!memcmp(&before,&d,sizeof d));assert(!remove(paths[4]));
 assert(campaign_save(&c,paths[4]));malformed_field(paths[4],14,(unsigned)c.links[0]|1u);assert(!campaign_load(&d,paths[4]));assert(!memcmp(&before,&d,sizeof d));assert(!remove(paths[4]));
 c=valid;c.links[0]=1;assert(!campaign_save(&c,path));c=valid;c.cargo[0]=999;assert(!campaign_save(&c,path));c=valid;c.mode=CAM_COMBAT;assert(!campaign_save(&c,path));c=valid;c.game.crew_count=7;assert(!campaign_save(&c,path));
 /* Every transaction path respects foreign data, including historical v9 magic. */
 c=valid;for(i=0;i<4;i++){foreign(paths[i],"FOREIGN",7);assert(!campaign_save(&c,path));n=read_bytes(paths[i]);assert(n==7&&!memcmp(bytes,"FOREIGN",7));assert(!remove(paths[i]));}
 foreign(path,"FTLDSAV\0",8);assert(!campaign_save(&c,path));assert(!campaign_load(&d,path));n=read_bytes(path);assert(n==8&&!memcmp(bytes,"FTLDSAV\0",8));assert(!remove(path));
 for(i=0;i<8;i++)assert(!exists(paths[i]));
 puts("PASS: v10 saves preserve upgraded multi-sector runs, crew/cargo/map/stock; backup recovery and resave; corrupt rejection, invalid fields/graphs, rescue, foreign/v9 protection.");return 0;
}
