/* Forced-charge/phase fixtures validate persistence, not earned boss victories. */
#include "save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d,before;
static unsigned char bytes[17000],original[17000],legacy[17000];
static const char*path="tests/boss-retreat.sav",*backup="tests/boss-retreat.sav.bak";
static void clean(void){remove(path);remove(backup);remove("tests/boss-retreat.sav.bad");remove("tests/boss-retreat.sav.tmp");}
static size_t readfile(const char*p){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(bytes,1,sizeof bytes,f);assert(feof(f)&&!fclose(f));return n;}
static void put32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(i*8));}
static void writefile(size_t n,int version){uint32_t h=2166136261u;put32(bytes+8,version);put32(bytes+12,(uint32_t)n-20);for(size_t i=20;i<n;i++)h=(h^bytes[i])*16777619u;put32(bytes+16,h);FILE*f=fopen(path,"wb");assert(f&&fwrite(bytes,1,n,f)==n&&!fclose(f));}
int main(void){
 for(int phase=0;phase<3;phase++){
  clean();campaign_init_legacy_seed(&c,282200+phase);c.sector=c.sector_count-1;c.boss_phase=c.boss_resume_phase=phase;
  c.visited[2]=1;assert(game_begin_encounter(&c.game,ENEMY_FLAGSHIP,c.sector,phase));c.mode=CAM_COMBAT;c.game.player_ftl_charge=GAME_PLAYER_FTL_CHARGE;
  assert(campaign_can_retreat(&c,2)&&campaign_retreat(&c,2,0));assert(c.mode==CAM_EVENT&&c.event_resolved);campaign_continue(&c);assert(c.mode==CAM_MAP&&c.boss_resume_phase==phase);
  assert(campaign_save(&c,path));memset(&d,0,sizeof d);assert(campaign_load(&d,path)&&d.boss_resume_phase==phase&&d.boss_phase==-1);
  int hull=d.game.player.hull,ammo=d.game.missiles;d.mode=CAM_EVENT;d.event_id=EVENT_EXIT;d.event_resolved=0;assert(campaign_choice(&d,0));assert(d.mode==CAM_COMBAT&&d.boss_phase==phase&&d.game.player.hull==hull&&d.game.missiles==ammo);
 }
 /* Latest malformed phase/sector combinations reject without touching caller. */
 clean();campaign_init_legacy_seed(&c,282210);assert(campaign_save(&c,path));size_t n=readfile(path);assert(bytes[8]==32);memcpy(original,bytes,n);
 int bad[]={-2,3,0,1,2};for(int i=0;i<5;i++){memcpy(bytes,original,n);put32(bytes+n-468,(uint32_t)bad[i]);writefile(n-464,23);before=d;assert(!campaign_load(&d,path)&&!memcmp(&before,&d,sizeof d));}
 c.boss_resume_phase=0;assert(!campaign_save(&c,path));c.sector=c.sector_count-1;c.boss_resume_phase=3;assert(!campaign_save(&c,path));
 /* Structurally faithful save21 fixture omits only the newly appended word. */
 memcpy(bytes,original,n);writefile(n-468,21);size_t old=readfile(path);memcpy(legacy,bytes,old);assert(old==n-468&&campaign_load(&d,path)&&d.boss_resume_phase==-1);
 assert(campaign_save(&d,path));assert(readfile(backup)==old&&!memcmp(bytes,legacy,old));assert(readfile(path)==n&&bytes[8]==32&&!memcmp(bytes+20,legacy+20,old-20));for(size_t i=n-468;i<n-464;i++)assert(bytes[i]==255);
 clean();puts("Boss retreat save23: phases0-2 retreat/coldload/reentry without repeat tender, malformed phase/sector atomic rejection and exact save21 prefix/backup migration passed.");
}
