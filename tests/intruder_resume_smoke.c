/* Persistence fixtures, not earned campaign runs. Both branches use ordinary
 * service ticks after explicitly constructing the carried-intruder checkpoint. */
#include "save.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign uninterrupted,resumed;
static unsigned char bytes[2][17000];
static void clean(const char*path){char p[256];remove(path);snprintf(p,sizeof p,"%s.bak",path);remove(p);snprintf(p,sizeof p,"%s.bad",path);remove(p);snprintf(p,sizeof p,"%s.tmp",path);remove(p);}
static void equal_disk(const char*a,const char*b){FILE*f=fopen(a,"rb"),*h=fopen(b,"rb");assert(f&&h);size_t n=fread(bytes[0],1,sizeof bytes[0],f),m=fread(bytes[1],1,sizeof bytes[1],h);assert(n<sizeof bytes[0]&&m<sizeof bytes[1]&&!ferror(f)&&!ferror(h));fclose(f);fclose(h);assert(n==m&&!memcmp(bytes[0],bytes[1],n));}
static int cell_room(const Game*g,int cell){for(int r=0;r<g->player.room_count;r++){int n=g->player.rooms[r].w*g->player.rooms[r].h;if(cell<n)return g->player.rooms[r].id;cell-=n;}assert(0);return -1;}
static void setup(int full){campaign_init_seed(&uninterrupted,282121);Game*g=&uninterrupted.game;memset(g->enemy_crew,0,sizeof g->enemy_crew);g->enemy_crew_count=GAME_MAX_HOSTILES;
 for(int i=full?GAME_MAX_ENEMY_CREW:GAME_MAX_HOSTILES-1;i<GAME_MAX_HOSTILES;i++){int room=full?cell_room(g,i-GAME_MAX_ENEMY_CREW):g->player.systems[SYS_WEAPONS].room;game_init_crew(g,1,i,room);Crew*c=&g->enemy_crew[i];c->ship=0;c->health=75;c->combat_ticks=4;c->sabotage_ticks=9;c->skill[0]=17;c->station=31;snprintf(c->name,sizeof c->name,"Resume%u",(unsigned)i%56);}
}
int main(void){const char*path="tests/intruder-resume.sav",*a="tests/intruder-continuous.sav",*b="tests/intruder-restored.sav";clean(path);clean(a);clean(b);
 setup(1);assert(campaign_save(&uninterrupted,path));assert(campaign_load(&resumed,path));assert(resumed.game.enemy_crew_count==GAME_MAX_HOSTILES);
 for(int i=GAME_MAX_ENEMY_CREW;i<GAME_MAX_HOSTILES;i++)assert(!memcmp(&uninterrupted.game.enemy_crew[i],&resumed.game.enemy_crew[i],sizeof(Crew)));
 unsigned id=resumed.game.enemy_crew[GAME_MAX_HOSTILES-1].id;for(int frame=0;frame<10;frame++){game_service_step(&uninterrupted.game);game_service_step(&resumed.game);}assert(resumed.game.enemy_crew[GAME_MAX_HOSTILES-1].id==id);
 assert(campaign_save(&uninterrupted,a)&&campaign_save(&resumed,b));equal_disk(a,b);
 /* Sparse last-slot intruder, fire, oxygen loss, melee and sabotage timers remain
  * deterministic across six real disk save/cold-load boundaries. */
 clean(path);clean(a);clean(b);setup(0);Game*g=&uninterrupted.game;int target=g->player.systems[SYS_WEAPONS].room;
 for(int r=0;r<g->player.room_count;r++)if(g->player.rooms[r].id==target){g->player.rooms[r].fires=1;g->player.rooms[r].breaches=1;g->player.rooms[r].oxygen=100;}
 g->enemy_crew[GAME_MAX_HOSTILES-1].species=SPEC_ROCK;g->enemy_crew[GAME_MAX_HOSTILES-1].health=100;g->crew[2].health=90;
 assert(campaign_save(&uninterrupted,path)&&campaign_load(&resumed,path));
 for(int frame=1;frame<=180;frame++){
  game_service_step(&uninterrupted.game);game_service_step(&resumed.game);
  assert(uninterrupted.game.outcome!=GAME_DEFEAT&&resumed.game.outcome!=GAME_DEFEAT);
  if(frame%30==0){assert(campaign_save(&resumed,path));memset(&resumed,0,sizeof resumed);assert(campaign_load(&resumed,path));}
 }
 assert(resumed.game.crew[2].health<90&&resumed.game.enemy_crew[GAME_MAX_HOSTILES-1].health<100);assert(resumed.game.enemy_crew[GAME_MAX_HOSTILES-1].id==uninterrupted.game.enemy_crew[GAME_MAX_HOSTILES-1].id);
 assert(campaign_save(&uninterrupted,a)&&campaign_save(&resumed,b));equal_disk(a,b);
 clean(path);clean(a);clean(b);
 puts("Intruder resume: all48 carried records/slot55, complete Crew equality, service hazards/melee and byte-identical continuation through6 disk cold loads passed (constructed fixtures).");
}
