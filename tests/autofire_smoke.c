#include "game.h"
#include "render.h"
#include <assert.h>
#include <stdio.h>
static void setup(Game *g) {
    game_init(g);g->enemy.hull=g->enemy.max_hull=1000;
    g->enemy.systems[SYS_WEAPONS].capacity=0;
    g->enemy.systems[SYS_WEAPONS].health=0;
    g->enemy.systems[SYS_WEAPONS].power=0;
}
/* Bonuses may shorten charge; wait for an observable shot instead of assuming
 * that the last tick of the nominal cooldown must be the firing tick. */
static int await_laser(Game *g) {
    int i;for(i=0;i<=g->weapons[1].cooldown_ticks;i++) {
        game_step(g);if(g->weapons[1].pending_shots>0)return i+1;
    }
    assert(!"Expected laser volley within nominal cooldown");return 0;
}
int main(void) {
    Game g;Ui ui={0};int oldtarget,ticks;setup(&g);
    assert(!g.autofire[0] && !g.autofire[1]);
    assert(!game_toggle_autofire(&g,-1) && !game_toggle_autofire(&g,2));
    ui.aiming=1;ui.weapon=1;g.paused=1;ui_touch(&g,&ui,190,160);
    assert(g.autofire[1] && !g.autofire[0]);
    g.weapons[1].charge_ticks=g.weapons[1].cooldown_ticks;
    game_step(&g);assert(!g.weapons[1].pending_shots);
    g.paused=0;game_step(&g);
    assert(g.weapons[1].pending_shots==2 && !g.weapons[1].charge_ticks);
    oldtarget=g.weapons[1].target_room;assert(game_set_weapon_target(&g,1,1));g.target_room=0;
    for(int i=0;i<12;i++)game_step(&g);
    assert(g.weapons[1].target_room==oldtarget);
    await_laser(&g);assert(g.weapons[1].target_room==1 && g.missiles==8);
    ui_touch(&g,&ui,190,160);assert(!g.autofire[1]);
    for(int i=0;i<2*g.weapons[1].cooldown_ticks;i++)game_step(&g);
    assert(!g.weapons[1].pending_shots && g.weapons[1].charge_ticks==g.weapons[1].cooldown_ticks);
    setup(&g);game_toggle_autofire(&g,0);g.missiles=1;
    for(int i=0;i<3*g.weapons[0].cooldown_ticks;i++)game_step(&g);
    assert(g.missiles==0 && !g.weapons[0].pending_shots);
    setup(&g);game_toggle_autofire(&g,1);g.player.systems[SYS_WEAPONS].power=0;
    g.weapons[1].charge_ticks=g.weapons[1].cooldown_ticks;
    game_step(&g);assert(!g.weapons[1].pending_shots && !g.weapons[1].charge_ticks);
    g.player.systems[SYS_WEAPONS].power=3;ticks=await_laser(&g);
    assert(ticks<g.weapons[1].cooldown_ticks); /* stationed crew accelerates charge */
    setup(&g);ui.weapon=1;ui.aiming=1;game_toggle_autofire(&g,1);
    ui_touch(&g,&ui,60,160);assert(!g.weapons[1].enabled && g.autofire[1]);
    for(int i=0;i<400;i++)game_step(&g);
    assert(!g.weapons[1].charge_ticks && !g.weapons[1].pending_shots);
    ui_touch(&g,&ui,60,160);assert(g.weapons[1].enabled);await_laser(&g);
    setup(&g);game_toggle_autofire(&g,0);g.outcome=GAME_VICTORY;
    g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
    game_step(&g);assert(g.missiles==8 && !game_toggle_autofire(&g,0));
    Campaign c;campaign_init_legacy(&c);c.game.autofire[0]=c.game.autofire[1]=1;
    assert(campaign_jump(&c,1));assert(!c.game.autofire[0] && !c.game.autofire[1]);
    puts("PASS: autofire touch/pause/repeat, independent retarget, queued retention, ammo, activation, power, charge bonus and encounter reset.");
    return 0;
}
