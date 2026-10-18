/* Include the scheduler to assert bounded durations without pretending that
 * silent host wrappers validate hardware sound. Do not link audio.c twice. */
#include <assert.h>
#include <stdio.h>
#include "../source/audio.c"
int main(void){
    Game g; int i;
    memset(&g,0,sizeof g);g.player.hull=g.enemy.hull=20;
    audio_init();assert(audio_enabled()&&audio_volume_level()==3&&!strcmp(audio_volume_label(),"HIGH"));
    assert(audio_gain(12)==48&&audio_gain(18)==72&&audio_gain(100)==127);
    audio_cycle_volume();assert(!audio_enabled()&&!strcmp(audio_volume_label(),"OFF"));
    audio_cycle_volume();assert(audio_volume_level()==1&&audio_gain(12)==12);
    audio_cycle_volume();assert(audio_volume_level()==2&&audio_gain(12)==24);
    audio_cycle_volume();assert(audio_volume_level()==3&&audio_gain(12)==48);audio_update(&g,AUDIO_COMBAT);
    assert(audio_state.music_volume==0);
    audio_update(&g,AUDIO_COMBAT);assert(audio_state.music_volume==4);
    audio_update(&g,AUDIO_COMBAT);assert(audio_state.music_volume==8);
    audio_update(&g,AUDIO_COMBAT);assert(audio_state.music_volume==12);
    for(i=0;i<32;i++)assert(soft_wave[i]>=-22000&&soft_wave[i]<=22000);
    assert(soft_wave[0]==0&&soft_wave[8]==22000&&soft_wave[16]==0&&soft_wave[24]==-22000);
    g.effects[0].active=1;g.effects[0].kind=EFFECT_LASER;
    audio_update(&g,AUDIO_COMBAT);assert(audio_state.fx_ticks==3);
    for(i=0;i<3;i++)audio_update(&g,AUDIO_COMBAT);
    assert(audio_state.fx_ticks==0); /* Paused projectile age cannot retrigger. */
    g.player.hull--;audio_update(&g,AUDIO_COMBAT);assert(audio_state.noise_ticks==4);
    for(i=0;i<4;i++)audio_update(&g,AUDIO_COMBAT);
    assert(!audio_state.noise_ticks);
    audio_toggle();assert(!audio_enabled());audio_update(&g,AUDIO_COMBAT);
    assert(audio_state.mode==AUDIO_SILENT);
    audio_toggle();audio_update(&g,AUDIO_VICTORY);
    for(i=0;i<3000;i++)audio_update(&g,AUDIO_VICTORY);
    assert(audio_state.note==32&&!audio_state.music_volume); /* End stinger never restarts itself. */
    for(i=0;i<3000;i++)audio_update(&g,AUDIO_MAP);
    assert(audio_state.note<=32&&audio_state.note>0);
    puts("Audio envelope, waveform bounds, scheduling and stale-effect checks passed; hardware output not tested.");return 0;
}
