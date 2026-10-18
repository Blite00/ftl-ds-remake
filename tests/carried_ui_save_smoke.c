#include "save.h"
#include "render.h"
#include "species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static struct {unsigned before;uint16_t pixels[256*192];unsigned after;} top,bottom;
static uint16_t plain[256*192];
static unsigned char saved[8192],after[8192];
static size_t read_file(const char*p,unsigned char*b){FILE*f=fopen(p,"rb");assert(f);size_t n=fread(b,1,8192,f);assert(feof(f)&&!fclose(f));return n;}
static void ppm(const char*p){FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");for(int s=0;s<2;s++)for(int i=0;i<256*192;i++){unsigned v=(s?bottom.pixels:top.pixels)[i];unsigned char rgb[3]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31};assert(fwrite(rgb,1,3,f)==3);}assert(!fclose(f));}
int main(int argc,char**argv){
 const char *path="tests/carried-ui-save.sav";Ui u={0};
 campaign_init_seed(&c,281);assert(campaign_save(&c,path));size_t n=read_file(path,saved);
 game_begin_encounter(&c.game,ENEMY_REBEL,0,0);
 c.game.enemy_crew[1].ship=0;c.game.enemy_crew[1].room=c.game.enemy_crew[1].destination=c.game.player.systems[SYS_OXYGEN].room;
 assert(game_carry_intruders(&c.game));
 /* A last-slot fixture exercises every renderer loop beyond mask width32. */
 c.game.enemy_crew[GAME_MAX_HOSTILES-1]=c.game.enemy_crew[GAME_MAX_ENEMY_CREW];
 memset(&c.game.enemy_crew[GAME_MAX_ENEMY_CREW],0,sizeof(Crew));c.game.enemy_crew_count=GAME_MAX_HOSTILES;
 Crew*intruder=&c.game.enemy_crew[GAME_MAX_HOSTILES-1];unsigned id=intruder->id;assert(id&&intruder->health>0);
 memcpy(intruder->name,"OLD BOARDER",12);
 assert(campaign_save(&c,path));assert(campaign_load(&d,path));
 assert(d.game.enemy_crew_count==GAME_MAX_HOSTILES&&!memcmp(intruder,&d.game.enemy_crew[GAME_MAX_HOSTILES-1],sizeof *intruder));
 assert(n==read_file("tests/carried-ui-save.sav.bak",after)&&!memcmp(saved,after,n));
 top.before=bottom.before=0x12345678;top.after=bottom.after=0x87654321;
 render_game(&c.game,&u,top.pixels,bottom.pixels);memcpy(plain,bottom.pixels,sizeof plain);
 int health=intruder->health;intruder->health=0;render_game(&c.game,&u,top.pixels,bottom.pixels);
 assert(memcmp(plain,bottom.pixels,sizeof plain));intruder->health=health;
 u.boarding=1;u.board_side=0;u.board_room=intruder->room;render_game(&c.game,&u,top.pixels,bottom.pixels);
 assert(top.before==0x12345678&&bottom.before==0x12345678&&top.after==0x87654321&&bottom.after==0x87654321);
 assert(intruder->id==id&&intruder->health==health);if(argc>1){assert(d.game.enemy_departed&&!d.game.enemy.hull);render_game(&d.game,&u,top.pixels,bottom.pixels);ppm(argv[1]);}
 /* Preserve a pending home death too; fully processed dead intruders need no
    record. Unpromoted hostiles still reject without changing a checkpoint. */
 intruder->health=0;intruder->death_processed=0;assert(campaign_save(&c,path)&&campaign_load(&d,path));assert(!d.game.enemy_crew[GAME_MAX_HOSTILES-1].health&&!d.game.enemy_crew[GAME_MAX_HOSTILES-1].death_processed&&d.game.enemy_crew[GAME_MAX_HOSTILES-1].id==id);
 intruder->death_processed=1;assert(campaign_save(&c,path));
 assert(campaign_load(&d,path)&&!d.game.enemy_crew_count);
 n=read_file(path,saved);c.game.enemy_crew[0].ship=0;assert(!campaign_save(&c,path));assert(n==read_file(path,after)&&!memcmp(saved,after,n));c.game.enemy_crew[0].ship=1;
 c.game.enemy_crew_count=GAME_MAX_HOSTILES+1;assert(!campaign_save(&c,path));
 c.game.enemy_crew_count=-1;assert(!campaign_save(&c,path));
 assert(!remove(path)&&!remove("tests/carried-ui-save.sav.bak"));
 puts("Carried intruders UI/save: last slot visible in bounded framebuffers; rendering preserves identity; save21 preserves live/pending-death carried crew; unpromoted invaders reject without changing existing checkpoint");
 return 0;
}
