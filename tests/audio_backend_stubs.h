#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef uint16_t u16;typedef uint8_t u8;
typedef unsigned mm_word;typedef void *mm_addr;typedef int mm_stream_formats;
typedef struct {unsigned sampling_rate,buffer_length;mm_word (*callback)(mm_word,mm_addr,mm_stream_formats);int format,timer,manual;} mm_stream;
#define TIMER2_CR 0
#define TIMER_ENABLE 128
#define MM_STREAM_16BIT_MONO 2
#define MM_TIMER2 2
#define SoundFormat_16Bit 1
static int stub_volume,stub_open,stub_kills,stub_nitro=1;
static inline bool nitroFSInit(const char *p){(void)p;return stub_nitro!=0;}
static inline bool mmInitNoSoundbank(void){return true;}
static inline void mmStreamOpen(mm_stream *s){(void)s;stub_open++;}
static inline void mmStreamClose(void){stub_open--;}
static inline void mmStreamVolume(int v){stub_volume=v;}
static inline void mmStreamUpdate(void){}
static inline void soundKill(int c){(void)c;stub_kills++;}
static inline void soundPlaySampleChannel(int c,const void*d,int f,size_t n,u16 r,u8 v,u8 p,bool l,int o){(void)c;(void)d;(void)f;(void)n;(void)r;(void)v;(void)p;(void)l;(void)o;}
static inline void soundSetVolume(int c,u8 v){(void)c;(void)v;}
static inline void soundPlayNoiseChannel(int c,u16 f,u8 v,int p){(void)c;(void)f;(void)v;(void)p;}
static inline void DC_FlushRange(const void *p,size_t n){(void)p;(void)n;}
static inline void soundEnable(void){}
static inline void soundSetMasterVolume(int v){(void)v;}
enum {AUDIO_TRACK_MENU,AUDIO_TRACK_MAP,AUDIO_TRACK_COMBAT,AUDIO_TRACK_FINALE,AUDIO_TRACK_VICTORY,AUDIO_TRACK_COUNT};
enum {AUDIO_SFX_LASER,AUDIO_SFX_MISSILE,AUDIO_SFX_ION,AUDIO_SFX_BEAM,AUDIO_SFX_BOMB,AUDIO_SFX_HULL,AUDIO_SFX_SHIELD,AUDIO_SFX_MISS,AUDIO_SFX_EXPLOSION,AUDIO_SFX_JUMP,AUDIO_SFX_SELECT,AUDIO_SFX_FAIL,AUDIO_SFX_SYSTEM,AUDIO_SFX_DEATH,AUDIO_SFX_FIRE,AUDIO_SFX_BREACH,AUDIO_SFX_WARNING,AUDIO_SFX_COUNT};
typedef struct {const char *path;unsigned bytes,rate;} AudioAsset;
#define TEST_ASSET {"audio-backend-test.pcm",8,0}
static const AudioAsset audio_tracks[AUDIO_TRACK_COUNT]={TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET};
static const AudioAsset audio_sfx[AUDIO_SFX_COUNT]={TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,
 TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET,TEST_ASSET};
