#include "outcome.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static Game game,before;
int main(void) {
    int reason,outcome,age,index,x,y,r,a,b,c,seen[OUTCOME_SAMPLE_COUNT]={0};
    for(reason=RESULT_NONE;reason<=RESULT_ENEMY_CREW;reason++)for(outcome=GAME_PLAYING;outcome<=GAME_DEFEAT;outcome++) {
        game.result_reason=reason;game.outcome=outcome;game.rng=98765;
        int eligible=(reason==RESULT_PLAYER_HULL&&outcome==GAME_DEFEAT)||(reason==RESULT_ENEMY_HULL&&outcome==GAME_VICTORY);
        assert(outcome_duration(&game)==(eligible?2*GAME_HZ:0));
        assert(outcome_ship(&game)==(eligible?(reason==RESULT_ENEMY_HULL): -1));
        before=game;
        for(age=-1;age<=2*GAME_HZ;age++)for(index=-1;index<=OUTCOME_SAMPLE_COUNT;index++) {
            x=y=r=-999;
            int kind=outcome_sample(&game,age,index,&x,&y,&r);
            if(!eligible||age<0||age>=2*GAME_HZ||index<0||index>=OUTCOME_SAMPLE_COUNT)assert(!kind);
            if(kind) {
                assert(x>=0&&x<=99&&y>=0&&y<=99&&r>=1&&r<=18);
                assert(kind==(index<6?OUTCOME_BLAST:OUTCOME_DEBRIS));
                assert(outcome_sample(&game,age,index,&a,&b,&c)==kind&&a==x&&b==y&&c==r);
                seen[index]=1;
            }else assert(x==-999&&y==-999&&r==-999);
        }
        assert(!memcmp(&before,&game,sizeof game));
    }
    for(index=0;index<OUTCOME_SAMPLE_COUNT;index++)assert(seen[index]);
    assert(!outcome_duration(NULL)&&outcome_ship(NULL)==-1);
    assert(!outcome_sample(NULL,0,0,&x,&y,&r));
    game.outcome=GAME_VICTORY;game.result_reason=RESULT_ENEMY_HULL;
    assert(!outcome_sample(&game,INT_MAX,INT_MAX,&x,&y,&r));
    assert(!outcome_sample(&game,0,0,NULL,&y,&r));
    assert(outcome_sample(&game,0,0,&x,&y,&r)==OUTCOME_BLAST);
    game.paused=1;assert(outcome_sample(&game,0,0,&a,&b,&c)==OUTCOME_BLAST&&a==x&&b==y&&c==r);
    puts("Outcome presentation: hull-only gate, bounded deterministic blasts/debris, endpoints, purity and null guards passed");
    return 0;
}
