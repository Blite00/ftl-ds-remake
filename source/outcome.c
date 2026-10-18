#include "outcome.h"

int outcome_ship(const Game *g) {
    if(!g)return -1;
    if(g->outcome==GAME_DEFEAT&&g->result_reason==RESULT_PLAYER_HULL)return 0;
    if(g->outcome==GAME_VICTORY&&g->result_reason==RESULT_ENEMY_HULL)return 1;
    return -1;
}
int outcome_duration(const Game *g) {return outcome_ship(g)<0?0:2*GAME_HZ;}
static int clamp(int value) {return value<0?0:value>99?99:value;}
int outcome_sample(const Game *g,int age,int index,int *x,int *y,int *radius) {
    static const unsigned char centers[6][2]={{50,48},{28,32},{70,59},{35,72},{76,27},{48,45}};
    static const signed char drift[6][2]={{-2,-1},{1,-2},{2,1},{-1,2},{-2,1},{2,-1}};
    int local,r,slot;
    if(!x||!y||!radius||age<0||age>=outcome_duration(g)||index<0||index>=OUTCOME_SAMPLE_COUNT)return OUTCOME_INACTIVE;
    slot=index%6;
    if(index<6) {
        local=age-index*GAME_HZ/5;
        if(local<0||local>=2*GAME_HZ/3)return OUTCOME_INACTIVE;
        /* Expand quickly, then contract; integer-only and independent of RNG. */
        r=local<GAME_HZ/3?3+local*15/(GAME_HZ/3):18-(local-GAME_HZ/3)*17/(GAME_HZ/3);
        *x=centers[slot][0];*y=centers[slot][1];*radius=r;
        return OUTCOME_BLAST;
    }
    local=age-GAME_HZ/2-slot*2;
    if(local<0)return OUTCOME_INACTIVE;
    *x=clamp((int)centers[slot][0]+drift[slot][0]*local/2);
    *y=clamp((int)centers[slot][1]+drift[slot][1]*local/2);
    *radius=age<3*GAME_HZ/2?2:1;
    return OUTCOME_DEBRIS;
}
