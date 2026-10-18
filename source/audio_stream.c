#include "audio_stream.h"
#include <string.h>
#include <limits.h>
void audio_stream_init(AudioStream *s){memset(s,0,sizeof *s);s->music_gain=80;s->effect_gain=127;}
void audio_stream_close(AudioStream *s){if(s->file)fclose(s->file);s->file=NULL;s->remaining=0;s->ended=1;}
void audio_stream_stop_effects(AudioStream *s){memset(s->voices,0,sizeof s->voices);memset(s->loops,0,sizeof s->loops);}
int audio_stream_open(AudioStream *s,const char *path,unsigned bytes,int loop){
 audio_stream_close(s);s->failed=0;s->bytes=bytes;s->loop=loop!=0;
 if(!path||!bytes||(bytes&1)||bytes>INT_MAX){s->failed=1;return 0;}
 s->file=fopen(path,"rb");if(!s->file){s->failed=1;return 0;}
 if(fseek(s->file,0,SEEK_END)||ftell(s->file)!=(long)bytes||fseek(s->file,0,SEEK_SET)){audio_stream_close(s);s->failed=1;return 0;}
 s->remaining=bytes;s->ended=0;return 1;
}
void audio_stream_effect(AudioStream *s,const int16_t *data,size_t count,int volume){audio_stream_effect_rate(s,data,count,volume,AUDIO_SAMPLE_RATE);}
int audio_stream_loop(AudioStream *s,unsigned slot,const int16_t *data,size_t count,int volume,unsigned rate,unsigned gap){
 if(slot>=AUDIO_LOOPS||!data||!count||volume<=0||s->loops[slot].data)return 0;
 AudioVoice *v=&s->loops[slot];v->data=data;v->count=count;v->position=v->fraction=0;v->volume=volume>127?127:volume;
 v->rate=rate?rate:AUDIO_SAMPLE_RATE;v->gap=gap;v->loop=1;return 1;
}
void audio_stream_loop_stop(AudioStream *s,unsigned slot){if(slot<AUDIO_LOOPS)memset(&s->loops[slot],0,sizeof s->loops[slot]);}
int audio_stream_loop_active(const AudioStream *s,unsigned slot){return slot<AUDIO_LOOPS&&s->loops[slot].data;}
void audio_stream_effect_rate(AudioStream *s,const int16_t *data,size_t count,int volume,unsigned rate){
 if(!data||!count||volume<=0)return;
 unsigned slot=s->next_voice++%AUDIO_VOICES;
 for(unsigned i=0;i<AUDIO_VOICES;i++)if(!s->voices[i].data){slot=i;break;}
 AudioVoice *v=&s->voices[slot];v->data=data;v->count=count;v->position=v->fraction=0;v->volume=volume>127?127:volume;
 v->rate=rate?rate:AUDIO_SAMPLE_RATE;v->gap=0;v->loop=0;
}
/* Adds one voice's interpolated sample; returns 0 while idle or in a loop gap. */
static int voice_mix(AudioVoice *v,int *effects,int gain){
 if(!v->data)return 0;
 if(v->position>=v->count+v->gap){if(!v->loop){v->data=NULL;return 0;}v->position%=v->count+v->gap;}
 int sounding=v->position<v->count;
 if(sounding){
  int value=v->data[v->position];
  if(v->position+1<v->count)value+=((int)v->data[v->position+1]-value)*(int)v->fraction/AUDIO_STREAM_RATE;
  *effects+=value*v->volume/127*gain/127;
 }
 v->fraction+=v->rate?v->rate:AUDIO_SAMPLE_RATE;v->position+=v->fraction/AUDIO_STREAM_RATE;v->fraction%=AUDIO_STREAM_RATE;
 if(!v->loop&&v->position>=v->count)v->data=NULL;
 return sounding;
}
void audio_stream_fill(AudioStream *s,int16_t *out,size_t count){
 size_t done=0;memset(out,0,count*sizeof *out);
 while(done<count&&s->file){
  if(!s->remaining){
   if(!s->loop){audio_stream_close(s);break;}
   if(fseek(s->file,0,SEEK_SET)){s->failed=1;audio_stream_close(s);break;}
   s->remaining=s->bytes;
  }
  size_t take=s->remaining/2;if(take>count-done)take=count-done;
  size_t got=fread(out+done,2,take,s->file);done+=got;s->remaining-=(unsigned)(got*2);
  if(got!=take){s->failed=1;audio_stream_close(s);break;}
 }
 for(size_t i=0;i<count;i++){
  int mixed=(int)out[i]*s->music_gain/127,effects=0,voices=0;
  for(unsigned j=0;j<AUDIO_VOICES;j++)voices+=voice_mix(&s->voices[j],&effects,s->effect_gain);
  for(unsigned j=0;j<AUDIO_LOOPS;j++)voices+=voice_mix(&s->loops[j],&effects,s->effect_gain);
  /* Average overlapping effects and duck music; imported -3dBFS peaks retain
   * headroom even in volleys. Final saturation also protects malformed peaks. */
  if(voices)mixed=mixed/2+effects/voices;
  out[i]=(int16_t)(mixed>32767?32767:mixed< -32768?-32768:mixed);
 }
}
