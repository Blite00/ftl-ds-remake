static void surrender_clear_modals(Ui*u);
/* Wrapping consumes the complete source string without a fixed narrative copy.
   Each frame scans bounded imported text; only one 38-character row is stored. */
static const char* event_row(const char*cursor,char row[39]){
 int n=0,last=-1;
 while(cursor[n]&&cursor[n]!='\n'&&n<38){if(cursor[n]==' ')last=n;n++;}
 if(n==38&&cursor[n]&&cursor[n]!='\n'&&last>0)n=last;
 memcpy(row,cursor,(size_t)n);row[n]=0;cursor+=n;
 while(*cursor==' '||*cursor=='\n'||*cursor=='\r')cursor++;
 return cursor;
}
int ui_event_text_pages(const Campaign*c){
 const char*cursor=campaign_text(c);char row[39];int lines=0;
 while(*cursor){cursor=event_row(cursor,row);lines++;}
 return lines?(lines+10)/11:1;
}
void ui_event_text_key(const Campaign*c,Ui*u,int key){
 if(campaign_surrender_pending(c))surrender_clear_modals(u);
 if(c->mode!=CAM_EVENT||u->page||u->logs||u->drone_panel||u->crew_details||u->boarding||(key!=0&&key!=1))return;
 int pages=ui_event_text_pages(c),page=u->event_text_page;
 if(page<0)page=0;
 if(page>=pages)page=pages-1;
 if(key&&page<pages-1)page++;
 if(!key&&page>0)page--;
 u->event_text_page=page;
 /* Avoid feedback overlays obscuring the text the player just requested. */
 u->feedback_ticks=0;
}
static void event_narrative(const Campaign*c,const Ui*u,uint16_t*p){
 int pages=ui_event_text_pages(c),page=u->event_text_page;char row[39];
 if(page<0)page=0;
 if(page>=pages)page=pages-1;
 const char*cursor=campaign_text(c);
 for(int i=0;i<page*11&&*cursor;i++)cursor=event_row(cursor,row);
 for(int i=0;i<11&&*cursor;i++){cursor=event_row(cursor,row);text(p,12,62+i*10,row,WHITE);}
 if(pages>1){char status[64];snprintf(status,sizeof status,"TEXT %d/%d / L-R TO READ",page+1,pages);text(p,12,177,status,CYAN);}
}
/* V62: choice boxes use the space the choices leave: fewer choices, taller boxes and
 * more text rows (tools/event_compile.py CHOICE_ROWS must match). */
static int choice_step(int n){static const int step[7]={60,60,60,40,31,25,20};return step[n<1?1:n>6?6:n];}
static int choice_height(int n){static const int h[7]={56,56,56,38,30,24,19};return h[n<1?1:n>6?6:n];}
static int choice_rows(int n){static const int rows[7]={6,6,6,4,3,3,2};return rows[n<1?1:n>6?6:n];}
/* V63: original-style reply list: no box per reply, a number and indented wrapped
 * text inside the shared event window; the selected reply is yellow with a marker. */
static void event_choice_button(uint16_t*p,int y,int h,int rows,int number,int selected,const char*label,const char*hint,int color){
 char row[39],num[12];const char*cursor;int drawn=0;(void)h;
 snprintf(num,sizeof num,"%d.",number);
 if(selected){rect(p,6,y+1,2,7,FTL_YELLOW);}
 text(p,10,y+1,num,color);
 for(int part=0;part<2;part++){
  cursor=part?hint:label;
  while(*cursor&&drawn<rows){cursor=event_row(cursor,row);text(p,24,y+1+drawn*8,row,part?(color==MUTED?MUTED:CYAN):color);drawn++;}
 }
}
/* Only unresolved decisions are accepted here; repeated input after a choice
   cannot dismiss its outcome or advance another sector. */
/* Negotiation owns input until an explicit original event choice resolves it. */
static void surrender_clear_modals(Ui*u){
 u->page=u->actions=u->controller_focus=u->retreat=u->retreat_confirm=u->home_defense=0;
 u->boarding=u->abandon_confirm=u->crew_details=u->renaming=u->dismiss_confirm=0;
 u->drone_panel=u->drone_sale=u->sale_confirm=u->aiming=u->doors=u->beam_pending=0;
}
void ui_event_key(Campaign*c,Ui*u,int key){
 if(campaign_surrender_pending(c))surrender_clear_modals(u);
 int count=campaign_choice_count(c);
 if(!count||u->page||u->logs||u->drone_panel||u->crew_details||u->boarding)return;
 if(u->event_selected<0||u->event_selected>=count)u->event_selected=0;
 if(key==0||key==1){u->event_selected=(u->event_selected+count+(key?1:-1))%count;ui_feedback(u,"Choice selected. A confirms.",0);return;}
 if(key!=2)return;
 if(!campaign_choice_enabled(c,u->event_selected)){ui_feedback(u,game_away_crew(&c->game)&&(c->event_id==EVENT_EXIT||c->imported_battle==1)?"Recall or abandon away crew first.":c->event_id==EVENT_EXIT?"This sector route is unavailable.":"Choice unavailable: check requirements.",1);return;}
 int ok=campaign_choice(c,u->event_selected);
 ui_feedback(u,ok?"Choice confirmed.":"Choice unavailable: check requirements.",!ok);
 if(ok){u->event_text_page=0;u->event_selected=0;u->aiming=0;u->doors=0;}
}
#include "map_view.inc"
/* Map keys are shared by DS input and host checks. */
void ui_map_key(Campaign*c,Ui*u,int key){
 if(c->mode!=CAM_MAP||u->page||u->drone_panel||u->logs)return;
 if((key==0||key==1)&&c->node_count>0){c->selected=(c->selected+c->node_count+(key?1:-1))%c->node_count;u->feedback_ticks=0;}
 else if(key==3){u->map_overview=!u->map_overview;u->feedback_ticks=0;}
 else if(key==4){c->selected=c->current;u->map_overview=0;u->feedback_ticks=0;}
 else if(key==2){int ok=campaign_wait(c);ui_feedback(u,ok?"Waited one turn. Rebel fleet advanced.":"Cannot wait here.",!ok);}
}
/* Shared DS campaign panels, 256x192 each screen. */
#include "audio.h"

void ui_drone_open(Ui*u){u->drone_panel=1;u->drone_focus=0;u->drone_sale=0;u->drone_selected=0;u->drone_cargo=0;ui_feedback(u,"Drone controls. D-pad / A / B back.",0);}
/* V56 per-kind activity line for an active drone. */
static const char*drone_activity(const DroneState*d){static char s[40];switch(drones_kind(d->catalog_id)){
 case 2:return "BEAM / FIRING";
 case 3:snprintf(s,sizeof s,"HULL REPAIR / %d LEFT",d->remaining);return s;
 case 4:snprintf(s,sizeof s,"REPAIRING / ROOM %d",d->unit_room+1);return s;
 case 5:snprintf(s,sizeof s,"GUARDING / ROOM %d",d->unit_room+1);return s;
 case 6:if(d->unit_room<0)return "BOARDING / IN FLIGHT";snprintf(s,sizeof s,"BOARDED / ENEMY ROOM %d",d->unit_room+1);return s;
 default:return "ACTIVE / FIRING";}}
static const char* drone_label(int id){static const char*const labels[]={"COMBAT I","DEFENSE I","COMBAT II","DEFENSE II","BEAM DRONE","HULL REPAIR","SYS REPAIR","ANTI-PERS","BOARDING"};return id>=0&&id<(int)(sizeof labels/sizeof labels[0])?labels[id]:drones_name(id);}
static int drone_management(const Campaign*c){return c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved);}
/* Shared touch/controller action list: tabs, rows, power, primary, remove,
   reorder, sell, upgrade, back. Inventory mutations stay campaign guarded. */
static void drone_action(Campaign*c,Ui*u,int action){
 Game*g=&c->game;int ok=0,slot=u->drone_selected;
 if(action==0){u->drone_cargo=!u->drone_cargo;u->drone_selected=0;u->drone_focus=0;ui_feedback(u,u->drone_cargo?"Drone cargo selected.":"Installed drones selected.",0);return;}
 if(action>=1&&action<=4){if(!u->drone_cargo&&(!g->drones.installed||action>g->drones.slot_count)){ui_feedback(u,"This drone slot is unavailable.",1);return;}u->drone_selected=action-1;ui_feedback(u,"Drone slot selected.",0);return;}
 if(action==12){u->drone_panel=u->drone_sale=0;return;}
 if(action==5||action==6){ok=drones_adjust_power(g,action==5?-1:1);ui_feedback(u,g->drones.installed?g->message:"Buy drone control at a store.",!ok);return;}
 if(!u->drone_cargo&&action==7){if(!g->drones.installed||slot>=g->drones.slot_count||g->drones.slots[slot].catalog_id<0){ui_feedback(u,"Select an installed drone schematic.",1);return;}if(c->mode!=CAM_COMBAT&&!drones_outside_combat(g->drones.slots[slot].catalog_id)){ui_feedback(u,"Deploy this drone during combat.",1);return;}ok=drones_toggle(g,slot);ui_feedback(u,g->message,!ok);return;}
 if(!drone_management(c)){ui_feedback(u,"Inventory changes require a safe beacon.",1);return;}
 if(action==7||action==8){if(u->drone_cargo)ok=campaign_drone_swap(c,slot,action-7);else ok=campaign_drone_unequip(c,slot);ui_feedback(u,ok?"Drone equipment updated.":"Check control, slot and cargo space.",!ok);}
 else if(action==9){if(u->drone_cargo){ui_feedback(u,"Select installed drones to reorder.",1);return;}ok=campaign_drone_reorder(c,slot,slot==0?1:0);if(ok)u->drone_selected=slot==0?1:0;ui_feedback(u,ok?"Drone power priority updated.":"Select an installed drone.",!ok);}
 else if(action==10){if(c->mode!=CAM_STORE||campaign_drone_sell_price(c,!u->drone_cargo,slot)<0)ui_feedback(u,"Select a drone at a store to sell.",1);else {u->drone_sale=1;u->drone_focus=0;}}
 else if(action==11){ok=campaign_upgrade_drones(c);ui_feedback(u,ok?"Drone control upgraded.":"Check installed control, scrap and cap.",!ok);}
}
void ui_drone_key(Campaign*c,Ui*u,int key){
 if(!u->drone_panel)return;
 if(key==3){if(u->drone_sale)u->drone_sale=0;else u->drone_panel=0;return;}
 if(u->drone_sale){if(key==0||key==1)u->drone_focus=!u->drone_focus;else if(key==2){if(u->drone_focus){int ok=campaign_sell_drone(c,!u->drone_cargo,u->drone_selected);ui_feedback(u,ok?"Selected drone sold.":"Drone sale unavailable.",!ok);}u->drone_sale=0;}return;}
 if(key==0||key==1){u->drone_focus=(u->drone_focus+13+(key?1:-1))%13;return;}
 if(key==2)drone_action(c,u,u->drone_focus);
 else if(key==4)drone_action(c,u,0);
 else if(key==5||key==6)drone_action(c,u,key);
 else if(key==7){game_toggle_pause(&c->game);ui_feedback(u,c->game.paused?"Paused.":"Resumed.",0);}
}
static void drone_button(const Campaign*c,uint16_t*p,const Ui*u,int action,int x,int y,int w,int h,const char*label){
 int enabled=1;const DroneControl*d=&c->game.drones;
 if(action>=1&&action<=4&&!u->drone_cargo)enabled=action<=d->slot_count&&d->installed;
 if(action==5||action==6)enabled=d->installed;
 if(action==7)enabled=u->drone_cargo?drone_management(c):c->mode==CAM_COMBAT&&d->installed;
 if(action==8||action==9)enabled=drone_management(c)&&d->installed&&(action!=9||!u->drone_cargo);
 if(action==10)enabled=c->mode==CAM_STORE&&campaign_drone_sell_price(c,!u->drone_cargo,u->drone_selected)>=0;
 if(action==11){int cost=campaign_drone_upgrade_price(c);enabled=drone_management(c)&&cost>=0&&c->scrap>=cost;}
 button(p,x,y,w,h,label,enabled?(u->drone_focus==action?GOLD:action>=1&&action<=4&&u->drone_selected==action-1?GREEN:CYAN):MUTED);if(!enabled&&u->drone_focus==action)box(p,x,y,w,h,GOLD);
}
static void drone_view(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 const Game*g=&c->game;const DroneControl*d=&g->drones;char s[96];
 rect(bottom,0,0,256,192,BG);text(bottom,5,5,"DRONES / D-PAD A / B BACK",WHITE);
 snprintf(s,sizeof s,"PARTS %d / POWER %d / HP %d/%d",g->drone_parts,drones_effective_power(g),d->health,d->capacity);text(bottom,5,18,s,d->health<d->capacity?RED:CYAN);
 if(d->installed)snprintf(s,sizeof s,"CONTROL R%d / ION %dS / REACTOR %d/%d",d->room+1,(d->ion_ticks+GAME_HZ-1)/GAME_HZ,game_used_power(g),game_reactor(g));else snprintf(s,sizeof s,"BUY CONTROL PACKAGE AT A STORE");text(bottom,5,30,s,MUTED);
 drone_button(c,bottom,u,0,4,42,248,17,u->drone_cargo?"CARGO / Y SWITCH":"INSTALLED / Y SWITCH");
 for(int i=0;i<4;i++){int id=u->drone_cargo?c->drone_cargo[i]:(i<d->slot_count?d->slots[i].catalog_id:-1);snprintf(s,sizeof s,"%c%d %.12s%s",u->drone_cargo?'C':'D',i+1,id>=0?drone_label(id):"EMPTY",i==u->drone_selected?" <":"");if(!u->drone_cargo&&i>=d->slot_count)snprintf(s,sizeof s,"--");drone_button(c,bottom,u,i+1,4+(i%2)*126,62+(i/2)*20,122,18,s);}
 drone_button(c,bottom,u,5,4,104,58,18,"PWR- L");drone_button(c,bottom,u,6,65,104,58,18,"PWR+ R");
 drone_button(c,bottom,u,7,126,104,126,18,u->drone_cargo?"FIT D1":u->drone_selected<d->slot_count&&d->slots[u->drone_selected].enabled?"SUSPEND":"DEPLOY / RESUME");
 drone_button(c,bottom,u,8,4,125,120,18,u->drone_cargo?"FIT D2":"REMOVE");drone_button(c,bottom,u,9,128,125,124,18,"SWAP PRIORITY");
 int cost=campaign_drone_upgrade_price(c);snprintf(s,sizeof s,"UPGRADE %d",cost);drone_button(c,bottom,u,10,4,146,120,18,"SELL SELECTED");drone_button(c,bottom,u,11,128,146,124,18,cost<0?"UPGRADE MAX":s);
 drone_button(c,bottom,u,12,4,174,120,18,"BACK B");button(bottom,128,174,124,18,g->paused?"RESUME X":"PAUSE X",GOLD);
 /* Selected drone detail replaces weapon tiles while this panel is open. */
 rect(top,3,138,249,34,PANEL);
 if(!u->drone_cargo&&u->drone_selected<d->slot_count){int slot=u->drone_selected;const DroneState*dr=&d->slots[slot];const char*status=dr->catalog_id<0?"EMPTY":!dr->enabled?(dr->deployed?"SUSPENDED / FREE RESUME":"NOT DEPLOYED / COST 1 PART"):!drones_powered(g,slot)?"WAITING FOR POWER":drones_is_defense(dr->catalog_id)?(dr->charge_ticks>=drones_cooldown(dr->catalog_id)?"MISSILE DEFENSE / READY":"MISSILE DEFENSE / RELOADING"):drone_activity(dr);text(top,8,143,status,dr->enabled?GREEN:GOLD);
  {int kind=drones_kind(dr->catalog_id);if(kind==3)bar(top,8,160,239,dr->remaining,5,GREEN);else if(kind>=4)bar(top,8,160,239,dr->unit_health,dr->catalog_id==7?150:kind==6?150:100,GREEN);else bar(top,8,160,239,dr->charge_ticks,drones_cooldown(dr->catalog_id),CYAN);}}
 else text(top,8,146,"FIT CARGO TO D1 OR D2 BETWEEN FIGHTS",WHITE);
 if(u->drone_sale){int id=u->drone_cargo?c->drone_cargo[u->drone_selected]:d->slots[u->drone_selected].catalog_id;panel(bottom,3,42,250,123,GOLD);text(bottom,12,51,"SELL THIS DRONE?",GOLD);text(bottom,12,70,drones_name(id),WHITE);snprintf(s,sizeof s,"%s %d / EARN %d SCRAP",u->drone_cargo?"CARGO":"SLOT",u->drone_selected+1,campaign_drone_sell_price(c,!u->drone_cargo,u->drone_selected));text(bottom,12,88,s,WHITE);button(bottom,8,124,115,29,"CANCEL",u->drone_focus?CYAN:GOLD);button(bottom,132,124,115,29,"CONFIRM",u->drone_focus?GOLD:CYAN);}
}
static void drone_touch(Campaign*c,Ui*u,int x,int y){
 u->tap_x=x;u->tap_y=y;u->tap_ticks=8;
 if(u->drone_sale){if(y>=124&&y<153){u->drone_focus=x>=128;ui_drone_key(c,u,2);}return;}
 int a=-1;if(y>=42&&y<59)a=0;else if(y>=62&&y<100)a=1+((y-62)/20)*2+(x>=128);else if(y>=104&&y<122)a=x<64?5:x<125?6:7;else if(y>=125&&y<143)a=x<128?8:9;else if(y>=146&&y<164)a=x<128?10:11;else if(y>=174){if(x>=128){ui_drone_key(c,u,7);return;}a=12;}
 if(a>=0){u->drone_focus=a;drone_action(c,u,a);}
}

/* V54: the original top-bar resource boxes: mint icon square, dark number box. */
static void campaign_resource(uint16_t*p,int x,const uint16_t*icon,int iw,int count,int color){
 char value[16];int w=56-iw;bitmap(p,x,27,icon,iw,15);
 for(int j=0;j<15;j++){rect(p,x+iw,27+j,w-(j>=12?j-11:0),1,j==0||j==14?FTL_MINT:ui_hex_tile[(j&7)*8+((x)&7)]);}
 rect(p,x+iw+w-1,27,1,12,FTL_MINT);
 snprintf(value,sizeof value,"%d",count);text(p,x+iw+(w-(int)strlen(value)*6)/2,31,value,color);
}
static void overview(const Campaign*c,uint16_t*top){
 char s[96];space_background(top);rect(top,0,0,256,20,PANEL);
 snprintf(s,sizeof s,"%s / SECTOR %d/%d",ship_label(&c->game),c->sector+1,c->sector_count);text(top,8,7,s,WHITE);text(top,231,7,"V71",MUTED);
 campaign_resource(top,4,ui_res_fuel,UI_RES_FUEL_WIDTH,c->fuel,c->fuel?FTL_MINT:RED);
 campaign_resource(top,66,ui_res_missiles,UI_RES_MISSILES_WIDTH,c->game.missiles,c->game.missiles?FTL_MINT:RED);
 campaign_resource(top,128,ui_res_drones,UI_RES_DRONES_WIDTH,c->game.drone_parts,FTL_MINT);
 campaign_resource(top,190,ui_res_scrap,UI_RES_SCRAP_WIDTH,c->scrap,FTL_YELLOW);
 panel(top,6,52,244,80,EDGE);player_art_use(c->game.player_ship);bitmap(top,12,58,player_art_combat(),110,70);
 snprintf(s,sizeof s,"HULL %d/%d",c->game.player.hull,c->game.player.max_hull);text(top,138,61,s,WHITE);
 bar(top,138,75,100,c->game.player.hull,c->game.player.max_hull,GREEN);
 snprintf(s,sizeof s,"CREW %d/%d",game_alive_crew(&c->game),c->game.crew_count);text(top,138,87,s,MUTED);
 snprintf(s,sizeof s,"EVADE %d%%",game_evasion(&c->game));text(top,138,102,s,CYAN);
 snprintf(s,sizeof s,"POWER %d/%d",game_used_power(&c->game),game_reactor(&c->game));text(top,138,117,s,GREEN);
 panel(top,6,138,244,50,EDGE);
 const char*cursor=c->text;for(int row=0;row<4&&*cursor;row++){
  char part[39];int n=0,last=-1;while(cursor[n]&&n<38){if(cursor[n]==' ')last=n;n++;}
  if(n==38&&cursor[n]&&last>0)n=last;
  memcpy(part,cursor,(size_t)n);part[n]=0;text(top,12,144+row*10,part,WHITE);cursor+=n;while(*cursor==' ')cursor++;
 }
}
static void manage_view(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];
 if(u->page==1){Ui service=*u;service.aiming=0;render_game(&c->game,&service,top,bottom);overview(c,top);}
 else {overview(c,top);space_background(bottom);}
 static const char*const tabs[]={"CREW","UPGRADE","EQUIP","HELP"};
 rect(bottom,0,0,256,20,BG);for(int i=0;i<4;i++)button(bottom,i*64,0,63,20,tabs[i],(u->page==i+1||(i==1&&u->page==5))?GOLD:MUTED);
 if(u->page==2){
  button(bottom,6,22,244,14,"DOORS / SENSORS UPGRADES >",CYAN);
  for(int i=0;i<6;i++){
   int sys=i-1,cost=campaign_upgrade_price(c,sys);snprintf(s,sizeof s,"%s %d / %s",sys<0?"REACTOR":game_system_name(sys),sys<0?c->game.player.reactor:c->game.player.systems[sys].capacity,cost<0?"MAX":"");
   if(cost>=0)snprintf(s,sizeof s,"%s %d / %d SCRAP",sys<0?"REACTOR":game_system_name(sys),sys<0?c->game.player.reactor:c->game.player.systems[sys].capacity,cost);
   button(bottom,6,38+i*18,244,17,s,cost>=0&&c->scrap>=cost?GREEN:MUTED);
  }
  int price=campaign_teleporter_price(c);
  snprintf(s,sizeof s,"TELEPORT %s / %d SCRAP",c->game.teleporter[0].installed?"UPGRADE":"INSTALL",price);
  button(bottom,6,147,244,22,price<0?"TELEPORT MAX / STORE TO INSTALL":s,price>=0&&c->scrap>=price?GREEN:MUTED);
 }else if(u->page==5){
  text(bottom,8,27,"SUBSYSTEMS / NO REACTOR COST",CYAN);
  for(int k=0;k<2;k++){int cost=campaign_subsystem_upgrade_price(c,k);snprintf(s,sizeof s,"%s LEVEL %d / %d SCRAP",k==SUB_DOORS?"DOORS":"SENSORS",c->game.subsystems[0][k].capacity,cost);button(bottom,6,44+k*36,244,30,cost<0?"SUBSYSTEM AT MAXIMUM":s,cost>=0&&c->scrap>=cost?GREEN:MUTED);}
  {int cost=campaign_cloak_upgrade_price(c);if(!c->game.player_cloak.capacity)snprintf(s,sizeof s,"CLOAKING / BUY AT A STORE");else if(cost<0)snprintf(s,sizeof s,"CLOAKING LEVEL %d / MAX",c->game.player_cloak.capacity);else snprintf(s,sizeof s,"CLOAKING LEVEL %d / %d SCRAP",c->game.player_cloak.capacity,cost);
   if(c->game.player_artillery.capacity>0){char cl[32],ar[32];int ac=campaign_artillery_upgrade_price(c);
    if(!c->game.player_cloak.capacity)snprintf(cl,sizeof cl,"CLOAK: STORE");else if(cost<0)snprintf(cl,sizeof cl,"CLOAK %d MAX",c->game.player_cloak.capacity);else snprintf(cl,sizeof cl,"CLOAK %d / %d",c->game.player_cloak.capacity,cost);
    if(ac<0)snprintf(ar,sizeof ar,"ARTY %d MAX",c->game.player_artillery.capacity);else snprintf(ar,sizeof ar,"ARTY %d / %d",c->game.player_artillery.capacity,ac);
    button(bottom,6,114,120,17,cl,cost>=0&&c->scrap>=cost?GREEN:MUTED);button(bottom,130,114,120,17,ar,ac>=0&&c->scrap>=ac?GREEN:MUTED);}
   else button(bottom,6,114,244,17,s,cost>=0&&c->scrap>=cost?GREEN:MUTED);}
  /* V55 augments (three slots), shown with the ship's upgrades. */
  {int n=0;for(int i=0;i<GAME_MAX_AUGMENTS;i++)if(c->game.augments[i]>=0){snprintf(s,sizeof s,"AUG %d %.30s",i+1,game_augment_name(c->game.augments[i]));text(bottom,8,133+n*8,s,FTL_YELLOW);n++;}
   if(!n)text(bottom,8,138,"NO AUGMENTS / BUY AT STORES",MUTED);}
  button(bottom,6,156,244,17,"< SYSTEM UPGRADES",CYAN);
 }else if(u->page==3){
  panel(top,6,52,244,80,EDGE);text(top,12,58,"POWER PRIORITY: W1 > W2 > W3 > W4",GOLD);
  for(int i=0;i<c->game.weapon_count;i++){const WeaponState*w=&c->game.weapons[i];snprintf(s,sizeof s,"W%d %.20s / P%d",i+1,game_weapon_name(w->catalog_id),w->power);text(top,12,73+i*13,s,u->equipment_installed&&i==u->equipment_slot?GOLD:WHITE);}
  if(!c->game.weapon_count)text(top,12,80,"NO WEAPONS INSTALLED",MUTED);
  button(bottom,4,22,122,14,"CARGO",u->equipment_installed?MUTED:GOLD);button(bottom,130,22,122,14,"INSTALLED",u->equipment_installed?GOLD:MUTED);
  for(int i=0;i<4;i++){int id=u->equipment_installed?(i<c->game.weapon_count?c->game.weapons[i].catalog_id:-1):c->cargo[i];snprintf(s,sizeof s,"%c%d %s",u->equipment_installed?'W':'C',i+1,id<0?"EMPTY":game_weapon_name(id));button(bottom,6,38+i*21,244,20,s,i==(u->equipment_installed?u->equipment_slot:u->cargo_selected)?GOLD:MUTED);}
  int price=campaign_weapon_sell_price(c,u->equipment_installed,u->equipment_installed?u->equipment_slot:u->cargo_selected);snprintf(s,sizeof s,"SELL SELECTED / %d SCRAP",price);
  button(bottom,6,124,244,18,c->mode!=CAM_STORE?"SELL AT A STORE":price<0?"SELECT A WEAPON TO SELL":s,c->mode==CAM_STORE&&price>=0?GREEN:MUTED);
  if(u->equipment_installed){button(bottom,2,145,98,24,"REMOVE",CYAN);button(bottom,104,145,73,24,"< LEFT",CYAN);button(bottom,181,145,73,24,"RIGHT >",CYAN);}
  else for(int i=0;i<GAME_MAX_WEAPONS;i++){snprintf(s,sizeof s,"W%d %s",i+1,i<c->game.weapon_count?"SWAP":"ADD");button(bottom,2+i*64,145,62,24,s,i<=c->game.weapon_count?CYAN:MUTED);}
 }else if(u->page==4){
  static const char*const help[]={"DPAD: CHOOSE A ROOM","L/R: CREW; A: MOVE","Y: AIM; L/R: WEAPON","AIM: DPAD ROOM; A: FIRE","X: PAUSE / B: BACK","START: POWER, DOORS, MORE","SOUND: CYCLE VOLUME BELOW"};
  for(int i=0;i<7;i++)text(bottom,8,30+i*19,help[i],i==5?GOLD:WHITE);
  text(bottom,8,162,audio_status(),CYAN);
 }
 rect(bottom,0,174,256,18,BG);button(bottom,2,175,66,17,c->mode==CAM_STORE?"STORE":c->mode==CAM_EVENT?"RETURN":"MAP",CYAN);
 button(bottom,116,175,44,17,"DRONES",CYAN);button(bottom,72,175,42,17,u->page==1?(c->game.paused?"RESUME":"PAUSE"):"CREW",GOLD);
 snprintf(s,sizeof s,"VOL %s",audio_volume_label());button(bottom,161,175,93,17,s,MUTED);
 if(u->sale_confirm&&u->page==3){int index=u->equipment_installed?u->equipment_slot:u->cargo_selected;int id=u->equipment_installed?(index<c->game.weapon_count?c->game.weapons[index].catalog_id:-1):(index<CAM_CARGO_COUNT?c->cargo[index]:-1);int price=campaign_weapon_sell_price(c,u->equipment_installed,index);panel(bottom,3,25,250,144,GOLD);text(bottom,12,38,"SELL THIS WEAPON?",GOLD);text(bottom,12,57,id>=0?game_weapon_name(id):"EMPTY SLOT",WHITE);snprintf(s,sizeof s,"%s SLOT %d / EARN %d SCRAP",u->equipment_installed?"WEAPON":"CARGO",index+1,price);text(bottom,12,76,s,WHITE);text(bottom,12,95,"THE SELECTED ITEM WILL BE REMOVED.",MUTED);button(bottom,8,127,115,29,"CANCEL",CYAN);button(bottom,132,127,115,29,"CONFIRM SALE",GOLD);}

}
static void render_campaign_scene(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];
 if(u->home_defense&&!u->logs&&campaign_can_service_home(c)){render_game(&c->game,u,top,bottom);rect(bottom,0,175,256,17,BG);button(bottom,2,176,120,16,"RETURN / B",CYAN);button(bottom,128,176,125,16,c->game.paused?"RESUME / X":"PAUSE / X",GOLD);return;}
 if(c->mode==CAM_COMBAT||u->logs){render_game(&c->game,u,top,bottom);if(!u->logs&&c->game.outcome!=GAME_PLAYING)outcome_view(c,top,bottom);return;}
 if(u->page>0&&(c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved))){manage_view(c,u,top,bottom);return;}
 overview(c,top);space_background(bottom);
 if(c->mode==CAM_MAP){
  sector_map_view(c,u,top,bottom);
 }else if(c->mode==CAM_STORE){
  rect(bottom,0,0,256,24,PANEL);text(bottom,8,8,"STORE / UP-DOWN / A BUY",GOLD);
  for(int j=0;j<4;j++){int i=u->shop_page*4+j;if(i>=campaign_offer_count(c))continue;int stock=c->stock[c->current][i],price=campaign_offer_price(c,i);snprintf(s,sizeof s,"%s",campaign_offer_name(c,i));panel(bottom,4,29+j*29,248,27,u->shop_selected==j?GOLD:stock>0?CYAN:EDGE);text(bottom,10,33+j*29,s,WHITE);if(i==7)snprintf(s,sizeof s,"SELECT EXACT ITEM");else snprintf(s,sizeof s,"%s %d / STOCK %d",price<0?"EARN":"COST",price<0?-price:price,stock);text(bottom,10,44+j*29,s,stock>0?GREEN:MUTED);}
  button(bottom,4,149,120,21,"SHIP / Y",CYAN);button(bottom,130,149,122,21,"MORE OFFERS",GOLD);
  button(bottom,4,175,248,17,"LEAVE / B",WHITE);
 }else{
  int terminal=c->mode==CAM_COMPLETE||c->mode==CAM_DEFEAT;
  if(c->mode==CAM_EVENT&&c->event_id==EVENT_EXIT&&campaign_route_count(c)&&campaign_choice_count(c))sector_choice_view(c,u,top);
  else {panel(top,4,52,248,136,terminal?GOLD:CYAN);event_narrative(c,u,top);}
  rect(bottom,0,0,256,29,PANEL);text(bottom,9,10,c->mode==CAM_COMPLETE?"CAMPAIGN COMPLETE":c->mode==CAM_DEFEAT?"SHIP LOST":campaign_surrender_pending(c)?"SURRENDER OFFER":c->game.outcome==GAME_SURRENDER&&c->imported_battle==3?"SURRENDER ACCEPTED":c->event_id==EVENT_EXIT&&campaign_route_count(c)?"CHOOSE NEXT SECTOR":"INCOMING TRANSMISSION",GOLD);
  int pages=ui_event_text_pages(c);
  if(pages>1){button(bottom,4,29,122,14,"PREV TEXT / L",u->event_text_page>0?CYAN:MUTED);button(bottom,130,29,122,14,"NEXT TEXT / R",u->event_text_page<pages-1?CYAN:MUTED);}
  int count=campaign_choice_count(c);
  if(count){
   int routes=c->event_id==EVENT_EXIT?campaign_route_count(c):0;
   int selected=u->event_selected>=0&&u->event_selected<count?u->event_selected:0;
   if(!routes)panel(bottom,2,42,252,124,EDGE); /* One original event window holds every reply. */
   for(int i=0;i<count;i++){
    int enabled=campaign_choice_enabled(c,i),y=routes?42+i*59:45+i*choice_step(count),h=routes?54:choice_height(count);
    int color=enabled?(i==selected?FTL_YELLOW:campaign_choice_blue(c,i)?RGB(112,181,255):WHITE):MUTED;
    if(routes){panel(bottom,4,y,248,h,color);snprintf(s,sizeof s,"%c %s",i==selected?'>':' ',campaign_choice_name(c,i));text(bottom,10,y+6,s,color);
     const char*hint=campaign_sector_summary(c->next_sector[i]);
     for(int row=0;row<3&&*hint;row++){char part[39];int n=0,last=-1;while(hint[n]&&n<38){if(hint[n]==' ')last=n;n++;}if(n==38&&hint[n]&&last>0)n=last;memcpy(part,hint,(size_t)n);part[n]=0;text(bottom,10,y+19+row*10,part,MUTED);hint+=n;while(*hint==' ')hint++;}
    }else {char hint[96];campaign_choice_hint(c,i,hint,sizeof hint);event_choice_button(bottom,y,h,choice_rows(count),i+1,i==selected,campaign_choice_name(c,i),hint,color);}
    if(routes&&!enabled&&i==selected)box(bottom,4,y,248,h,GOLD);
   }
   text(bottom,12,169,"UP/DOWN SELECT / A CONFIRM",MUTED);if(campaign_can_service_home(c))button(bottom,4,179,248,13,campaign_home_threat(c)?"DANGER: DEFEND SHIP / Y":"DEFEND / REPAIR SHIP / Y",campaign_home_threat(c)?RED:CYAN);else text(bottom,12,181,campaign_surrender_pending(c)?"COMBAT HELD UNTIL YOU CHOOSE":"OR TAP A CHOICE",MUTED);
  }
  else {if(!terminal)button(bottom,8,132,240,25,"SHIP MANAGEMENT / Y",CYAN);text(bottom,12,52,terminal?"RETURN TO THE MAIN MENU":"CONTINUE WHEN READY",WHITE);button(bottom,8,168,240,24,terminal?"MAIN MENU / START":"CONTINUE / A",GOLD);}
 }
}
static void campaign_touch_action(Campaign*c,Ui*u,int x,int y){
 if(x<0||x>=256||y<0||y>=192)return;
 if(u->logs){u->logs=0;return;}
 if(c->mode==CAM_COMBAT){ui_touch(&c->game,u,x,y);return;}
 if(u->sale_confirm){if(y>=127&&y<156){if(x>=132&&x<247){int ok=campaign_sell_weapon(c,u->equipment_installed,u->equipment_installed?u->equipment_slot:u->cargo_selected);ui_feedback(u,ok?"Selected weapon sold.":"Sale unavailable. Check the selected item.",!ok);u->weapon=0;u->beam_pending=0;u->sale_confirm=0;}else if(x>=8&&x<123)u->sale_confirm=0;}return;}

 if(u->page>0&&(c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved))){
  if(y<20){u->page=1+x/64;return;}
  if(y>=175){if(x<70)u->page=0;else if(x<160){if(u->page==1)game_toggle_pause(&c->game);else u->page=1;}else audio_cycle_volume();return;}
  if(u->page==1){u->aiming=0;ui_touch(&c->game,u,x,y);return;}
  if(u->page==2&&y>=22&&y<36){u->page=5;return;}
  if(u->page==5&&y>=114&&y<131&&c->game.player_artillery.capacity>0&&x>=130){int ok=campaign_upgrade_artillery(c);ui_feedback(u,ok?"Artillery upgraded. Allocate power.":"Check scrap or maximum level.",!ok);return;}
  if(u->page==5&&y>=114&&y<131){int ok=campaign_upgrade_cloak(c);ui_feedback(u,ok?"Cloaking upgraded. Allocate power.":c->game.player_cloak.capacity?"Check scrap or maximum level.":"Buy cloaking at a store.",!ok);return;}
  if(u->page==5){if(y>=44&&y<110){int k=(y-44)/36;if(k<2){int ok=campaign_upgrade_subsystem(c,k);ui_feedback(u,ok?"Subsystem upgraded.":"Check scrap or maximum level.",!ok);}}else if(y>=156&&y<173)u->page=2;return;}
  if(u->page==2&&y>=147&&y<169){int ok=c->game.teleporter[0].installed?campaign_upgrade_teleporter(c):campaign_install_teleporter(c);ui_feedback(u,ok?"Teleporter updated. Allocate power.":"Check store, scrap or maximum level.",!ok);return;}
  if(u->page==2&&y>=38&&y<146){int sys=(y-38)/18-1;if(!campaign_upgrade(c,sys))ui_feedback(u,campaign_upgrade_price(c,sys)<0?"System already at maximum capacity.":"Not enough scrap for this upgrade.",1);else ui_feedback(u,"Upgrade installed. Allocate power.",0);return;}
  if(u->page==3){
   if(y>=22&&y<36){u->equipment_installed=x>=128;return;}
   if(y>=38&&y<122){int index=(y-38)/21;if(u->equipment_installed)u->equipment_slot=index;else u->cargo_selected=index;return;}
   if(y>=124&&y<142){int index=u->equipment_installed?u->equipment_slot:u->cargo_selected;if(c->mode!=CAM_STORE)ui_feedback(u,"Weapons can only be sold at stores.",1);else if(campaign_weapon_sell_price(c,u->equipment_installed,index)<0)ui_feedback(u,"Select an occupied weapon slot.",1);else u->sale_confirm=1;return;}
   if(y>=145&&y<169){int ok;if(u->equipment_installed){int slot=u->equipment_slot;if(x<102){ok=campaign_unequip(c,slot);ui_feedback(u,ok?"Weapon moved to cargo.":"Select a weapon and free a cargo slot.",!ok);}else{int to=slot+(x<179?-1:1);ok=campaign_reorder_weapon(c,slot,to);if(ok)u->equipment_slot=to;ui_feedback(u,ok?"Weapon priority updated. Left powers first.":"Cannot move beyond installed slots.",!ok);}}
    else{ok=campaign_swap(c,u->cargo_selected,x/64);ui_feedback(u,ok?"Weapon installed. Left slots power first.":"Select occupied cargo and a valid slot.",!ok);}
    u->weapon=0;u->beam_pending=0;return;}
   return;
  }
  return;
 }
 if(c->mode==CAM_MAP){
  if(y<30){if(x>=195)ui_map_key(c,u,2);else if(x>=130)ui_map_key(c,u,3);else ui_map_key(c,u,4);return;}
  if(y>=174){if(x>=132){if(c->fuel){if(!campaign_can_jump(c,c->selected))ui_feedback(u,"Cannot jump: select a linked beacon.",1);else campaign_jump(c,c->selected);}else campaign_rescue(c);u->aiming=0;u->doors=0;}else if(x>=67)u->page=1;return;}
  int nearest=-1,distance=170;
  if(y>=34&&y<143)for(int i=0;i<c->node_count;i++){int nx,ny;if(!ui_map_node_position(c,u,i,&nx,&ny))continue;int dx=x-nx,dy=y-ny,d=dx*dx+dy*dy;if(d<distance){distance=d;nearest=i;}}
  if(nearest>=0)c->selected=nearest;
 }else if(c->mode==CAM_STORE){
  if(y>=175)campaign_continue(c);
  else if(y>=149&&y<170){if(x<128)u->page=1;else u->shop_page=(u->shop_page+1)%((campaign_offer_count(c)+3)/4);}
  else if(y>=29&&y<145){int i=u->shop_page*4+(y-29)/29;if(i>=campaign_offer_count(c))return;if(i==7){u->page=3;u->sale_confirm=0;return;}if(!campaign_purchase(c,i)){
 const char*reason=c->stock[c->current][i]<=0?"This offer is sold out.":c->scrap<campaign_offer_price(c,i)?"Not enough scrap.":i==2?"Hull is already fully repaired.":i==3?"Crew capacity reached.":i==11?"Drone control already installed.":(i==12||i==14)?"Drone cargo full. Fit or sell a drone.":i==13?"Drone parts limit reached.":i==7?"No cargo available to sell.":"Cargo is full. Sell or install a weapon.";
 ui_feedback(u,reason,1);
 }else ui_feedback(u,i==7?"Cargo sold.":"Purchase complete.",0);}
 }else{
  if(c->mode==CAM_EVENT&&ui_event_text_pages(c)>1&&y>=29&&y<43){ui_event_text_key(c,u,x>=128);return;}
  int n=campaign_choice_count(c);
  int routes=c->event_id==EVENT_EXIT?campaign_route_count(c):0;
  int first=routes?42:45,step=routes?59:choice_step(n),height=routes?54:choice_height(n);
  if(n&&x>=4&&x<252&&y>=first&&y<first+n*step&&(y-first)%step<height){u->event_selected=(y-first)/step;ui_event_key(c,u,2);}
  else if(!n&&c->mode==CAM_EVENT&&y>=132&&y<158){u->page=1;}
  else if(!n&&y>=168){if(c->mode==CAM_COMPLETE||c->mode==CAM_DEFEAT)u->menu_requested=1;else campaign_continue(c);}
  u->aiming=0;u->doors=0;
 }
}

void render_campaign(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 if(campaign_surrender_pending(c)&&!u->logs){Ui offer=*u;surrender_clear_modals(&offer);offer.feedback_ticks=offer.tap_ticks=0;render_campaign_scene(c,&offer,top,bottom);return;}
 if((c->mode==CAM_DEFEAT||c->mode==CAM_COMPLETE)&&!u->logs){Ui result=*u;result.page=result.home_defense=result.retreat=result.boarding=result.crew_details=result.drone_panel=0;render_campaign_scene(c,&result,top,bottom);return;}
 if(c->reward.kind!=REWARD_NONE&&!u->logs){reward_view(c,u,top,bottom);feedback_view(u,top,bottom);return;}
 /* Result presentation takes precedence over any modal left open at impact. */
 if(c->mode==CAM_COMBAT&&c->game.outcome!=GAME_PLAYING&&!u->logs){
  Ui result=*u;result.boarding=0;result.crew_details=0;result.renaming=0;
  result.feedback_ticks=0;result.tap_ticks=0;
  render_campaign_scene(c,&result,top,bottom);return;
 }
 render_campaign_scene(c,u,top,bottom);
 if(c->mode==CAM_COMBAT&&!u->logs&&!u->boarding&&!u->crew_details&&!u->drone_panel)retreat_controls(c,u,bottom);
 if(u->boarding&&!u->logs)board_view(&c->game,u,bottom);
 if(u->crew_details&&!u->logs){crew_species_view(&c->game,u,top);crew_detail_view(&c->game,u,bottom,c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved));}
 if(u->drone_panel&&!u->logs)drone_view(c,u,top,bottom);
 if(u->retreat&&!u->logs&&c->mode==CAM_COMBAT)retreat_view(c,u,top,bottom);
 if(u->actions&&!u->logs&&(c->mode==CAM_COMBAT||u->home_defense||u->page==1))combat_actions_view(c,u,bottom);
 if(!u->logs)feedback_view(u,top,bottom);
}
void campaign_touch(Campaign*c,Ui*u,int x,int y){
 if(campaign_surrender_pending(c)&&!u->logs){if(x<0||x>=256||y<0||y>=192)return;surrender_clear_modals(u);u->tap_x=x;u->tap_y=y;u->tap_ticks=8;campaign_touch_action(c,u,x,y);return;}
 if(c->reward.kind!=REWARD_NONE&&!u->logs&&c->mode!=CAM_DEFEAT&&c->mode!=CAM_COMPLETE){if(x>=0&&x<256&&y>=0&&y<192){u->tap_x=x;u->tap_y=y;u->tap_ticks=8;reward_touch(c,u,x,y);}return;}
 if(c->mode==CAM_COMBAT&&c->game.outcome!=GAME_PLAYING)return;
 if(x<0||x>=256||y<0||y>=192)return;
 if(u->actions&&!u->logs){ui_combat_touch(c,u,x,y);return;}
 if(!u->logs){u->tap_x=x;u->tap_y=y;u->tap_ticks=8;}
 if(u->retreat&&!u->logs){retreat_touch(c,u,x,y);return;}
 if(!u->logs&&u->home_defense&&!u->boarding&&!u->crew_details){if(y>=175){if(x<128)u->home_defense=0;else game_toggle_pause(&c->game);}else ui_touch(&c->game,u,x,y);return;}
 if(!u->logs&&!u->page&&c->mode==CAM_EVENT&&!c->event_resolved&&y>=179&&campaign_can_service_home(c)){ui_home_open(c,u);return;}
 if(!u->logs&&!u->crew_details&&!u->boarding&&!u->drone_panel&&c->mode==CAM_COMBAT&&y>=176){if(x<45){game_toggle_pause(&c->game);ui_feedback(u,c->game.paused?"Paused.":"Resumed.",0);}else if(x<108)ui_fire(&c->game,u);else if(x<156)ui_drone_open(u);else if(x<206)ui_retreat_open(c,u);else{u->aiming=!u->aiming;u->beam_pending=0;ui_feedback(u,u->aiming?"Targeting opened.":"Crew controls.",0);}return;}
 if(u->drone_panel&&!u->logs){drone_touch(c,u,x,y);return;}
 if(!u->logs&&!u->crew_details&&!u->boarding&&((c->mode==CAM_COMBAT&&y>=176&&x>=136&&x<188)||(u->page&&drone_management(c)&&y>=175&&x>=116&&x<161))){ui_drone_open(u);return;}
 if(u->crew_details){u->tap_x=x;u->tap_y=y;u->tap_ticks=8;crew_detail_touch(&c->game,u,x,y,c->mode==CAM_MAP||c->mode==CAM_STORE||(c->mode==CAM_EVENT&&c->event_resolved));if(u->dismiss_confirm==2){int ok=campaign_dismiss_crew(c,u->selected_crew);ui_feedback(u,ok?"Crew dismissed.":"Cannot dismiss: away, dead or last crew.",!ok);u->dismiss_confirm=0;if(ok){u->crew_details=0;u->selected_crew=0;u->crew_page=u->board_page=0;u->crew_group=u->board_party=0;u->group_mode=0;}}return;}
 if(u->boarding){u->tap_x=x;u->tap_y=y;u->tap_ticks=8;board_touch(&c->game,u,x,y);if(u->abandon_confirm==2){int n=campaign_abandon_boarders(c);ui_feedback(u,n?"Away crew abandoned.":"Abandon unavailable during combat.",!n);u->abandon_confirm=0;}return;}
 int imported_node=c->imported_node;
 int page=u->page,selected=c->selected,cargo=u->cargo_selected,shop=u->shop_page,mode=c->mode,sound=audio_volume_level(),paused=c->game.paused;
 char previous[sizeof c->text];snprintf(previous,sizeof previous,"%s",c->text);
 u->tap_x=x;u->tap_y=y;u->tap_ticks=8;u->feedback_error=0;
 campaign_touch_action(c,u,x,y);
 if(mode!=c->mode||imported_node!=c->imported_node){u->event_text_page=0;u->event_selected=0;}
 if(mode==CAM_COMBAT)return;
 if(page!=u->page)ui_feedback(u,u->page?"Ship page opened.":"Returned to campaign.",0);
 else if(selected!=c->selected)ui_feedback(u,campaign_can_jump(c,c->selected)?"Beacon selected. Ready to jump.":"Select a linked beacon to jump.",!campaign_can_jump(c,c->selected));
 else if(cargo!=u->cargo_selected)ui_feedback(u,"Cargo selected. Choose weapon slot.",0);
 else if(shop!=u->shop_page)ui_feedback(u,"Store offers changed.",0);
 else if(sound!=audio_volume_level()){char volume[48];snprintf(volume,sizeof volume,"Volume: %s",audio_volume_label());ui_feedback(u,volume,0);}
 else if(paused!=c->game.paused)ui_feedback(u,c->game.paused?"Paused.":"Resumed.",0);
 else if(strcmp(previous,c->text))ui_feedback(u,c->text,strstr(c->text,"unavailable")!=0);
}
