#ifndef FTL_AUDIO_STREAM_H
#define FTL_AUDIO_STREAM_H
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
/* Bounded PCM reader/mixer; all calls run in the main loop, never IRQ context. */
#define AUDIO_VOICES 4
#define AUDIO_STREAM_RATE 22050
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_SFX_BUDGET (768u*1024u)
/* V68 alarm voices: one fixed slot per alarm type, so a cue can never stack. */
#define AUDIO_LOOPS 5
/* rate 0 means AUDIO_SAMPLE_RATE; looping voices repeat after gap silent samples. */
typedef struct { const int16_t *data; size_t count; unsigned position, fraction; int volume; unsigned rate, gap; int loop; } AudioVoice;
typedef struct {
 FILE *file; unsigned bytes, remaining; int loop, failed, ended, music_gain, effect_gain;
 AudioVoice voices[AUDIO_VOICES]; unsigned next_voice;
 AudioVoice loops[AUDIO_LOOPS];
} AudioStream;
void audio_stream_init(AudioStream *s);
int audio_stream_open(AudioStream *s,const char *path,unsigned bytes,int loop);
void audio_stream_close(AudioStream *s);
void audio_stream_stop_effects(AudioStream *s);
void audio_stream_effect(AudioStream *s,const int16_t *data,size_t count,int volume);
void audio_stream_effect_rate(AudioStream *s,const int16_t *data,size_t count,int volume,unsigned rate);
/* Starts slot only when idle (returns 1); an active slot keeps playing untouched. */
int audio_stream_loop(AudioStream *s,unsigned slot,const int16_t *data,size_t count,int volume,unsigned rate,unsigned gap);
void audio_stream_loop_stop(AudioStream *s,unsigned slot);
int audio_stream_loop_active(const AudioStream *s,unsigned slot);
void audio_stream_fill(AudioStream *s,int16_t *out,size_t count);
#endif
