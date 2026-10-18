/* Pending rewards freeze the underlying event until an explicit decision. */
static int reward_rows(const Campaign*c){return c->reward.kind==REWARD_CREW?c->game.crew_count:CAM_CARGO_COUNT;}
static void reward_open(Ui*u){if(!u->reward_active){u->reward_active=1;u->reward_selected=-1;u->reward_confirm=0;u->feedback_ticks=0;}}
static const char*reward_slot_name(const Campaign*c,int slot){
 if(c->reward.kind==REWARD_CREW)return slot>=0&&slot<c->game.crew_count?game_crew_name(&c->game,slot):"FREE CREW POSITION";
 return slot>=0&&slot<CAM_CARGO_COUNT&&c->cargo[slot]>=0?game_weapon_name(c->cargo[slot]):"EMPTY CARGO SLOT";
}
void ui_reward_key(Campaign*c,Ui*u,int key){
 if(c->reward.kind==REWARD_NONE||c->mode==CAM_DEFEAT||c->mode==CAM_COMPLETE)return;
 reward_open(u);int rows=reward_rows(c),slot=u->reward_selected;
 if(key==3){u->reward_confirm=u->reward_confirm?0:2;u->feedback_ticks=0;return;}
 if((key==0||key==1)&&!u->reward_confirm){u->reward_selected=(slot+1+rows+1+(key?1:-1))%(rows+1)-1;return;}
 if(key!=2)return;
 if(u->reward_confirm==2){rewards_decline(&c->reward);ui_feedback(u,"Reward declined.",0);}
 else {
  int occupied=slot>=0&&(c->reward.kind==REWARD_CREW?(slot<c->game.crew_count&&c->game.crew[slot].health>0):(slot<CAM_CARGO_COUNT&&c->cargo[slot]>=0));
  int replace=occupied?1:0;
  if(!rewards_can_accept(&c->reward,&c->game,c->cargo,CAM_CARGO_COUNT,slot,replace)){ui_feedback(u,slot<0?"No free space. Select an item to replace.":"That slot cannot be replaced now.",1);return;}
  if(occupied&&!u->reward_confirm){u->reward_confirm=1;u->feedback_ticks=0;return;}
  if(!rewards_accept(&c->reward,&c->game,c->cargo,CAM_CARGO_COUNT,slot,replace)){ui_feedback(u,"Reward unavailable. Nothing changed.",1);return;}
  ui_feedback(u,occupied?"Replacement confirmed.":"Reward received.",0);
 }
 u->reward_active=u->reward_confirm=0;u->reward_selected=-1;
 u->selected_crew=u->crew_page=u->board_page=0;u->crew_group=u->board_party=0;
 u->boarding=u->crew_details=u->renaming=u->dismiss_confirm=u->drone_panel=u->sale_confirm=u->aiming=u->group_mode=u->retreat=u->home_defense=0;
}
static void reward_view(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];int crew=c->reward.kind==REWARD_CREW,selected=u->reward_active?u->reward_selected:-1;
 space_background(top);space_background(bottom);panel(top,3,3,250,186,CYAN);
 text(top,12,12,crew?"CREW RECRUITMENT":"RECOVERED WEAPON",GOLD);
 if(crew){snprintf(s,sizeof s,"%.12s / %s",c->reward.recruit.name,species_name(c->reward.recruit.species));text(top,12,35,s,WHITE);snprintf(s,sizeof s,"HEALTH %d / NEW CREW MEMBER",c->reward.recruit.health);text(top,12,54,s,CYAN);}
 else {text(top,12,35,game_weapon_name(c->reward.weapon),WHITE);text(top,12,54,"ADDED TO CARGO; FIT IT LATER",CYAN);}
 text(top,12,78,"CHOOSE FREE SPACE OR A REPLACEMENT",WHITE);
 text(top,12,98,crew?"REPLACED CREW LEAVE PERMANENTLY":"REPLACED CARGO IS LEFT BEHIND",GOLD);
 text(top,12,119,"REPLACEMENTS REQUIRE CONFIRMATION",MUTED);
 text(top,12,142,"UP/DOWN SELECT / A ACCEPT",WHITE);text(top,12,160,"B DECLINE / TIME IS HELD",MUTED);
 if(u->reward_confirm){
  panel(bottom,3,8,250,153,u->reward_confirm==2?GOLD:RED);
  text(bottom,12,23,u->reward_confirm==2?"DECLINE THIS REWARD?":"CONFIRM REPLACEMENT",GOLD);
  if(u->reward_confirm==1){text(bottom,12,47,crew?"CREW LEAVING:":"CARGO LEFT BEHIND:",MUTED);snprintf(s,sizeof s,"%.36s",reward_slot_name(c,selected));text(bottom,12,65,s,WHITE);text(bottom,12,90,"RECEIVE:",MUTED);}
  else text(bottom,12,47,"LEAVE BEHIND:",MUTED);
  snprintf(s,sizeof s,"%.36s",crew?c->reward.recruit.name:game_weapon_name(c->reward.weapon));text(bottom,12,u->reward_confirm==1?108:67,s,CYAN);
  text(bottom,12,136,"THIS DECISION CANNOT BE UNDONE",GOLD);
  button(bottom,3,173,120,19,"CANCEL / B",CYAN);button(bottom,130,173,123,19,"CONFIRM / A",RED);return;
 }
 text(bottom,8,8,crew?"CHOOSE CREW POSITION":"CHOOSE CARGO SLOT",WHITE);
 button(bottom,4,24,248,20,"USE FREE SPACE",selected<0?GOLD:CYAN);
 int rows=reward_rows(c),height=crew?14:24;
 for(int i=0;i<rows;i++){
  int y=48+i*height;snprintf(s,sizeof s,"%d %.30s",i+1,reward_slot_name(c,i));
  if(crew)snprintf(s,sizeof s,"%d %.12s %s %d",i+1,game_crew_name(&c->game,i),c->game.crew[i].ship?"AWAY":"HOME",c->game.crew[i].health);
  button(bottom,4,y,248,height-1,s,i==selected?GOLD:MUTED);
 }
 text(bottom,8,163,"SELECT, THEN ACCEPT",MUTED);
 button(bottom,3,175,120,17,"DECLINE / B",GOLD);button(bottom,130,175,123,17,"ACCEPT / A",CYAN);
}
static void reward_touch(Campaign*c,Ui*u,int x,int y){
 reward_open(u);
 if(u->reward_confirm){if(y>=173)ui_reward_key(c,u,x<128?3:2);return;}
 if(y>=175){ui_reward_key(c,u,x<128?3:2);return;}
 if(y>=24&&y<44){u->reward_selected=-1;return;}
 int h=c->reward.kind==REWARD_CREW?14:24,slot=(y-48)/h;
 if(y>=48&&slot<reward_rows(c)){u->reward_selected=slot;return;}
}
