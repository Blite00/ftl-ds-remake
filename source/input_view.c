#include "input_view.h"
#include <string.h>
void input_view_context(const Campaign*c,const Ui*u,const MainMenu*m,InputView*out){
 int weapon=u->weapon>=0&&u->weapon<c->game.weapon_count?u->weapon:0;
 const WeaponState*w=&c->game.weapons[weapon];
 /* Plain selection, feedback, power and inventory counts do not change hit
    coordinates. Zoom selection DOES move beacons; weapon family/side changes
    may replace the targeting space. Confirmation modes always form a barrier. */
 int fields[]={m->active,m->page,m->confirm,c->mode,c->current,c->sector,
 c->event_id,c->event_resolved,c->imported_node,c->imported_battle,c->reward.kind,
 c->game.outcome,u->page,u->logs,u->aiming,u->doors,u->crew_page,u->shop_page,
 u->event_text_page,u->map_overview,c->mode==CAM_MAP&&!u->map_overview?c->selected:0,
 u->retreat,u->retreat_confirm,u->home_defense,u->reward_confirm,u->actions,
 u->drone_panel,u->drone_cargo,u->drone_sale,u->sale_confirm,u->boarding,u->board_side,
 u->board_page,u->abandon_confirm,u->dismiss_confirm,u->crew_details,u->renaming,
 u->aiming?(w->beam_length>0):0,u->aiming?w->bomb:0,u->aiming&&w->bomb?w->bomb_side:0,u->actions?u->action_selected/6:0,u->page==3?u->equipment_installed:0};
 memset(out,0,sizeof *out);memcpy(out->fields,fields,sizeof fields);
}
