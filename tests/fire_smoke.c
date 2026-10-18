#include "fire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void init(Game *g) {
    int i;memset(g,0,sizeof *g);g->rng=1;g->pilot_room=7;
    g->player.room_count=2;g->enemy.room_count=1;
    for(i=0;i<2;i++) {
        GameRoom *r=&g->player.rooms[i];r->id=i;r->w=r->h=2;r->oxygen=1000;r->system=-1;
    }
    g->enemy.rooms[0]=g->player.rooms[0];g->player.rooms[0].system=SYS_SHIELDS;
    g->player.systems[SYS_SHIELDS].health=g->player.systems[SYS_SHIELDS].power=4;
    g->player.systems[SYS_SHIELDS].capacity=4;
    g->door_count=1;g->doors[0].room_a=0;g->doors[0].room_b=1;
    g->crew_count=1;g->crew[0].room=g->crew[0].destination=1;g->crew[0].health=100;
}
static void step(Game *g,int n,int service){while(n--){g->ticks++;fire_step(g,service);}}
int main(void) {
    Game g,snapshot;int i;
    init(&g);g.subsystems[0][SUB_DOORS].room=1;g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=3;fire_ignite(&g.player,0,999);assert(g.player.rooms[0].fires==4);
    fire_ignite(&g.player,99,1);fire_ignite(&g.player,0,-1);assert(g.player.rooms[0].fires==4);
    step(&g,GAME_HZ,0);assert(g.player.rooms[0].oxygen==900);
    step(&g,2*GAME_HZ,0);assert(g.player.systems[SYS_SHIELDS].health==3&&g.player.systems[SYS_SHIELDS].power==3);
    assert(g.player.rooms[1].fires==0);
    g.paused=1;snapshot=g;fire_step(&g,0);assert(!memcmp(&g,&snapshot,sizeof g));
    g.paused=0;g.outcome=GAME_VICTORY;snapshot=g;fire_step(&g,0);assert(!memcmp(&g,&snapshot,sizeof g));
    g.enemy.rooms[0].fires=1;g.enemy.rooms[0].fire_ticks=12;snapshot=g;
    step(&g,GAME_HZ,1);assert(!memcmp(&g.enemy,&snapshot.enemy,sizeof g.enemy));
    init(&g);g.player.rooms[0].oxygen=0;fire_ignite(&g.player,0,1);assert(!g.player.rooms[0].fires);
    g.player.rooms[0].fires=2;step(&g,2*GAME_HZ,0);assert(!g.player.rooms[0].fires);
    init(&g);g.crew[0].room=g.crew[0].destination=0;fire_ignite(&g.player,0,1);
    step(&g,3*GAME_HZ,0);assert(g.player.rooms[0].fires==0&&g.crew[0].health==96);
    assert(g.player.rooms[0].fire_ticks==0&&g.player.rooms[0].firefight_ticks==0);
    init(&g);g.crew[0].room=0;g.crew[0].destination=1;fire_ignite(&g.player,0,1);
    step(&g,3*GAME_HZ,0);assert(g.player.rooms[0].fires==1&&g.crew[0].health==94);
    init(&g);g.doors[0].open=1;g.crew_count=0;fire_ignite(&g.player,0,1);
    for(i=0;i<120*GAME_HZ&&!g.player.rooms[1].fires;i++) {
        g.player.rooms[0].oxygen=g.player.rooms[1].oxygen=1000;step(&g,1,0);
    }
    assert(g.player.rooms[1].fires>0);
    init(&g);g.enemy_drone.room=0;g.enemy_drone.capacity=g.enemy_drone.health=g.enemy_drone.power=2;
    fire_ignite(&g.enemy,0,1);step(&g,3*GAME_HZ,0);
    assert(g.enemy_drone.health==1&&g.enemy_drone.power==1);
    /* Ignite off the global second boundary: periodic damage still occurs. */
    init(&g);g.ticks=7;fire_ignite(&g.player,0,1);step(&g,3*GAME_HZ,0);
    assert(g.player.systems[SYS_SHIELDS].health==3);
    g.outcome=GAME_DEFEAT;snapshot=g;fire_step(&g,1);assert(!memcmp(&g,&snapshot,sizeof g));
    /* Seeded trials compare basic, reinforced and strongest closed doors.
     * Fire spread remains possible through an open strongest-level door. */
    {int counts[4]={0},level,seed;
     for(level=0;level<4;level++)for(seed=1;seed<=1000;seed++){
        init(&g);g.rng=(unsigned)seed;g.crew_count=0;
        g.subsystems[0][SUB_DOORS].capacity=g.subsystems[0][SUB_DOORS].health=level<3?level+1:3;
        g.doors[0].open=level==3;fire_ignite(&g.player,0,1);step(&g,GAME_HZ,0);
        counts[level]+=g.player.rooms[1].fires>0;
     }
     assert(counts[0]>counts[1]&&counts[1]>0&&counts[2]==0&&counts[3]>counts[0]);
    }
    puts("fire smoke passed");return 0;
}
