/* One focused native smoke check. From workspace in PowerShell:
 * $env:PATH="$PWD\.tools\msys64\ucrt64\bin;$env:PATH"
 * & .\.tools\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wextra -Werror -Isource -Igenerated tests/smoke.c source/game.c -o tests/smoke.exe
 * & .\tests\smoke.exe
 */
#include "game.h"
#include <assert.h>
#include <stdio.h>

static void advance(Game *g,int ticks) {
    while(ticks-->0) game_step(g);
}
int main(void) {
    Game g;
    int i,missiles,combat_ticks;
    GameSystemState *weapons;
    game_init(&g);
    assert(g.player.hull==30 && g.player.reactor==8);
    assert(g.player.room_count==17 && g.missiles==8);
    assert(g.weapons[0].cooldown_ticks==11*GAME_HZ);
    assert(g.weapons[0].damage==2 && g.weapons[0].missiles==1);
    assert(g.weapons[1].cooldown_ticks==12*GAME_HZ);
    assert(g.weapons[1].shots==3 && g.weapons[1].damage==1);
    assert(game_used_power(&g)==8);
    assert(g.enemy.automated && g.enemy.room_count==5 && g.enemy.hull==6);
    assert(g.enemy.systems[SYS_WEAPONS].room==4 && g.target_room==4);
    assert(g.enemy.systems[SYS_OXYGEN].room==-1 && g.enemy.oxygen==0);
    assert(g.enemy.systems[SYS_SHIELDS].capacity==0 && g.enemy.shields==0);
    game_toggle_pause(&g);
    advance(&g,60);
    assert(g.ticks==0 && g.weapons[0].charge_ticks==0);
    game_toggle_pause(&g);
    /* Isolate crew and power checks from enemy damage. */
    g.enemy.systems[SYS_WEAPONS].capacity=0;
    g.enemy.systems[SYS_WEAPONS].health=0;
    g.enemy.systems[SYS_WEAPONS].power=0;
    assert(game_order_crew(&g,0,16));
    advance(&g,10*GAME_HZ);
    assert(g.crew[0].room==16);
    assert(!game_order_crew(&g,0,999));
    weapons=&g.player.systems[SYS_WEAPONS];
    weapons->health--; weapons->power--;
    assert(game_order_crew(&g,0,weapons->room));
    advance(&g,15*GAME_HZ);
    assert(g.crew[0].room==weapons->room);
    assert(weapons->health==weapons->capacity);
    assert(game_adjust_power(&g,SYS_WEAPONS,1));
    assert(!game_adjust_power(&g,SYS_WEAPONS,1));
    assert(game_used_power(&g)==g.player.reactor);
    assert(game_adjust_power(&g,SYS_OXYGEN,-1));
    assert(!game_adjust_power(&g,SYS_OXYGEN,-1));
    assert(game_adjust_power(&g,SYS_OXYGEN,1));
    advance(&g,12*GAME_HZ);
    assert(g.weapons[0].charge_ticks==g.weapons[0].cooldown_ticks);
    missiles=g.missiles;
    assert(game_fire(&g,0,g.enemy.systems[SYS_WEAPONS].room));
    assert(g.missiles==missiles-1 && g.weapons[0].charge_ticks==0);
    assert(!game_fire(&g,0,g.enemy.systems[SYS_WEAPONS].room));
    advance(&g,1);
    assert(g.weapons[0].pending_shots==0);

    /* A normal encounter must be winnable using only exposed player actions. */
    game_init(&g);
    for(combat_ticks=0;combat_ticks<180*GAME_HZ && g.outcome==GAME_PLAYING;combat_ticks++) {
        for(i=0;i<GAME_WEAPON_COUNT;i++)
            if(g.weapons[i].charge_ticks==g.weapons[i].cooldown_ticks)
                game_fire(&g,i,g.enemy.systems[SYS_WEAPONS].room);
        game_step(&g);
    }
    assert(g.outcome==GAME_VICTORY);
    printf("PASS: authentic data, pause, crew routing, repair, power bounds, ammunition and combat victory (%d ticks, %d hull).\n",combat_ticks,g.player.hull);
    return 0;
}
