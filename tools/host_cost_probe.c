/* Host cost probe: per-step simulation, full-frame render, audio fill and save
 * costs on this PC. Host timing is not DS timing; the report states how many
 * times slower than this host the DS could be before each budget is exceeded.
 * Build (ucrt64 gcc): gcc -std=c99 -O2 -Isource -Igenerated tools/host_cost_probe.c source/{render,menu,setup,diagnostics,campaign,rewards,game,fire,boarding,breach,subsystems,species,beam,drones,outcome,audio,audio_stream,save}.c */
#include "render.h"
#include "save.h"
#include "audio.h"
#include "audio_stream.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
static double now_us(void){static LARGE_INTEGER f;LARGE_INTEGER t;if(!f.QuadPart)QueryPerformanceFrequency(&f);QueryPerformanceCounter(&t);return (double)t.QuadPart*1e6/(double)f.QuadPart;}
static Campaign c;static uint16_t top[256*192],bottom[256*192];static int16_t pcm[2048],sfx[16000];
static void autopilot(Game*g){int aim=g->artillery_count?g->artillery[0].room:g->enemy.systems[SYS_SHIELDS].room;for(int w=0;w<g->weapon_count;w++)if(game_weapon_powered(g,w)&&g->weapons[w].charge_ticks>=g->weapons[w].cooldown_ticks)game_fire(g,w,aim);}
static void sim(const char*name,int kind,int sector,int phase){
 double total=0,worst=0,batch=0,worst_batch=0;int n=0;
 for(unsigned seed=1;seed<=5;seed++){campaign_init_seed(&c,seed);c.mode=CAM_COMBAT;Game*g=&c.game;game_begin_encounter(g,kind,sector,phase);g->player.hull=1000;
  for(int t=0;t<180*GAME_HZ&&g->outcome==GAME_PLAYING;t++){autopilot(g);double a=now_us();campaign_step(&c);double d=now_us()-a;total+=d;n++;if(d>worst)worst=d;batch+=d;if(t%4==3){if(batch>worst_batch)worst_batch=batch;batch=0;}}}
 printf("%-26s steps %6d  mean %7.2f us  max %8.2f us  worst 4-step batch %8.2f us\n",name,n,total/n,worst,worst_batch);
}
int main(void){
 audio_init();
 puts("HOST COST PROBE (this PC; not DS timing)");
 sim("sector-1 scout",ENEMY_SCOUT,0,0);sim("rebel boarder, sector 5",ENEMY_REBEL_BOARDER,5,0);
 sim("flagship stage 1",ENEMY_FLAGSHIP,7,0);sim("flagship stage 2 (drones)",ENEMY_FLAGSHIP,7,1);sim("flagship stage 3 (surge)",ENEMY_FLAGSHIP,7,2);
 /* Render: busiest combat frame and the map overview. */
 {Ui u={.focus_room=16,.weapon=1};campaign_init_seed(&c,7);c.mode=CAM_COMBAT;game_begin_encounter(&c.game,ENEMY_FLAGSHIP,7,1);for(int t=0;t<40*GAME_HZ;t++)campaign_step(&c);
  double worst=0,total=0;for(int i=0;i<200;i++){double a=now_us();render_campaign(&c,&u,top,bottom);double d=now_us()-a;total+=d;if(d>worst)worst=d;}
  printf("render flagship combat     mean %7.2f us  max %8.2f us\n",total/200,worst);
  campaign_init_seed(&c,7);u=(Ui){0};u.map_overview=1;worst=total=0;for(int i=0;i<200;i++){double a=now_us();render_campaign(&c,&u,top,bottom);double d=now_us()-a;total+=d;if(d>worst)worst=d;}
  printf("render map overview        mean %7.2f us  max %8.2f us\n",total/200,worst);}
 /* Audio: one 2048-sample ring fill with music and four resampled effects. */
 {AudioStream s;audio_stream_init(&s);FILE*f=fopen("nitrofiles/audio/track_finale.pcm","rb");unsigned bytes=0;if(f){fseek(f,0,SEEK_END);bytes=(unsigned)ftell(f);fclose(f);}
  int ok=bytes&&audio_stream_open(&s,"nitrofiles/audio/track_finale.pcm",bytes,1);double worst=0,total=0;
  for(int i=0;i<200;i++){if(i%8==0)for(int v=0;v<AUDIO_VOICES;v++)audio_stream_effect(&s,sfx,16000,100);double a=now_us();audio_stream_fill(&s,pcm,2048);double d=now_us()-a;total+=d;if(d>worst)worst=d;}
  printf("audio fill 2048 samples    mean %7.2f us  max %8.2f us  (music %s)\n",total/200,worst,ok?"streamed":"missing");audio_stream_close(&s);}
 /* Save: serialize, write, verify and rotate backup. */
 {campaign_init_seed(&c,9);double worst=0;for(int i=0;i<20;i++){double a=now_us();campaign_save(&c,"build/cost-probe.sav");double d=now_us()-a;if(d>worst)worst=d;}
  printf("campaign save (host disk)  max %8.2f us\n",worst);remove("build/cost-probe.sav");remove("build/cost-probe.sav.bak");}
 return 0;
}
