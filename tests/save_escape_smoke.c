#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[9000],valid[9000],legacy[9000];
static const char *path="tests/escape-event.sav",*backup="tests/escape-event.sav.bak";
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void writefile(size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n&&!fclose(f));}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static void header(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);}
static void invalid(size_t n,int version,int policy,int seen){memcpy(bytes,valid,n);put32(bytes+n-16,policy);put32(bytes+n-12,seen);size_t size=version<21?n-8:version<22?n-4:n;header(size,version);writefile(size);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
static void reset_transients(const Campaign *x){
 assert(x->imported_ship==-1&&x->imported_node==-1&&!x->imported_battle);
 assert(!x->game.enemy_escape_ticks&&!x->game.enemy_escape_total&&!x->game.enemy_escape_threshold&&!x->game.enemy_escape_active&&!x->game.enemy_departed);
}
int main(void){
 campaign_init_seed(&c,270027);c.event_policy=3;assert(c.event_policy==3);
 assert(campaign_wait(&c)&&c.sector_jumps==1&&c.jumps==0);
 c.fuel=0;assert(campaign_rescue(&c)&&c.sector_jumps==3&&c.jumps==0);
 assert(campaign_imported_begin(&c,8)&&campaign_choice(&c,0));
 assert(c.imported_battle==4&&c.mode==CAM_EVENT);assert(!campaign_save(&c,path));
 assert(campaign_choice(&c,0)&&c.mode==CAM_COMBAT);assert(!campaign_save(&c,path));
 /* Focused persistence fixture shortens charge; earned campaign evidence is separate. */
 c.game.enemy_escape_ticks=1;assert(game_enemy_escape_ready(&c.game));
 for(int i=0;i<200&&c.mode==CAM_COMBAT;i++)campaign_step(&c);
 assert(c.mode==CAM_EVENT&&c.event_resolved&&c.game.result_reason==RESULT_ENEMY_ESCAPE&&c.sector_jumps==4&&c.jumps==0);
 campaign_continue(&c);assert(c.mode==CAM_MAP&&c.event_seen==256);
 int fleet=campaign_fleet_x(&c,0);assert(campaign_save(&c,path));
 memset(&d,0x7f,sizeof d);assert(campaign_load(&d,path));reset_transients(&d);
 assert(d.event_policy==3&&d.event_seen==256&&d.sector_jumps==4&&d.jumps==0&&campaign_fleet_x(&d,0)==fleet);
 size_t n=readfile(path);assert(bytes[8]==32);memcpy(valid,bytes,n);size_t full=n;n-=464;
 invalid(n,22,3,512);invalid(n,22,4,0);invalid(n,22,2,256);invalid(n,20,3,512);invalid(n,20,4,0);invalid(n,20,2,256);
 invalid(n,19,3,0);invalid(n,19,2,256);invalid(n,18,2,0);invalid(n,18,1,64);
 memcpy(bytes,valid,n);put32(bytes+n-16,2);put32(bytes+n-12,255);size_t old=n-8;header(old,19);memcpy(legacy,bytes,old);writefile(old);
 assert(campaign_load(&d,path)&&d.event_policy==2&&d.event_seen==255);reset_transients(&d);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));assert(readfile(path)==full&&!memcmp(bytes+20,legacy+20,old-20));
 assert(!remove(path)&&!remove(backup));
 for(int pending=1;pending<=4;pending++){campaign_init_seed(&c,270028);c.imported_battle=pending;assert(!campaign_save(&c,path));}
 puts("Escape saves: WAIT/rescue/pursuit fleet turns roundtrip, policy3 ninth-root bit, historical bounds, unchanged payload/backup migration, map-only warning rejection and all escape transients reset passed");return 0;
}
