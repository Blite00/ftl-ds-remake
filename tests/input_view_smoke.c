#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "input_view.h"
int main(void){
 Campaign c={0};Ui u={0};MainMenu m={0};InputView a,b;
 c.game.weapon_count=2;u.map_overview=1;
 input_view_context(&c,&u,&m,&a);
 u.tap_ticks=8;u.feedback_ticks=75;u.event_selected=1;u.focus_room=5;u.selected_crew=2;
 c.scrap=99;c.selected=3;c.game.player.systems[0].power=1;m.selected=2;m.confirm_yes=1;
 input_view_context(&c,&u,&m,&b);assert(!memcmp(&a,&b,sizeof a));
 u.sale_confirm=1;input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));u.sale_confirm=0;
 m.confirm=1;input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));m.confirm=0;
 c.imported_node=3;input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));c.imported_node=0;
 u.aiming=1;input_view_context(&c,&u,&m,&a);u.weapon=1;
 input_view_context(&c,&u,&m,&b);assert(!memcmp(&a,&b,sizeof a));
 c.game.weapons[1].beam_length=50;input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));
 u.aiming=0;u.map_overview=0;input_view_context(&c,&u,&m,&a);c.selected++;
 input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));
 u.actions=1;u.action_selected=1;input_view_context(&c,&u,&m,&a);u.action_selected=2;
 input_view_context(&c,&u,&m,&b);assert(!memcmp(&a,&b,sizeof a));u.action_selected=6;
 input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));
 u.page=3;input_view_context(&c,&u,&m,&a);u.equipment_installed=1;
 input_view_context(&c,&u,&m,&b);assert(memcmp(&a,&b,sizeof a));
 puts("Input view: selections/power/feedback stay responsive; modal, event, map camera and target-space changes invalidate stale contacts");return 0;
}
