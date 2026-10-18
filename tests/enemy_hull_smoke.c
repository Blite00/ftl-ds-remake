/* Source-layout/targeting regression; optional previews are controlled fixtures. */
#include "render.h"
#include "audio.h"
#include "enemy_catalog.h"
#include "enemy_catalog_art.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Game g,before;static Ui u;
static struct {unsigned pre;uint16_t pixels[256*192];unsigned post;} top,bottom;
static void draw(void){before=g;top.pre=bottom.pre=17;top.post=bottom.post=31;render_game(&g,&u,top.pixels,bottom.pixels);assert(!memcmp(&g,&before,sizeof g));assert(top.pre==17&&bottom.pre==17&&top.post==31&&bottom.post==31);}
static void preview(const char*dir,int kind,const char*view){char path[256];snprintf(path,sizeof path,"%s/enemy-hull-%d-%s-v33.ppm",dir,kind,view);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");for(int side=0;side<2;side++)for(int i=0;i<256*192;i++){unsigned v=(side?bottom.pixels:top.pixels)[i];unsigned char p[3]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31};assert(fwrite(p,1,3,f)==3);}assert(!fclose(f));}
int main(int argc,char**argv){
 audio_init();
 for(int kind=0;kind<ENEMY_KIND_COUNT;kind++)for(int phase=0;phase<(kind==ENEMY_FLAGSHIP?3:1);phase++){
  /* The interim authored flagship remains for legacy runs; flagship_smoke covers V42 stages. */
  game_init(&g);memset(&u,0,sizeof u);g.interim_flagship=kind==ENEMY_FLAGSHIP;assert(game_begin_encounter(&g,kind,2,phase));
  int index=game_enemy_layout_index(&g);assert(index>=0&&index<IMPORTED_ENEMY_COUNT);const ImportedEnemy*d=&imported_enemies[index];assert(g.enemy.room_count==d->room_count&&g.enemy_door_count==d->door_count);
  for(int i=0;i<d->room_count;i++){const GameRoom*r=&g.enemy.rooms[i];const ImportedEnemyRoom*s=&d->rooms[i];assert(r->id==s->id&&r->x==s->x&&r->y==s->y&&r->w==s->w&&r->h==s->h);}
  for(int i=0;i<d->door_count;i++){const GameDoor*r=&g.enemy_doors[i];const ImportedEnemyDoor*s=&d->doors[i];assert(r->x==s->x&&r->y==s->y&&r->room_a==s->room_a&&r->room_b==s->room_b&&r->vertical==s->vertical);if(r->room_a>=0&&r->room_b>=0)assert(g.enemy_adjacency[r->room_a][r->room_b]&&g.enemy_adjacency[r->room_b][r->room_a]);}
  if(kind==ENEMY_INTERCEPTOR||kind==ENEMY_ARMORED||kind==ENEMY_FLAGSHIP){assert(game_enemy_import_index(&g)==-1&&g.enemy.automated&&!g.enemy_crew_count&&!g.enemy_drone.capacity);assert(g.enemy.hull==(kind==ENEMY_INTERCEPTOR?9:kind==ENEMY_ARMORED?12:18)+4+phase*3);assert(g.enemy.systems[SYS_WEAPONS].capacity==3&&g.enemy.systems[SYS_ENGINES].capacity==2);for(int s=0;s<3;s++){int matched=0;for(int i=0;i<g.enemy.room_count;i++)if(g.enemy.rooms[i].id==g.enemy.systems[s].room){assert(g.enemy.rooms[i].system==s);matched++;}assert(matched==1);}}
  draw();int matches=0;for(int y=0;y<65;y++)for(int x=0;x<90;x++){unsigned v=imported_enemy_art[index][y*90+x];if(v&&top.pixels[(50+y)*256+156+x]==v)matches++;}assert(matches>20);if(argc>1&&!phase)preview(argv[1],kind,"battle");
  u.aiming=1;u.weapon=0;g.subsystems[0][SUB_SENSORS].capacity=g.subsystems[0][SUB_SENSORS].health=3;
  int minx=1000,miny=1000,maxx=-1000,maxy=-1000;for(int i=0;i<d->room_count;i++){const ImportedEnemyRoom*r=&d->rooms[i];if(r->x<minx)minx=r->x;if(r->y<miny)miny=r->y;if(r->x+r->w>maxx)maxx=r->x+r->w;if(r->y+r->h>maxy)maxy=r->y+r->h;}
  int t=24;if((maxx-minx)*t>232)t=232/(maxx-minx);if((maxy-miny)*t>64)t=64/(maxy-miny);int ox=(256-(maxx-minx)*t)/2-minx*t,oy=44+(64-(maxy-miny)*t)/2-miny*t;
  for(int i=0;i<d->room_count;i++){const ImportedEnemyRoom*r=&d->rooms[i];ui_touch(&g,&u,ox+r->x*t+r->w*t/2,oy+r->y*t+r->h*t/2);assert(g.target_room==r->id&&g.weapons[0].aim_room==r->id);draw();}
  g.target_room=g.enemy.systems[SYS_WEAPONS].room;draw();if(argc>1&&!phase)preview(argv[1],kind,"aim");
 }
 puts("Enemy hulls: every active profile uses original matching hull/rooms/doors; authored strengths retained, source door paths, every touch target, visible source pixels, framebuffer bounds and pure rendering passed.");return 0;
}
