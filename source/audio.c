#include "audio.h"
#include <string.h>
#ifdef AUDIO_BACKEND_TEST
#include "../tests/audio_backend_stubs.h"
#include "audio_stream.h"
#include <stdlib.h>
#elif defined(ARM9)
#include <nds.h>
#include <filesystem.h>
#include <maxmod9.h>
#include <stdlib.h>
#include "audio_stream.h"
#include "audio_assets.h"
#endif
/* Original patterns and a smooth, looping PCM waveform; no PC soundtrack. */
static const short soft_wave[32] __attribute__((aligned(32)))={0,4292,8419,12223,15556,18292,20325,21577,22000,21577,20325,18292,15556,12223,8419,4292,0,-4292,-8419,-12223,-15556,-18292,-20325,-21577,-22000,-21577,-20325,-18292,-15556,-12223,-8419,-4292};
static const unsigned short map_notes[16]={262,330,392,330,294,349,440,349,220,262,330,262,196,247,294,247};
static const unsigned short fight_notes[16]={220,0,262,220,330,0,294,262,196,0,247,196,294,262,247,0};
static struct {
    int enabled,mode,clock,note,fx_ticks,noise_ticks,player_hull,enemy_hull;
    int effect_age[GAME_MAX_EFFECTS];
    int music_active,music_volume,volume_level;
} audio_state;
/* Imported audio is optional at runtime: NitroFS failure leaves the original
 * authored synthesizer available. No file reads or mixing occur in interrupts. */
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
static AudioStream original_stream;
static int originals_ready,original_opened,original_mode=-1,original_outcome;
static int original_age[GAME_MAX_EFFECTS],original_result[GAME_MAX_EFFECTS];
static int16_t *original_samples[AUDIO_SFX_COUNT];
static unsigned original_sample_counts[AUDIO_SFX_COUNT],original_rates[AUDIO_SFX_COUNT];
/* V68 damage-cue baselines: player hull, system bars per side, living crew slots. */
static int track_hull,track_systems[2];
static unsigned char track_alive[GAME_MAX_CREW+GAME_MAX_HOSTILES];
static const char *original_status="SYNTH FALLBACK";
static mm_word original_fill(mm_word length,mm_addr dest,mm_stream_formats format){
 (void)format;audio_stream_fill(&original_stream,(int16_t*)dest,length);return length;
}
static void original_effect(int id,int volume){
 if(originals_ready&&audio_state.enabled&&original_mode!=AUDIO_SILENT&&id>=0&&id<AUDIO_SFX_COUNT)
  audio_stream_effect_rate(&original_stream,original_samples[id],original_sample_counts[id],volume,original_rates[id]);
}
static int system_bars(const Game *g,int side){
 const Ship *s=side?&g->enemy:&g->player;int bars=0;
 for(int i=0;i<GAME_SYSTEM_COUNT;i++)bars+=s->systems[i].health;
 for(int i=0;i<SUB_COUNT;i++)bars+=g->subsystems[side][i].health;
 return bars;
}
/* Returns 1 when a tracked crew slot died since the previous call. */
static int track_crew(const Game *g){
 int died=0;
 for(int i=0;i<GAME_MAX_CREW+GAME_MAX_HOSTILES;i++){
  int enemy=i>=GAME_MAX_CREW,k=enemy?i-GAME_MAX_CREW:i,present=k<(enemy?g->enemy_crew_count:g->crew_count);
  int alive=present&&(enemy?g->enemy_crew[k].health:g->crew[k].health)>0;
  if(track_alive[i]&&present&&!alive)died=1;
  track_alive[i]=(unsigned char)alive;
 }
 return died;
}
/* Original loops (fire, airLeak) play seamlessly; the one-shot warning chime
 * repeats every few seconds while its cause remains. One fixed slot per alarm. */
static void original_alarms(unsigned mask){
 static const unsigned char volume[ALARM_COUNT]={80,90,70,70,70},period[ALARM_COUNT]={0,0,3,3,4};
 for(int a=0;a<ALARM_COUNT;a++){
  int id=a==ALARM_FIRE?AUDIO_SFX_FIRE:a==ALARM_BREACH?AUDIO_SFX_BREACH:AUDIO_SFX_WARNING;
  unsigned count=original_sample_counts[id],span=period[a]*original_rates[id];
  if(!((mask>>a)&1)||!originals_ready||!audio_state.enabled)audio_stream_loop_stop(&original_stream,(unsigned)a);
  else if(!audio_stream_loop_active(&original_stream,(unsigned)a))audio_stream_loop(&original_stream,(unsigned)a,original_samples[id],count,volume[a],original_rates[id],span>count?span-count:0);
 }
}
static void original_start(void){
 if(original_opened)return;
 mm_stream stream={.sampling_rate=AUDIO_STREAM_RATE,.buffer_length=2048,
  .callback=original_fill,.format=MM_STREAM_16BIT_MONO,.timer=MM_TIMER2,.manual=1};
 mmStreamOpen(&stream);original_opened=1;
}
static void original_stop(void){
 if(!originals_ready)return;
 audio_stream_close(&original_stream);audio_stream_stop_effects(&original_stream);original_mode=-1;
 /* Clear queued output immediately on mute, not after the software buffer. */
 if(original_opened){mmStreamVolume(0);mmStreamClose();original_opened=0;}
}
static void original_init(void){
 unsigned budget=0;int loaded=0;audio_stream_init(&original_stream);
 if((TIMER2_CR&TIMER_ENABLE)||!nitroFSInit(NULL))return;
 for(int i=0;i<AUDIO_TRACK_COUNT;i++){
  if(!audio_stream_open(&original_stream,audio_tracks[i].path,audio_tracks[i].bytes,0)){original_status="SYNTH: MUSIC MISSING";return;}
 }
 audio_stream_close(&original_stream);
 for(int i=0;i<AUDIO_SFX_COUNT;i++){
  const AudioAsset *a=&audio_sfx[i];
  if(!a->bytes||(a->bytes&1)||a->bytes>AUDIO_SFX_BUDGET-budget)continue;
  FILE *f=fopen(a->path,"rb");if(!f)continue;
  int16_t *data=malloc(a->bytes);
  if(data&&fread(data,1,a->bytes,f)==a->bytes&&fgetc(f)==EOF){
   original_samples[i]=data;original_sample_counts[i]=a->bytes/2;original_rates[i]=a->rate?a->rate:AUDIO_SAMPLE_RATE;budget+=a->bytes;loaded++;
  }else free(data);
  fclose(f);
 }
 if(loaded!=AUDIO_SFX_COUNT||!mmInitNoSoundbank()){
  for(int i=0;i<AUDIO_SFX_COUNT;i++){free(original_samples[i]);original_samples[i]=NULL;original_sample_counts[i]=0;}
  original_status="SYNTH: AUDIO LOAD FAILED";return;
 }
 originals_ready=1;original_status="ORIGINAL FTL";
}
static void original_update(const Game *game,enum AudioMode mode){
 int previous_mode=original_mode;
 int changed=original_mode!=(int)mode;
 if(changed){
  int track=mode==AUDIO_MENU?AUDIO_TRACK_MENU:mode==AUDIO_MAP?AUDIO_TRACK_MAP:
   mode==AUDIO_COMBAT?AUDIO_TRACK_COMBAT:mode==AUDIO_FINALE?AUDIO_TRACK_FINALE:
   mode==AUDIO_VICTORY?AUDIO_TRACK_VICTORY:-1;
  audio_stream_close(&original_stream);
  if(track>=0&&!audio_stream_open(&original_stream,audio_tracks[track].path,audio_tracks[track].bytes,track!=AUDIO_TRACK_VICTORY))original_status="ORIGINAL: READ FAILED";
  if(mode==AUDIO_SILENT){audio_stream_stop_effects(&original_stream);if(original_opened){mmStreamVolume(0);mmStreamClose();original_opened=0;}}
  else{original_start();mmStreamVolume(audio_state.volume_level==3?127:audio_state.volume_level==2?80:40);}
  original_mode=mode;
  /* Resuming a checkpoint does not replay every already visible projectile. */
  for(int i=0;i<GAME_MAX_EFFECTS;i++){
   original_age[i]=game->effects[i].active?game->effects[i].age:-1;
   original_result[i]=game->effects[i].result;
  }
  track_hull=game->player.hull;track_systems[0]=system_bars(game,0);track_systems[1]=system_bars(game,1);track_crew(game);
 }
 if(mode==AUDIO_SILENT){original_outcome=game->outcome;return;}
 int combat=mode==AUDIO_COMBAT||mode==AUDIO_FINALE,hull_cue=0;
 original_alarms(combat?audio_alarm_mask(game):0);
 if((previous_mode==AUDIO_COMBAT||previous_mode==AUDIO_FINALE)&&original_outcome==GAME_PLAYING&&game->outcome!=GAME_PLAYING&&
    (game->player.hull<=0||game->enemy.hull<=0))original_effect(AUDIO_SFX_EXPLOSION,127);
 original_outcome=game->outcome;
 if(!combat){track_hull=game->player.hull;track_systems[0]=system_bars(game,0);track_systems[1]=system_bars(game,1);track_crew(game);return;}
 for(int i=0;i<GAME_MAX_EFFECTS;i++){
  const GameEffect *fx=&game->effects[i];
  int fresh=fx->active&&(original_age[i]<0||fx->age<original_age[i]);
  if(fresh){
   int id=fx->kind==EFFECT_MISSILE?AUDIO_SFX_MISSILE:fx->kind==EFFECT_ION?AUDIO_SFX_ION:
    fx->kind==EFFECT_BEAM?AUDIO_SFX_BEAM:fx->kind==EFFECT_BOMB?AUDIO_SFX_BOMB:AUDIO_SFX_LASER;
   original_effect(id,95);
  }
  if(fx->active&&fx->result!=EFFECT_PENDING&&(fresh||fx->result!=original_result[i])){
   int id=fx->result==EFFECT_SHIELD?AUDIO_SFX_SHIELD:(fx->result==EFFECT_MISS||fx->result==EFFECT_INTERCEPT)?AUDIO_SFX_MISS:AUDIO_SFX_HULL;
   original_effect(id,100);hull_cue|=id==AUDIO_SFX_HULL;
  }
  original_age[i]=fx->active?fx->age:-1;original_result[i]=fx->result;
 }
 /* V68 one-shots, at most one of each per frame: hull damage without a resolved
  * hit cue (hitHull), lost system bars (sysExplosion), a crew death (death1). */
 int systems0=system_bars(game,0),systems1=system_bars(game,1),died=track_crew(game);
 if(game->player.hull<track_hull&&!hull_cue)original_effect(AUDIO_SFX_HULL,100);
 if(systems0<track_systems[0]||systems1<track_systems[1])original_effect(AUDIO_SFX_SYSTEM,80);
 if(died)original_effect(AUDIO_SFX_DEATH,100);
 track_hull=game->player.hull;track_systems[0]=systems0;track_systems[1]=systems1;
}
#endif
unsigned audio_alarm_mask(const Game *g){
 unsigned mask=0;
 if(!g||g->outcome!=GAME_PLAYING||g->player.hull<=0)return 0;
 for(int i=0;i<g->player.room_count&&i<GAME_MAX_ROOMS;i++){
  if(g->player.rooms[i].fires>0)mask|=1u<<ALARM_FIRE;
  if(g->player.rooms[i].breaches>0)mask|=1u<<ALARM_BREACH;
 }
 if(!g->player.automated&&g->player.oxygen<250)mask|=1u<<ALARM_OXYGEN;
 if(g->player.hull*4<=g->player.max_hull)mask|=1u<<ALARM_HULL;
 for(int i=0;i<g->enemy_crew_count&&i<GAME_MAX_HOSTILES;i++)if(g->enemy_crew[i].ship==0&&g->enemy_crew[i].health>0)mask|=1u<<ALARM_INTRUDER;
 return mask;
}
const char *audio_status(void){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
 return original_status;
#else
 return "SYNTH FALLBACK";
#endif
}
void audio_service(void){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
 if(originals_ready&&original_opened){mmStreamUpdate();if(original_stream.failed)original_status="ORIGINAL: READ FAILED";}
#endif
}
void audio_feedback(int error){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
 original_effect(error?AUDIO_SFX_FAIL:AUDIO_SFX_SELECT,80);
#else
 (void)error;
#endif
}
void audio_jump(void){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
 original_effect(AUDIO_SFX_JUMP,110);
#endif
}
static void stop_channel(int channel){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    soundKill(channel);
#else
    (void)channel;
#endif
}
static int audio_gain(int volume){
    int gain=audio_state.volume_level==3?4:audio_state.volume_level==2?2:1;
    volume*=gain;return volume>127?127:volume<0?0:volume;
}
static void tone(int channel,int hz,int volume,int pan){
    volume=audio_gain(volume);
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    soundPlaySampleChannel(channel,soft_wave,SoundFormat_16Bit,sizeof soft_wave,
                           (u16)(hz*32),(u8)volume,(u8)pan,true,0);
#else
    (void)channel;(void)hz;(void)volume;(void)pan;
#endif
}
static void channel_volume(int channel,int volume){
    volume=audio_gain(volume);
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    soundSetVolume(channel,(u8)volume);
#else
    (void)channel;(void)volume;
#endif
}
static void noise(int hz,int volume){
    volume=audio_gain(volume);
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    soundPlayNoiseChannel(14,(u16)hz,(u8)volume,64);
#else
    (void)hz;(void)volume;
#endif
}
static void stop_all(void){
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
if(originals_ready)return;
#endif
stop_channel(8);stop_channel(9);stop_channel(10);stop_channel(14);}
void audio_init(void){
    memset(&audio_state,0,sizeof audio_state);
    audio_state.enabled=1;audio_state.mode=-1;audio_state.volume_level=3;
    (void)soft_wave;
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    DC_FlushRange(soft_wave,sizeof soft_wave);
    soundEnable();soundSetMasterVolume(127);original_init();
#endif
}
void audio_toggle(void){
    audio_state.enabled=!audio_state.enabled;
    stop_all();audio_state.mode=-1;
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    original_stop();
#endif
}
int audio_enabled(void){return audio_state.enabled;}
int audio_volume_level(void){return audio_state.enabled?audio_state.volume_level:0;}
const char*audio_volume_label(void){static const char*const labels[]={"OFF","LOW","MED","HIGH"};return labels[audio_volume_level()];}
void audio_cycle_volume(void){
    int old_level=audio_volume_level();
    int level=(old_level+1)%4;
    audio_state.enabled=level>0;if(level)audio_state.volume_level=level;
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    if(originals_ready&&old_level>0&&level>0){
        if(original_opened)mmStreamVolume(level==3?127:level==2?80:40);
        return;
    }
#endif
    stop_all();audio_state.mode=-1;
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    original_stop();
#endif
}

void audio_update(const Game *game,enum AudioMode mode){
    int i,new_shot=0,missile=0,pan=64,period;
    if(!game)return;
    if(!audio_state.enabled)mode=AUDIO_SILENT;
#if defined(ARM9) || defined(AUDIO_BACKEND_TEST)
    if(originals_ready){audio_state.mode=mode;original_update(game,mode);return;}
#endif
    if(mode==AUDIO_FINALE)mode=AUDIO_COMBAT;
    if(mode==AUDIO_MENU)mode=AUDIO_MAP;
    if(audio_state.mode!=(int)mode){
        stop_all();audio_state.mode=mode;audio_state.clock=0;audio_state.note=0;
        audio_state.fx_ticks=audio_state.noise_ticks=0;
        audio_state.music_active=audio_state.music_volume=0;
        audio_state.player_hull=game->player.hull;audio_state.enemy_hull=game->enemy.hull;
        for(i=0;i<GAME_MAX_EFFECTS;i++)audio_state.effect_age[i]=game->effects[i].active?game->effects[i].age:-1;
    }
    if(mode==AUDIO_SILENT)return;
    if(audio_state.fx_ticks>0){
        audio_state.fx_ticks--;
        if(!audio_state.fx_ticks)stop_channel(10);
        else channel_volume(10,audio_state.fx_ticks*6);
    }
    if(audio_state.noise_ticks>0){
        audio_state.noise_ticks--;
        if(!audio_state.noise_ticks)stop_channel(14);
        else channel_volume(14,audio_state.noise_ticks*2);
    }
    period=mode==AUDIO_COMBAT?24:36;
    if(!audio_state.clock){
        int hz=mode==AUDIO_COMBAT?fight_notes[audio_state.note%16]:map_notes[audio_state.note%16];
        if(mode==AUDIO_DEFEAT)hz=audio_state.note<4?196-audio_state.note*24:0;
        if(mode==AUDIO_VICTORY)hz=audio_state.note<8?map_notes[audio_state.note]*2:0;
        audio_state.music_active=hz>0;
        if(hz){tone(8,hz,0,52);tone(9,hz/2,0,76);}
        else{stop_channel(8);stop_channel(9);}
        /* End stingers stay silent until the next mode. */
        if(audio_state.note<32)audio_state.note++;
        else if(mode==AUDIO_MAP||mode==AUDIO_COMBAT)audio_state.note=1;
    }
    /* Short attack and long release reduce abrupt note edges. */
    int age=audio_state.clock,volume=0;
    if(audio_state.music_active){
        if(age<3)volume=age*4;
        else if(age<period/2)volume=12;
        else volume=(period-1-age)*12/(period-period/2);
    }
    audio_state.music_volume=volume;
    channel_volume(8,volume);channel_volume(9,volume/2);
    audio_state.clock=(audio_state.clock+1)%period;
    for(i=0;i<GAME_MAX_EFFECTS;i++){
        const GameEffect *fx=&game->effects[i];
        if(mode==AUDIO_COMBAT&&fx->active&&
           (audio_state.effect_age[i]<0||fx->age<audio_state.effect_age[i])){
            new_shot=1;missile|=fx->kind==EFFECT_MISSILE;pan=fx->side?96:32;
        }
        audio_state.effect_age[i]=fx->active?fx->age:-1;
    }
    if(new_shot){tone(10,missile?180:660,18,pan);audio_state.fx_ticks=3;}
    if(mode==AUDIO_COMBAT&&(game->player.hull<audio_state.player_hull||game->enemy.hull<audio_state.enemy_hull)){
        noise(game->player.hull<audio_state.player_hull?500:900,8);audio_state.noise_ticks=4;
    }
    audio_state.player_hull=game->player.hull;audio_state.enemy_hull=game->enemy.hull;
}
