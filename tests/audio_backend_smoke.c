#define AUDIO_BACKEND_TEST
#include <assert.h>
#include <stdio.h>
#include "../source/audio.c"
static int cue(int id){for(int i=0;i<AUDIO_VOICES;i++)if(original_stream.voices[i].data==original_samples[id])return 1;return 0;}
int main(void){
 int16_t pcm[4]={1000,2000,-1000,-2000};FILE *f=fopen("audio-backend-test.pcm","wb");assert(f);assert(fwrite(pcm,2,4,f)==4);fclose(f);
 Game g;memset(&g,0,sizeof g);g.player.hull=g.enemy.hull=20;
 audio_init();assert(originals_ready&&!strcmp(audio_status(),"ORIGINAL FTL"));
 audio_update(&g,AUDIO_MENU);assert(stub_open==1&&original_stream.file&&stub_volume==127);
 audio_update(&g,AUDIO_COMBAT);assert(original_stream.loop);
 g.effects[0]=(GameEffect){.active=1,.kind=EFFECT_ION};audio_update(&g,AUDIO_COMBAT);assert(cue(AUDIO_SFX_ION));
 audio_stream_stop_effects(&original_stream);audio_update(&g,AUDIO_COMBAT);assert(!cue(AUDIO_SFX_ION));
 g.effects[0].result=EFFECT_SHIELD;audio_update(&g,AUDIO_COMBAT);assert(cue(AUDIO_SFX_SHIELD));
 audio_stream_stop_effects(&original_stream);audio_update(&g,AUDIO_COMBAT);assert(!cue(AUDIO_SFX_SHIELD));
 g.effects[0].result=EFFECT_INTERCEPT;audio_update(&g,AUDIO_COMBAT);assert(cue(AUDIO_SFX_MISS));
 g.enemy.hull=0;g.outcome=GAME_VICTORY;audio_update(&g,AUDIO_VICTORY);assert(cue(AUDIO_SFX_EXPLOSION)&&!original_stream.loop);
 audio_stream_stop_effects(&original_stream);audio_update(&g,AUDIO_VICTORY);assert(!cue(AUDIO_SFX_EXPLOSION));
 audio_feedback(0);assert(cue(AUDIO_SFX_SELECT));audio_feedback(1);assert(cue(AUDIO_SFX_FAIL));audio_jump();assert(cue(AUDIO_SFX_JUMP));
 audio_cycle_volume();assert(!audio_enabled()&&!stub_open&&!stub_volume&&!original_stream.file&&stub_kills==0);
 audio_cycle_volume();audio_update(&g,AUDIO_MENU);assert(audio_volume_level()==1&&stub_open==1);
 FILE *same=original_stream.file;original_stream.remaining=4;
 audio_cycle_volume();assert(audio_volume_level()==2&&original_stream.file==same&&original_stream.remaining==4);
 audio_cycle_volume();assert(audio_volume_level()==3&&original_stream.remaining==4&&stub_kills==0);
 audio_update(&g,AUDIO_SILENT);assert(!stub_open);audio_feedback(0);assert(!cue(AUDIO_SFX_SELECT));
 audio_update(&g,AUDIO_FINALE);assert(stub_open==1&&original_stream.loop);audio_toggle();assert(!stub_open&&!stub_volume&&stub_kills==0);
 for(int i=0;i<AUDIO_SFX_COUNT;i++)free(original_samples[i]);
 assert(!remove("audio-backend-test.pcm"));
 puts("Original backend cue edges, impact mapping, destruction, menu/finale, UI cues, mute and volume continuity passed with SDK stubs; hardware not tested.");return 0;
}
