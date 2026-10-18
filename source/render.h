#ifndef FTL_DS_RENDER_H
#define FTL_DS_RENDER_H
#include <stdint.h>
#include "game.h"
#include "campaign.h"
#include "menu.h"
typedef struct { int menu_requested;
    int crew_deselected;
    int selected_crew, aiming, briefing, logs, doors, focus_room;
    int weapon,crew_page,page,shop_page,shop_selected,cargo_selected;
    int event_selected,event_text_page;
    int map_overview;
    int retreat,retreat_selected,retreat_confirm,home_defense;
    int reward_active,reward_selected,reward_confirm;
    int actions,action_selected,controller_focus;
    int drone_panel,drone_cargo,drone_selected,drone_focus,drone_sale;
    int equipment_installed,equipment_slot,sale_confirm;
    int boarding,board_side,board_room,board_page,abandon_confirm;unsigned board_party;
    int group_mode,dismiss_confirm;unsigned crew_group;
    int beam_pending,beam_weapon,beam_x,beam_y;
    int crew_details,renaming;char rename_text[13];
    int feedback_ticks,feedback_error,tap_ticks,tap_x,tap_y;
    char feedback[96]; } Ui;
enum UiCombatKey { UI_COMBAT_UP,UI_COMBAT_DOWN,UI_COMBAT_LEFT,UI_COMBAT_RIGHT,UI_COMBAT_A,UI_COMBAT_B,UI_COMBAT_X,UI_COMBAT_Y,UI_COMBAT_L,UI_COMBAT_R,UI_COMBAT_START };
void ui_deselect_crew(Ui*);
void ui_combat_key(Campaign*,Ui*,int);
void ui_combat_touch(Campaign*,Ui*,int,int);
/* UI ticks run at 30 Hz even while simulation is paused. */
/* Drone keys: 0 previous, 1 next, 2 activate, 3 back, 4 tab, 5 power-, 6 power+, 7 pause. */
/* Event keys: 0 previous, 1 next, 2 confirm current choice. */
void ui_retreat_open(Campaign*,Ui*);
/* Retreat: 0 previous, 1 next, 2 confirm, 3 cancel. */
void ui_retreat_key(Campaign*,Ui*,int);
/* Reward keys: 0 previous, 1 next, 2 accept/confirm, 3 decline/cancel. */
void ui_reward_key(Campaign*,Ui*,int);
void ui_home_open(Campaign*,Ui*);
void ui_campaign_step(Campaign*,const Ui*);
void ui_event_key(Campaign*,Ui*,int);
/* Narrative keys: 0 previous page, 1 next page. No campaign mutation. */
int ui_event_text_pages(const Campaign*);
void ui_event_text_key(const Campaign*,Ui*,int);
/* Map keys: 0/1 selection, 2 wait, 3 zoom, 4 center on ship. */
void ui_map_key(Campaign*,Ui*,int);
int ui_map_node_position(const Campaign*,const Ui*,int,int*,int*);
void ui_drone_key(Campaign*,Ui*,int);
void ui_drone_open(Ui*);
void ui_tick(Ui *ui);
void ui_feedback(Ui *ui,const char *message,int error);
void ui_fire(Game *g,Ui *ui);
void ui_select_weapon(Game*,Ui*,int);
void ui_bomb_toggle_side(Game*,Ui*);
void ui_beam_pick_room(Game *g,Ui *ui);
void ui_board_action(Game *g,Ui *ui,int action);
void render_main_menu(const MainMenu*,uint16_t*,uint16_t*);
void render_game(const Game *g,const Ui *ui,uint16_t *top,uint16_t *bottom);
/* V46 crew walk point (sprite top-left) at tile size t with origin 0,0: slot -> door -> next room. */
void render_crew_path_point(const Game *g,int faction,int index,int t,int *x,int *y);
void render_campaign(const Campaign *campaign,const Ui *ui,uint16_t *top,uint16_t *bottom);
void campaign_touch(Campaign *campaign,Ui *ui,int x,int y);
void ui_touch(Game *g,Ui *ui,int x,int y);
#endif
