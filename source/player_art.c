#include "player_art.h"
#include "game.h"
#include "combat_art.h"
#include "ship_art.h"
#include "interior_art.h"
#include <stdio.h>
#include <string.h>
#ifdef ARM9
#include <filesystem.h>
#include <unistd.h>
#define PLAYER_ART_ROOT "nitro:/"
#else
#define PLAYER_ART_ROOT "nitrofiles/"
#endif
#define INTERIOR_PIXELS 6144
typedef struct { int room,w,h,code,offset; } InteriorRef;
static uint16_t tactical[PLAYER_ART_TACTICAL_W*PLAYER_ART_TACTICAL_H],combat[PLAYER_ART_COMBAT_W*PLAYER_ART_COMBAT_H],hangar[PLAYER_ART_HANGAR_W*PLAYER_ART_HANGAR_H];
static uint16_t interior_pixels[INTERIOR_PIXELS];
static InteriorRef interiors[PLAYER_ART_MAX_INTERIORS];
static int interior_count,loaded=-2,builtin;
static FILE*open_pack(int layout){char path[96];FILE*f;const char*pack=game_layout_pack(layout);
 if(!pack||!*pack)return 0;
 snprintf(path,sizeof path,"%s%s",PLAYER_ART_ROOT,pack);f=fopen(path,"rb");
#ifdef ARM9
 {static int tried;if(!f&&!tried){char home[256];int have=getcwd(home,sizeof home)!=NULL;tried=1;if(nitroFSInit(NULL))f=fopen(path,"rb");if(have)chdir(home);}} /* Keep the SD directory current (see main.c). */
#endif
 return f;}
static int read16(FILE*f,uint16_t*out,int count){unsigned char b[2];for(int i=0;i<count;i++){if(fread(b,1,2,f)!=2)return 0;out[i]=(uint16_t)(b[0]|(b[1]<<8));}return 1;}
static int header(FILE*f){unsigned char m[4];uint16_t size[6];
 if(fread(m,1,4,f)!=4||memcmp(m,"SHP1",4)||!read16(f,size,6))return 0;
 return size[0]==PLAYER_ART_TACTICAL_W&&size[1]==PLAYER_ART_TACTICAL_H&&size[2]==PLAYER_ART_COMBAT_W&&size[3]==PLAYER_ART_COMBAT_H&&size[4]==PLAYER_ART_HANGAR_W&&size[5]==PLAYER_ART_HANGAR_H;}
static int load_pack(int layout){FILE*f=open_pack(layout);uint16_t n;int ok,used=0;
 if(!f)return 0;
 ok=header(f)&&read16(f,tactical,PLAYER_ART_TACTICAL_W*PLAYER_ART_TACTICAL_H)&&read16(f,combat,PLAYER_ART_COMBAT_W*PLAYER_ART_COMBAT_H)&&read16(f,hangar,PLAYER_ART_HANGAR_W*PLAYER_ART_HANGAR_H)&&read16(f,&n,1);
 interior_count=0;
 for(int i=0;ok&&i<n;i++){uint16_t d[4];if(!read16(f,d,4)){ok=0;break;}
  int px=d[2]*d[3];if(i>=PLAYER_ART_MAX_INTERIORS||used+px>INTERIOR_PIXELS){ok=0;break;}
  if(!read16(f,interior_pixels+used,px)){ok=0;break;}
  interiors[interior_count++]=(InteriorRef){d[1],d[2],d[3],d[0],used};used+=px;}
 fclose(f);if(!ok)interior_count=0;return ok;}
/* Kestrel A built-ins (V56 and earlier art), used when NitroFS is unavailable.
 * System codes as in player_ships.h. */
typedef struct { int room,w,h,code; const uint16_t*pixels; } KestrelInterior;
static const KestrelInterior kestrel_interiors[]={
 {INTERIOR_PILOT_ROOM,INTERIOR_PILOT_WIDTH,INTERIOR_PILOT_HEIGHT,5,interior_pilot},{INTERIOR_DOORS_ROOM,INTERIOR_DOORS_WIDTH,INTERIOR_DOORS_HEIGHT,7,interior_doors},
 {INTERIOR_SENSORS_ROOM,INTERIOR_SENSORS_WIDTH,INTERIOR_SENSORS_HEIGHT,6,interior_sensors},{INTERIOR_MEDBAY_ROOM,INTERIOR_MEDBAY_WIDTH,INTERIOR_MEDBAY_HEIGHT,4,interior_medbay},
 {INTERIOR_OXYGEN_ROOM,INTERIOR_OXYGEN_WIDTH,INTERIOR_OXYGEN_HEIGHT,3,interior_oxygen},{INTERIOR_SHIELDS_ROOM,INTERIOR_SHIELDS_WIDTH,INTERIOR_SHIELDS_HEIGHT,0,interior_shields},
 {INTERIOR_ENGINES_ROOM,INTERIOR_ENGINES_WIDTH,INTERIOR_ENGINES_HEIGHT,1,interior_engines},{INTERIOR_WEAPONS_ROOM,INTERIOR_WEAPONS_WIDTH,INTERIOR_WEAPONS_HEIGHT,2,interior_weapons}};
#define KESTREL_INTERIOR_COUNT ((int)(sizeof kestrel_interiors/sizeof kestrel_interiors[0]))
static void use_builtin(void){memcpy(tactical,tactical_hull,sizeof tactical);memcpy(combat,combat_kestrel,sizeof combat);memcpy(hangar,kestrel_art,sizeof hangar);
 interior_count=0;int used=0;
 for(int i=0;i<KESTREL_INTERIOR_COUNT&&i<PLAYER_ART_MAX_INTERIORS;i++){const KestrelInterior*k=&kestrel_interiors[i];int px=k->w*k->h;if(used+px>INTERIOR_PIXELS)break;
  memcpy(interior_pixels+used,k->pixels,(size_t)px*2);interiors[interior_count++]=(InteriorRef){k->room,k->w,k->h,k->code,used};used+=px;}}
int player_art_use(int layout){
 if(layout==loaded)return builtin||loaded>=0;
 builtin=0;
 if(load_pack(layout)){loaded=layout;return 1;}
 if(layout==0){use_builtin();builtin=1;loaded=0;return 1;}
 memset(tactical,0,sizeof tactical);memset(combat,0,sizeof combat);memset(hangar,0,sizeof hangar);interior_count=0;loaded=layout;return 0;}
const uint16_t*player_art_tactical(void){return tactical;}
const uint16_t*player_art_combat(void){return combat;}
const uint16_t*player_art_hangar(void){return hangar;}
const uint16_t*player_art_interior(int room,int*w,int*h,int*code){
 for(int i=0;i<interior_count;i++)if(interiors[i].room==room){*w=interiors[i].w;*h=interiors[i].h;if(code)*code=interiors[i].code;return interior_pixels+interiors[i].offset;}
 return 0;}
const uint16_t*player_art_preview(int layout){static uint16_t preview[PLAYER_ART_HANGAR_W*PLAYER_ART_HANGAR_H];static int shown=-1,ok;
 if(layout!=shown){ok=player_art_hangar_preview(layout,preview);shown=layout;}
 return ok?preview:0;}
int player_art_hangar_preview(int layout,uint16_t*out){FILE*f;int ok;
 if(layout==loaded&&(builtin||loaded>=0)){memcpy(out,hangar,sizeof hangar);return 1;}
 f=open_pack(layout);
 if(!f){if(layout==0){memcpy(out,kestrel_art,sizeof hangar);return 1;}return 0;}
 ok=header(f)&&fseek(f,(long)(PLAYER_ART_TACTICAL_W*PLAYER_ART_TACTICAL_H+PLAYER_ART_COMBAT_W*PLAYER_ART_COMBAT_H)*2,SEEK_CUR)==0&&read16(f,out,PLAYER_ART_HANGAR_W*PLAYER_ART_HANGAR_H);
 fclose(f);return ok;}
