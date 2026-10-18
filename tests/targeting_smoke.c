/* Targeting UI regression: real touch coordinates and independent locks. */
#include "game.h"
#include "render.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    Game g;Ui ui={0};int missiles,paused;
    game_init(&g);g.paused=1;
    ui_touch(&g,&ui,220,183);assert(ui.aiming);
    ui_touch(&g,&ui,80,120);assert(ui.weapon==1);
    /* Original scout engine room, now within the 64px targeting viewport. */
    ui_touch(&g,&ui,90,75);
    assert(g.target_room==1 && g.weapons[1].aim_room==1);
    ui_touch(&g,&ui,10,40);assert(g.target_room==1);
    g.weapons[1].charge_ticks=g.weapons[1].cooldown_ticks;
    ui_touch(&g,&ui,100,183);
    assert(g.weapons[1].target_room==1 && g.weapons[1].pending_shots==3);
    assert(!g.weapons[1].charge_ticks && g.paused);
    ui_touch(&g,&ui,20,120);assert(ui.weapon==0);
    ui_touch(&g,&ui,180,55);assert(g.weapons[0].aim_room==4 && g.target_room==4);
    missiles=g.missiles;g.weapons[0].charge_ticks=g.weapons[0].cooldown_ticks;
    ui_touch(&g,&ui,100,183);
    assert(g.weapons[0].target_room==4 && g.weapons[0].pending_shots==1);
    assert(g.missiles==missiles-1 && g.weapons[1].target_room==1);
    ui_touch(&g,&ui,80,120);assert(g.target_room==1 && ui.weapon==1);
    ui_touch(&g,&ui,180,55);
    assert(g.weapons[1].aim_room==4 && g.weapons[1].target_room==1);
    paused=g.paused;ui_touch(&g,&ui,220,183);
    assert(!ui.aiming && g.paused==paused);
    puts("PASS: weapon tabs, room selection, independent locks, queued target retention, ammo and pause.");
    return 0;
}
