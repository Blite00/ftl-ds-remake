#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../source/audio_stream.h"
int main(void){
 const char *path="audio-stream-test.pcm";int16_t samples[]={1000,-2000,3000};
 FILE *f=fopen(path,"wb");assert(f);assert(fwrite(samples,2,3,f)==3);assert(!fclose(f));
 AudioStream s;int16_t out[20];audio_stream_init(&s);s.music_gain=127;
 assert(audio_stream_open(&s,path,6,1));audio_stream_fill(&s,out,8);
 for(int i=0;i<8;i++)assert(out[i]==samples[i%3]);
 assert(!s.failed&&!s.ended);
 assert(audio_stream_open(&s,path,6,0));audio_stream_fill(&s,out,8);
 for(int i=0;i<8;i++)assert(out[i]==(i<3?samples[i]:0));
 assert(s.ended&&!s.failed);
 audio_stream_fill(&s,out,8);for(int i=0;i<8;i++)assert(!out[i]);
 assert(!audio_stream_open(&s,path,8,1)&&s.failed);assert(!audio_stream_open(&s,path,5,1));
 assert(!audio_stream_open(&s,"audio-does-not-exist.pcm",6,1));
 assert(audio_stream_open(&s,path,6,1));s.music_gain=0;audio_stream_fill(&s,out,8);
 for(int i=0;i<8;i++)assert(!out[i]);
 audio_stream_close(&s);
 int16_t loud[]={32767,32767,32767,32767};
 for(int i=0;i<10;i++)audio_stream_effect(&s,loud,4,127);
 audio_stream_fill(&s,out,20);assert(out[0]==32767);assert(out[19]==0);
 for(int i=0;i<AUDIO_VOICES;i++)assert(!s.voices[i].data);
 audio_stream_effect(&s,samples,3,127);audio_stream_stop_effects(&s);
 audio_stream_fill(&s,out,20);for(int i=0;i<20;i++)assert(!out[i]);
 audio_stream_effect(&s,samples,3,127);s.effect_gain=0;audio_stream_fill(&s,out,20);
 for(int i=0;i<20;i++)assert(!out[i]);
 /* A read failure after a successful open silences the tail and terminates. */
 assert(audio_stream_open(&s,path,6,1));s.remaining=8;s.bytes=8;
 s.music_gain=127;audio_stream_fill(&s,out,8);assert(s.failed&&s.ended);
 assert(out[0]==1000&&out[1]==-2000&&out[2]==3000&&out[3]==0);
 audio_stream_close(&s);assert(!remove(path));
 puts("PCM loop, EOF, validation, short read, mute, bounded voices, resampling and saturation passed.");return 0;
}
