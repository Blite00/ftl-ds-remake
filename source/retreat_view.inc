/* Campaign-owned jump controls; opening and cancelling never mutate simulation. */
void ui_retreat_open(Campaign*c,Ui*u){
 if(c->mode!=CAM_COMBAT||c->game.outcome!=GAME_PLAYING)return;
 u->retreat=1;u->retreat_confirm=0;u->retreat_selected=c->current;
 for(int n=0;n<c->node_count;n++)if(n!=c->current&&campaign_linked(c,c->current,n)){u->retreat_selected=n;break;}
 u->aiming=u->doors=u->boarding=u->crew_details=u->drone_panel=0;
 u->beam_pending=u->group_mode=0;u->feedback_ticks=0;
}
void ui_home_open(Campaign*c,Ui*u){
 if(!campaign_can_service_home(c))return;
 u->home_defense=1;u->aiming=u->doors=u->boarding=u->crew_details=u->drone_panel=0;
 u->beam_pending=u->group_mode=0;ui_feedback(u,"Order defenders and repairs. B returns.",0);
}
void ui_retreat_key(Campaign*c,Ui*u,int key){
 if(!u->retreat||c->mode!=CAM_COMBAT||c->game.outcome!=GAME_PLAYING)return;
 if(key==3){if(u->retreat_confirm)u->retreat_confirm=0;else u->retreat=0;u->feedback_ticks=0;return;}
 if(key==0||key==1){
  if(u->retreat_confirm)return;
  for(int step=1;step<=c->node_count;step++){int n=(u->retreat_selected+c->node_count+(key?step:-step))%c->node_count;
   if(n!=c->current&&campaign_linked(c,c->current,n)){u->retreat_selected=n;break;}}
  return;
 }
 if(key!=2)return;
 if(!campaign_can_retreat(c,u->retreat_selected)){
  ui_feedback(u,c->fuel<=0?"No fuel for retreat.":!game_player_ftl_operational(&c->game)?"Need powered engines and a healthy pilot.":!game_player_ftl_ready(&c->game)?"FTL charging. Close this map to continue.":"Cannot depart to this beacon.",1);return;
 }
 if(game_away_crew(&c->game)&&!u->retreat_confirm){u->retreat_confirm=1;u->feedback_ticks=0;return;}
 if(!campaign_retreat(c,u->retreat_selected,u->retreat_confirm?1:0)){ui_feedback(u,"Departure refused. Ship state preserved.",1);return;}
 u->retreat=u->retreat_confirm=u->home_defense=0;u->page=0;u->event_selected=u->event_text_page=0;
 u->board_party=u->crew_group=0;u->weapon=0;
 ui_feedback(u,"Jump complete. Fuel spent; fleet advanced.",0);
}
static void retreat_controls(const Campaign*c,const Ui*u,uint16_t*p){
 char s[16];rect(p,0,175,256,17,BG);
 button(p,1,176,43,16,c->game.paused?"GO X":"PAUSE",GOLD);
 button(p,46,176,60,16,"FIRE A",GREEN);button(p,109,176,45,16,"DRONES",CYAN);
 if(game_player_ftl_ready(&c->game))snprintf(s,sizeof s,"JUMP B");else snprintf(s,sizeof s,"FTL%d%%",c->game.player_ftl_charge*100/GAME_PLAYER_FTL_CHARGE);
 button(p,157,176,47,16,s,game_player_ftl_ready(&c->game)?GREEN:GOLD);
 button(p,207,176,47,16,u->aiming?"BACK Y":"AIM Y",CYAN);
}
static void retreat_view(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];space_background(top);space_background(bottom);
 panel(top,3,3,250,186,CYAN);text(top,12,12,"RETREAT / SIMULATION HELD",GOLD);
 snprintf(s,sizeof s,"FTL %d%% / FUEL %d",c->game.player_ftl_charge*100/GAME_PLAYER_FTL_CHARGE,c->fuel);text(top,12,32,s,WHITE);
 bar(top,12,47,230,c->game.player_ftl_charge,GAME_PLAYER_FTL_CHARGE,CYAN);
 text(top,12,61,game_player_ftl_ready(&c->game)?"READY TO JUMP":game_player_ftl_operational(&c->game)?"CLOSE MAP TO KEEP CHARGING":"ENGINES / PILOT NOT OPERATIONAL",game_player_ftl_ready(&c->game)?GREEN:GOLD);
 text(top,12,81,"COST: 1 FUEL / FLEET ADVANCES",WHITE);
 text(top,12,98,"NO SALVAGE FROM ABANDONED BATTLE",MUTED);
 text(top,12,115,"HOSTILE BOARDERS TRAVEL WITH YOU",RED);
 snprintf(s,sizeof s,"AWAY CREW: %d",game_away_crew(&c->game));text(top,12,133,s,GOLD);
 text(top,12,154,"L/R: DESTINATION / A: JUMP",WHITE);text(top,12,171,"B: CANCEL / OR TOUCH CONTROLS",MUTED);
 if(u->retreat_confirm){
  panel(bottom,3,3,250,167,RED);text(bottom,12,12,"ABANDON AWAY CREW?",RED);
  int row=0;for(int i=0;i<c->game.crew_count;i++)if(c->game.crew[i].health>0&&c->game.crew[i].ship==1){snprintf(s,sizeof s,"%.12s WILL BE LOST",game_crew_name(&c->game,i));text(bottom,12,32+row++*14,s,WHITE);}
  button(bottom,3,174,120,18,"CANCEL / B",CYAN);button(bottom,130,174,123,18,"ABANDON / A",RED);return;
 }
 text(bottom,8,8,"SELECT A LINKED BEACON",WHITE);text(bottom,8,24,"GOLD: CAPTURED ON ARRIVAL",GOLD);
 for(int a=0;a<c->node_count;a++)for(int b=a+1;b<c->node_count;b++)if(campaign_linked(c,a,b))line(bottom,campaign_node_x(c,a),campaign_node_y(c,a),campaign_node_x(c,b),campaign_node_y(c,b),a==c->current||b==c->current?GREEN:EDGE);
 for(int i=0;i<c->node_count;i++){int x=campaign_node_x(c,i),y=campaign_node_y(c,i),color=i==c->current?CYAN:campaign_node_captured(c,i,1)?GOLD:campaign_linked(c,c->current,i)?GREEN:MUTED;
  rect(bottom,x-5,y-5,11,11,PANEL);box(bottom,x-5,y-5,11,11,color);
  if(i==u->retreat_selected)box(bottom,x-8,y-8,17,17,WHITE);
 }
 snprintf(s,sizeof s,"%02d %.23s",u->retreat_selected+1,campaign_node_name(c,u->retreat_selected));text(bottom,8,147,s,WHITE);
 text(bottom,8,159,campaign_node_captured(c,u->retreat_selected,1)?"REBEL INTERCEPTION ON ARRIVAL":"LINKED DESTINATION",GOLD);
 button(bottom,3,174,120,18,"CANCEL / B",CYAN);button(bottom,130,174,123,18,"JUMP / A",campaign_can_retreat(c,u->retreat_selected)?GREEN:MUTED);
}
static void retreat_touch(Campaign*c,Ui*u,int x,int y){
 if(y>=174){ui_retreat_key(c,u,x<128?3:2);return;}
 if(u->retreat_confirm)return;
 for(int i=0;i<c->node_count;i++)if(i!=c->current&&campaign_linked(c,c->current,i)&&x>=campaign_node_x(c,i)-12&&x<=campaign_node_x(c,i)+12&&y>=campaign_node_y(c,i)-12&&y<=campaign_node_y(c,i)+12){u->retreat_selected=i;return;}
}

/* One shared clock policy for DS input and host UI integration tests. */
void ui_campaign_step(Campaign*c,const Ui*u){
 if(c->reward.kind!=REWARD_NONE||u->logs||u->crew_details||u->retreat||(u->actions&&(c->mode!=CAM_COMBAT||c->game.outcome==GAME_PLAYING)))return;
 if(u->home_defense){campaign_service_home(c);return;}
 int management=c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved);
 if(c->mode==CAM_COMBAT||(!u->drone_panel&&(u->page==1||u->boarding)&&management))campaign_step(c);
}
