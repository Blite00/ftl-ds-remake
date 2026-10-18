#include "audio.h"
/* Portable combat navigation. D-pad chooses space; actions require A. */
static int combat_room(const Ship*s,int current,int key){
 const GameRoom*from=room_by_id(s,current);if(!s->room_count)return current;if(!from)return s->rooms[0].id;
 int best=current,score=0x7fffffff,fx=2*from->x+from->w,fy=2*from->y+from->h;
 for(int i=0;i<s->room_count;i++){const GameRoom*r=&s->rooms[i];int dx=2*r->x+r->w-fx,dy=2*r->y+r->h-fy,forward=key==UI_COMBAT_LEFT?-dx:key==UI_COMBAT_RIGHT?dx:key==UI_COMBAT_UP?-dy:dy;
  if(forward<=0)continue;
  int cross=key==UI_COMBAT_LEFT||key==UI_COMBAT_RIGHT?dy:dx;if(cross<0)cross=-cross;int rank=forward*forward+cross*cross*4;if(rank<score){score=rank;best=r->id;}}
 return best;
}
/* V55: with cloaking installed, rows 12-13 are CLOAK POWER (left/right) and ACTIVATE CLOAK. */
static int cloak_rows(const Campaign*c){return c->game.player_cloak.capacity>0?2:0;}
/* V58: the Federation Artillery Beam adds an ARTILLERY POWER row after the cloak rows. */
static int artillery_row(const Campaign*c){return c->game.player_artillery.capacity>0?12+cloak_rows(c):-1;}
static int extra_rows(const Campaign*c){return cloak_rows(c)+(c->game.player_artillery.capacity>0);}
static int combat_rows(const Campaign*c,const Ui*u){int w=selected_weapon(&c->game,u);return u->actions==2?c->game.door_count+1:u->actions==3?6:12+extra_rows(c)+(c->game.weapon_count&&(c->game.weapons[w].bomb||c->game.weapons[w].beam_length));}
static int power_row(const Campaign*c,const Ui*u,int row){return u->actions==1&&(row<5||(cloak_rows(c)&&row==12)||row==artillery_row(c));}
static void combat_action_label(const Campaign*c,const Ui*u,int row,char*s,int n){
 const Game*g=&c->game;int w=selected_weapon(g,u);
 if(u->actions==2){if(!row)snprintf(s,n,"SEAL ALL DOORS");else{const GameDoor*d=&g->doors[row-1];if(d->room_a<0||d->room_b<0)snprintf(s,n,"%d AIRLOCK %s",row,d->open?"OPEN":"CLOSED");else snprintf(s,n,"%d R%d-R%d %s",row,d->room_a+1,d->room_b+1,d->open?"OPEN":"CLOSED");}return;}
 if(u->actions==3){static const char*const labels[]={"SELECTED CREW +/-","GATHER AT TELEPORTER","SEND PARTY","RECALL PARTY","TELEPORT POWER -","TELEPORT POWER +"};snprintf(s,n,"%s",labels[row]);return;}
 if(row<5){snprintf(s,n,"%s %d/%d",game_system_name(row),game_system_power(&g->player,row),g->player.systems[row].health);return;}
 static const char*const labels[]={"WEAPON POWER","AUTOFIRE","DOORS","BOARDING","DRONES","RETREAT","SOUND"};
 if(cloak_rows(c)&&row==12){snprintf(s,n,"CLOAK POWER %d/%d",g->player_cloak.power,g->player_cloak.health);return;}
 if(cloak_rows(c)&&row==13){const EnemyCloak*k=&g->player_cloak;if(k->active_ticks)snprintf(s,n,"CLOAKED %dS",(k->active_ticks+GAME_HZ-1)/GAME_HZ);else if(k->ion_bars)snprintf(s,n,"CLOAK RECHARGE %dS",(k->ion_ticks+GAME_HZ-1)/GAME_HZ);else snprintf(s,n,"ACTIVATE CLOAK");return;}
 if(row==artillery_row(c)){const Artillery*a=&g->player_artillery;int need=game_player_artillery_need(g);if(need)snprintf(s,n,"ARTILLERY %d/%d %dS",a->power,a->health,(need-a->charge+GAME_HZ-1)/GAME_HZ);else snprintf(s,n,"ARTILLERY POWER %d/%d",a->power,a->health);return;}
 if(row==12+extra_rows(c)){snprintf(s,n,"%s",g->weapon_count&&g->weapons[w].bomb?"BOMB: SWITCH HOME / ENEMY":"RESET BEAM LINE");return;}
 if(row==11){snprintf(s,n,"SOUND  %s",audio_volume_label());return;}
 if(row==5||row==6)snprintf(s,n,"%s  %s",labels[row-5],g->weapon_count&&(row==5?g->weapons[w].enabled:g->autofire[w])?"ON":"OFF");else snprintf(s,n,"%s",labels[row-5]);
}
static void combat_actions_view(const Campaign*c,const Ui*u,uint16_t*p){
 char s[64];rect(p,0,0,256,192,BG);panel(p,3,3,250,186,CYAN);text_scale(p,12,10,u->actions==2?"DOORS":u->actions==3?"BOARDING":"ACTIONS",GOLD,2);
 int count=combat_rows(c,u),page=u->action_selected/6;
 snprintf(s,sizeof s,"%d / %d",page+1,(count+5)/6);text(p,202,14,s,WHITE);
 for(int i=page*6;i<count&&i<page*6+6;i++){
  int y=33+(i%6)*22,power=power_row(c,u,i);
  combat_action_label(c,u,i,s,sizeof s);
  if(i==u->action_selected){panel(p,8,y,240,20,GOLD);s[power?14:19]=0;text_scale(p,12,y+3,s,WHITE,2);}else button(p,8,y,240,20,s,MUTED);
  if(power){button(p,188,y,28,20,"-",CYAN);button(p,220,y,28,20,"+",CYAN);}
 }
 text(p,12,171,power_row(c,u,u->action_selected)?"LEFT/RIGHT POWER / B BACK":"UP/DOWN  A SELECT  B BACK",WHITE);
 text(p,12,182,"TIME HELD",GOLD);
}
static void combat_action(Campaign*c,Ui*u,int key){
 Game*g=&c->game;int row=u->action_selected,w=selected_weapon(g,u),ok=1;
 if(u->actions==2){if(!row)game_close_all_doors(g);else ok=game_toggle_door(g,row-1);ui_feedback(u,g->message,!ok);return;}
 if(u->actions==3){if(!row){unsigned bit=1u<<u->selected_crew;int count=0;for(int i=0;i<g->crew_count;i++)if(u->board_party&(1u<<i))count++;if(u->board_party&bit)u->board_party&=~bit;else if(count<2)u->board_party|=bit;else ui_feedback(u,"Select up to two crew.",1);}else{static const int actions[]={2,0,1,3,4};ui_board_action(g,u,actions[row-1]);}return;}
 if(row<5){ok=game_adjust_power(g,row,key==UI_COMBAT_LEFT?-1:1);ui_feedback(u,g->message,!ok);return;}
 if(row==5||row==6){if(!g->weapon_count){ui_feedback(u,"No weapons installed.",1);return;}if(row==5)game_toggle_weapon(g,w);else game_toggle_autofire(g,w);return;}
 if(row==11){audio_cycle_volume();return;}
 if(cloak_rows(c)&&row==12){ok=game_cloak_power(g,key==UI_COMBAT_LEFT?-1:1);ui_feedback(u,g->message,!ok);return;}
 if(cloak_rows(c)&&row==13){ok=game_cloak_activate(g);ui_feedback(u,g->message,!ok);return;}
 if(row==artillery_row(c)){ok=game_player_artillery_power(g,key==UI_COMBAT_LEFT?-1:1);ui_feedback(u,g->message,!ok);return;}
 if(row==12+extra_rows(c)){if(g->weapon_count&&g->weapons[w].bomb)ui_bomb_toggle_side(g,u);else if(g->weapon_count&&g->weapons[w].beam_length){g->weapons[w].beam_target_valid=0;u->beam_pending=0;}else ui_feedback(u,"Select a bomb or beam first.",1);return;}
 u->action_selected=0;
 if(row==7){u->actions=2;return;}
 u->actions=0;
 if(row==8){u->boarding=1;u->board_side=0;u->board_room=g->player.rooms[0].id;u->board_party=1u<<u->selected_crew;}
 else if(row==9)ui_drone_open(u);else if(c->mode==CAM_COMBAT)ui_retreat_open(c,u);else ui_feedback(u,"Retreat is only available in combat.",1);
}
void ui_combat_key(Campaign*c,Ui*u,int key){
 if(!c||key<UI_COMBAT_UP||key>UI_COMBAT_START)return;
 if(c->reward.kind){if(key==UI_COMBAT_UP||key==UI_COMBAT_DOWN||key==UI_COMBAT_A||key==UI_COMBAT_B)ui_reward_key(c,u,key==UI_COMBAT_UP?0:key==UI_COMBAT_DOWN?1:key==UI_COMBAT_A?2:3);return;}
 if(c->mode!=CAM_COMBAT&&!((u->home_defense||u->page==1)&&campaign_can_service_home(c)))return;
 Game*g=&c->game;
 if(u->logs){if(key==UI_COMBAT_B||key==UI_COMBAT_START)u->logs=0;return;}
 if(u->retreat){if(key==UI_COMBAT_UP||key==UI_COMBAT_DOWN||key==UI_COMBAT_A||key==UI_COMBAT_B)ui_retreat_key(c,u,key==UI_COMBAT_UP?0:key==UI_COMBAT_DOWN?1:key==UI_COMBAT_A?2:3);return;}
 if(u->crew_details){if(key==UI_COMBAT_B)u->crew_details=u->renaming=0;return;}
 if(u->drone_panel){int k=key==UI_COMBAT_UP?0:key==UI_COMBAT_DOWN?1:key==UI_COMBAT_A?2:key==UI_COMBAT_B?3:(key==UI_COMBAT_LEFT||key==UI_COMBAT_L)?5:(key==UI_COMBAT_RIGHT||key==UI_COMBAT_R)?6:key==UI_COMBAT_Y?4:key==UI_COMBAT_X?7:-1;if(k>=0)ui_drone_key(c,u,k);return;}
 if(key==UI_COMBAT_X){game_toggle_pause(g);return;}
 if(c->mode==CAM_COMBAT&&g->outcome!=GAME_PLAYING){if(key==UI_COMBAT_B)u->actions=0;return;}
 if(u->actions){if(key==UI_COMBAT_B||key==UI_COMBAT_START){if(u->actions==2){u->actions=1;u->action_selected=7;}else u->actions=0;return;}int count=combat_rows(c,u);if(key==UI_COMBAT_UP||key==UI_COMBAT_DOWN)u->action_selected=(u->action_selected+count+(key==UI_COMBAT_UP?-1:1))%count;else if(key==UI_COMBAT_A||(power_row(c,u,u->action_selected)&&(key==UI_COMBAT_LEFT||key==UI_COMBAT_RIGHT)))combat_action(c,u,key);return;}
 if(key==UI_COMBAT_START){u->actions=u->boarding?3:1;u->action_selected=0;return;}
 if(key==UI_COMBAT_B){if(u->boarding)u->boarding=0;else if(u->aiming){int w=selected_weapon(g,u);if(g->weapon_count&&g->weapons[w].aim_room!=GAME_AIM_HOLD){game_clear_weapon_target(g,w);u->beam_pending=0;ui_feedback(u,"Target cleared. This weapon holds fire.",0);}else{u->aiming=u->beam_pending=0;}}else if(u->doors)u->doors=0;else if(!u->crew_deselected){ui_deselect_crew(u);}else if(u->home_defense)u->home_defense=0;else if(c->mode!=CAM_COMBAT)u->page=0;else ui_retreat_open(c,u);return;}
 if(c->mode==CAM_COMBAT&&g->outcome!=GAME_PLAYING)return;
 if(u->selected_crew<0||u->selected_crew>=g->crew_count)u->selected_crew=0;
 if(key==UI_COMBAT_Y){if(c->mode!=CAM_COMBAT&&!u->boarding){if(!u->home_defense&&u->page==1)u->page=2;else ui_feedback(u,"Aiming is available in combat.",1);return;}if(u->boarding){u->board_side=!u->board_side;const Ship*s=u->board_side?&g->enemy:&g->player;if(s->room_count)u->board_room=s->rooms[0].id;}else{u->aiming=!u->aiming;u->beam_pending=0;if(u->aiming&&g->weapon_count)ui_select_weapon(g,u,selected_weapon(g,u));}return;}
 if(key==UI_COMBAT_L||key==UI_COMBAT_R){int delta=key==UI_COMBAT_L?-1:1;if(u->aiming&&!u->boarding){if(g->weapon_count)ui_select_weapon(g,u,(selected_weapon(g,u)+g->weapon_count+delta)%g->weapon_count);}else if(g->crew_count){u->crew_deselected=0;for(int tries=0;tries<g->crew_count;tries++){u->selected_crew=(u->selected_crew+g->crew_count+delta)%g->crew_count;if(g->crew[u->selected_crew].health>0)break;}u->crew_page=u->selected_crew/3;u->board_page=u->selected_crew/6;u->focus_room=g->crew[u->selected_crew].room;}u->controller_focus=1;return;}
 if(key<=UI_COMBAT_RIGHT){
  u->doors=0;
  if(u->boarding){const Ship*s=u->board_side?&g->enemy:&g->player;u->board_room=combat_room(s,u->board_room,key);}
  else if(u->aiming&&g->weapon_count){int w=selected_weapon(g,u);WeaponState*weapon=&g->weapons[w];const Ship*s=weapon->bomb&&weapon->bomb_side?&g->player:&g->enemy;int room=combat_room(s,g->target_room,key);if(weapon->bomb)game_set_bomb_target(g,w,weapon->bomb_side,room);else if(!weapon->beam_length)game_set_weapon_target(g,w,room);g->target_room=room;}
  else{if(u->crew_deselected){ui_feedback(u,"Select crew with L/R first.",0);return;}if(!u->controller_focus)u->focus_room=g->crew[u->selected_crew].room;u->focus_room=combat_room(&g->player,u->focus_room,key);}u->controller_focus=1;return;
 }
 if(key==UI_COMBAT_A){if(u->boarding){for(int i=0;i<g->crew_count;i++)if((u->board_party&(1u<<i))&&g->crew[i].ship==u->board_side)game_order_crew(g,i,u->board_room);}
 else if(u->aiming){if(g->weapon_count&&g->weapons[selected_weapon(g,u)].beam_length&&(!g->weapons[selected_weapon(g,u)].beam_target_valid||u->beam_pending))ui_beam_pick_room(g,u);else ui_fire(g,u);}
 else{if(u->crew_deselected){ui_feedback(u,"Select crew with L/R first.",0);return;}if(!u->controller_focus)u->focus_room=g->crew[u->selected_crew].room;int ok=game_order_crew(g,u->selected_crew,u->focus_room);ui_feedback(u,g->message,!ok);}}
}
void ui_combat_touch(Campaign*c,Ui*u,int x,int y){
 if(!u->actions)return;
 if(x<0||x>=256||y<0||y>=192)return;
 if(y>=166){ui_combat_key(c,u,UI_COMBAT_B);return;}
 if(y>=33&&y<165){int row=(u->action_selected/6)*6+(y-33)/22;if(row<combat_rows(c,u)){u->action_selected=row;if(power_row(c,u,row)){if(x>=188)ui_combat_key(c,u,x<220?UI_COMBAT_LEFT:UI_COMBAT_RIGHT);}else ui_combat_key(c,u,UI_COMBAT_A);}}
}
