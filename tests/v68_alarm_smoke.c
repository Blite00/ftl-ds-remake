/* V68: original alarm loops and damage one-shots, driven only by Game state. */
#define AUDIO_BACKEND_TEST
#include <assert.h>
#include <stdio.h>
#include "../source/audio.c"
static int uses(int id){int n=0;for(int i=0;i<AUDIO_VOICES;i++)n+=original_stream.voices[i].data==original_samples[id];for(int i=0;i<AUDIO_LOOPS;i++)n+=original_stream.loops[i].data==original_samples[id];return n;}
static int shots(int id){int n=0;for(int i=0;i<AUDIO_VOICES;i++)n+=original_stream.voices[i].data==original_samples[id];return n;}
static int loops(void){int n=0;for(int i=0;i<AUDIO_LOOPS;i++)n+=original_stream.loops[i].data!=NULL;return n;}
static int on(int alarm,int id){return audio_stream_loop_active(&original_stream,(unsigned)alarm)&&original_stream.loops[alarm].data==original_samples[id];}
static void quiet(void){memset(original_stream.voices,0,sizeof original_stream.voices);}
/* Cause appears -> one loop; held cause never restarts or stacks; cleared -> stops. */
static void alarm_cycle(Game*g,int alarm,int id,int*cause,int value,int clear){
 *cause=value;audio_update(g,AUDIO_COMBAT);assert(on(alarm,id)&&loops()==1);
 original_stream.loops[alarm].position=3;
 for(int i=0;i<5;i++){audio_update(g,AUDIO_COMBAT);assert(on(alarm,id)&&original_stream.loops[alarm].position==3&&loops()==1);}
 *cause=clear;audio_update(g,AUDIO_COMBAT);assert(!audio_stream_loop_active(&original_stream,(unsigned)alarm)&&!loops());
}
int main(void){
 int16_t pcm[4]={1000,2000,-1000,-2000};FILE *f=fopen("audio-backend-test.pcm","wb");assert(f);assert(fwrite(pcm,2,4,f)==4);fclose(f);
 /* Mixer: a looping voice repeats after its silent gap at its own rate; a busy slot is kept. */
 {AudioStream s;int16_t d[2]={1000,1000},out[8];audio_stream_init(&s);
  assert(audio_stream_loop(&s,0,d,2,127,AUDIO_STREAM_RATE,2)&&!audio_stream_loop(&s,0,d,2,127,AUDIO_STREAM_RATE,2));
  audio_stream_fill(&s,out,8);for(int i=0;i<8;i++)assert(out[i]==(i%4<2?1000:0));
  assert(audio_stream_loop_active(&s,0));audio_stream_loop_stop(&s,0);assert(!audio_stream_loop_active(&s,0));
  assert(!audio_stream_loop(&s,AUDIO_LOOPS,d,2,127,0,0));
  audio_stream_loop(&s,1,d,2,127,0,0);audio_stream_stop_effects(&s);assert(!audio_stream_loop_active(&s,1));}
 Game g;memset(&g,0,sizeof g);
 g.player.hull=g.player.max_hull=30;g.player.oxygen=1000;g.player.room_count=2;g.player.rooms[1].id=1;g.enemy.hull=20;
 g.crew_count=2;g.crew[0].health=g.crew[1].health=100;g.player.systems[SYS_SHIELDS].health=2;g.enemy.systems[SYS_WEAPONS].health=3;
 /* Pure causes. */
 assert(!audio_alarm_mask(&g)&&!audio_alarm_mask(NULL));
 audio_init();assert(originals_ready&&original_rates[AUDIO_SFX_WARNING]==AUDIO_SAMPLE_RATE);
 audio_update(&g,AUDIO_MENU);audio_update(&g,AUDIO_COMBAT);assert(!loops()&&!shots(AUDIO_SFX_DEATH)&&!shots(AUDIO_SFX_SYSTEM));
 alarm_cycle(&g,ALARM_FIRE,AUDIO_SFX_FIRE,&g.player.rooms[1].fires,2,0);
 alarm_cycle(&g,ALARM_BREACH,AUDIO_SFX_BREACH,&g.player.rooms[0].breaches,1,0);
 alarm_cycle(&g,ALARM_OXYGEN,AUDIO_SFX_WARNING,&g.player.oxygen,249,250);
 g.player.oxygen=200;audio_update(&g,AUDIO_COMBAT);assert(original_stream.loops[ALARM_OXYGEN].gap==3*AUDIO_SAMPLE_RATE-4&&original_stream.loops[ALARM_OXYGEN].loop);
 g.player.oxygen=1000;audio_update(&g,AUDIO_COMBAT);assert(!loops());
 /* Hull at 25% (original warning_hull_25 "HULL CRITICAL"); the drop itself is one hitHull. */
 quiet();g.player.hull=8;audio_update(&g,AUDIO_COMBAT);assert(!(audio_alarm_mask(&g)&(1u<<ALARM_HULL))&&shots(AUDIO_SFX_HULL)==1);
 quiet();audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_HULL));
 alarm_cycle(&g,ALARM_HULL,AUDIO_SFX_WARNING,&g.player.hull,7,30);
 quiet();
 /* Intruders: one loop while any living boarder is aboard; the kill is one death1. */
 g.enemy_crew_count=1;g.enemy_crew[0].ship=1;g.enemy_crew[0].health=100;audio_update(&g,AUDIO_COMBAT);assert(!loops());
 alarm_cycle(&g,ALARM_INTRUDER,AUDIO_SFX_WARNING,&g.enemy_crew[0].ship,0,1);
 g.enemy_crew[0].ship=0;audio_update(&g,AUDIO_COMBAT);assert(on(ALARM_INTRUDER,AUDIO_SFX_WARNING));quiet();
 g.enemy_crew[0].health=0;audio_update(&g,AUDIO_COMBAT);assert(!loops()&&shots(AUDIO_SFX_DEATH)==1);
 quiet();for(int i=0;i<3;i++){audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_DEATH));}
 /* Removing a slot is not a death; a player crew death is exactly one cue. */
 g.enemy_crew_count=0;audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_DEATH));
 g.crew[1].health=0;audio_update(&g,AUDIO_COMBAT);assert(shots(AUDIO_SFX_DEATH)==1);quiet();audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_DEATH));
 /* System damage on either ship: one sysExplosion per frame, even with two systems hit. */
 g.player.systems[SYS_SHIELDS].health=1;g.enemy.systems[SYS_WEAPONS].health=1;audio_update(&g,AUDIO_COMBAT);assert(shots(AUDIO_SFX_SYSTEM)==1);
 quiet();audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_SYSTEM));
 g.player.systems[SYS_SHIELDS].health=2;audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_SYSTEM));
 /* Resolved hits: shield hit once; a hull hit and its hull loss in one frame are one hitHull. */
 g.effects[0]=(GameEffect){.active=1,.side=1,.kind=EFFECT_LASER,.age=1,.duration=12};audio_update(&g,AUDIO_COMBAT);quiet();
 g.effects[0].result=EFFECT_SHIELD;audio_update(&g,AUDIO_COMBAT);assert(shots(AUDIO_SFX_SHIELD)==1);
 quiet();audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_SHIELD));
 g.effects[1]=(GameEffect){.active=1,.side=1,.kind=EFFECT_LASER,.age=1,.duration=12};audio_update(&g,AUDIO_COMBAT);quiet();
 g.effects[1].result=EFFECT_HIT;g.player.hull=28;audio_update(&g,AUDIO_COMBAT);assert(shots(AUDIO_SFX_HULL)==1);
 quiet();audio_update(&g,AUDIO_COMBAT);assert(!shots(AUDIO_SFX_HULL));
 g.effects[0].active=g.effects[1].active=0;audio_update(&g,AUDIO_COMBAT);quiet();
 /* Concurrent alarms keep separate slots; the shared warning chime is one voice per type. */
 g.player.rooms[0].fires=1;g.player.oxygen=100;g.enemy_crew_count=1;g.enemy_crew[0].ship=0;g.enemy_crew[0].health=50;
 for(int i=0;i<4;i++)audio_update(&g,AUDIO_FINALE);
 assert(loops()==3&&on(ALARM_FIRE,AUDIO_SFX_FIRE)&&on(ALARM_OXYGEN,AUDIO_SFX_WARNING)&&on(ALARM_INTRUDER,AUDIO_SFX_WARNING)&&uses(AUDIO_SFX_FIRE)==1&&uses(AUDIO_SFX_WARNING)==2);
 /* Leaving combat stops every loop; returning with the causes restarts each once. */
 audio_update(&g,AUDIO_MAP);assert(!loops());audio_update(&g,AUDIO_COMBAT);assert(loops()==3);
 /* Results end alarms (and victory/defeat modes would too). */
 g.outcome=GAME_VICTORY;audio_update(&g,AUDIO_COMBAT);assert(!loops());g.outcome=GAME_PLAYING;audio_update(&g,AUDIO_COMBAT);assert(loops()==3);
 /* Silent mode, mute and volume OFF all clear the loops; unmuting resumes them. */
 audio_update(&g,AUDIO_SILENT);assert(!loops());audio_update(&g,AUDIO_COMBAT);assert(loops()==3);
 audio_toggle();assert(!loops());for(int i=0;i<3;i++){audio_update(&g,AUDIO_COMBAT);assert(!loops());}
 audio_toggle();audio_update(&g,AUDIO_COMBAT);assert(loops()==3);
 audio_cycle_volume();assert(!audio_enabled()&&!loops());audio_update(&g,AUDIO_COMBAT);assert(!loops());
 audio_cycle_volume();audio_update(&g,AUDIO_COMBAT);assert(audio_volume_level()==1&&loops()==3);
 audio_cycle_volume();audio_update(&g,AUDIO_COMBAT);assert(audio_volume_level()==2&&loops()==3);
 /* Clearing every cause leaves no voice behind. */
 g.player.rooms[0].fires=0;g.player.oxygen=1000;g.enemy_crew[0].health=0;quiet();audio_update(&g,AUDIO_COMBAT);assert(!loops()&&shots(AUDIO_SFX_DEATH)==1);
 audio_toggle();for(int i=0;i<AUDIO_SFX_COUNT;i++)free(original_samples[i]);
 assert(!remove("audio-backend-test.pcm"));
 puts("V68 alarms: fire/breach/O2/hull/intruder loops start once, never stack, stop on clear, combat exit, result, silence and mute; hit/shield/system/death one-shots fire once per event.");return 0;
}
