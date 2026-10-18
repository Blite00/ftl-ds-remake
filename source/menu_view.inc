/* V54: the original title art (img/main_menus/main_base2.png, letterboxed) on top;
 * menu entries in original frames below, yellow when selected. */
void render_main_menu(const MainMenu*m,uint16_t*top,uint16_t*bottom){
 bitmap(top,0,0,ui_menu_back,UI_MENU_BACK_WIDTH,UI_MENU_BACK_HEIGHT);space_background(bottom);
 text(top,6,8,"DS EDITION",FTL_MINT);
 text(top,6,174,"KESTREL CAMPAIGN",FTL_YELLOW);
 text(top,226,184,"V71",MUTED);
 if(m->page){
  text(bottom,18,10,"FLIGHT CONTROLS",CYAN);
  text(bottom,18,30,"MAP: L/R SELECT  A JUMP",WHITE);
  text(bottom,18,44,"UP ZOOM  B CENTER  X SAVE",WHITE);
  text(bottom,18,58,"Y SHIP  START MENU",WHITE);
  text(bottom,18,80,"COMBAT: D-PAD ROOM",WHITE);
  text(bottom,18,94,"L/R CREW OR WEAPON",WHITE);
  text(bottom,18,108,"A ORDER  Y AIM  X PAUSE",WHITE);
  text(bottom,18,122,"B CLEAR/BACK  START ACTIONS",WHITE);
  text(bottom,18,142,"TOUCH ALSO SUPPORTED",MUTED);
  button(bottom,12,157,232,28,"A / B  BACK",CYAN);return;
 }
 if(m->confirm){
  text(bottom,18,14,"START A NEW RUN?",GOLD);
  text(bottom,18,40,m->live_run?"CURRENT RUN WILL BE LEFT.":"START FRESH WITH THE KESTREL.",WHITE);
  text(bottom,18,59,"YOUR SAVED FILE STAYS SAFE",MUTED);
  text(bottom,18,73,"UNTIL YOU SAVE A NEW RUN.",MUTED);
  button(bottom,12,112,232,30,"CANCEL",m->confirm_yes?MUTED:CYAN);
  button(bottom,12,148,232,30,"START NEW RUN",m->confirm_yes?GOLD:MUTED);return;
 }
 text(bottom,18,10,"MAIN MENU",CYAN);
 const char*labels[]={m->live_run?"RESUME RUN":"CONTINUE","NEW RUN","TUTORIAL","FINALE","CONTROLS","SOUND"};
 for(int i=0;i<MAIN_MENU_ITEMS;i++){
  char line[40];int enabled=i!=0||m->live_run||m->saved_available;
  snprintf(line,sizeof line,i==3||i==5?"%s: %s":"%s",labels[i],i==3?(m->finale_original?"ORIGINAL":"TUNED"):audio_volume_label());
  int color=!enabled?EDGE:m->selected==i?GOLD:WHITE;
  int y=MAIN_MENU_Y+i*MAIN_MENU_STEP;panel(bottom,12,y,232,21,color);
  if(m->selected==i)text(bottom,20,y+7,">",color);
  text_scale(bottom,36,y+4,line,ftl_tone(color),2);
 }
 text(bottom,18,165,m->live_run?"B RESUME   UP/DOWN + A":m->saved_available?"SAVED CAMPAIGN AVAILABLE":"NO VALID SAVE / SD UNAVAILABLE",MUTED);
 text(bottom,18,179,m->selected==5?audio_status():m->selected==2?"GUIDED KESTREL LESSONS / SELECT SKIPS":m->selected==3?(m->finale_original?"NEW RUNS: ORIGINAL NORMAL FLAGSHIP":"NEW RUNS: DS-TUNED FLAGSHIP"):"TOUCH AN OPTION TO SELECT",MUTED);
}
