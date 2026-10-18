#include <nds.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "game.h"
#include "subsystems.h"
#include "render.h"
#include "diagnostics.h"
#include "performance.h"
#include "save.h"
#include "audio.h"
#include "input_capture.h"
#include "input_view.h"
#include "setup.h"
#include "pacing.h"
#include "hangar.h"
#include "profile.h"
#include "tutorial.h"
#include "achievements.h"
static uint16_t top[256*192] __attribute__((aligned(32)));
static uint16_t bottom[256*192] __attribute__((aligned(32)));
static Campaign campaign;
static MainMenu main_menu,menu_before_setup;
static CrewSetup setup;
static Hangar hangar;
static Tutorial tutorial;
static MainMenu menu_before_tutorial;
static Profile profile;
static Pacing pacing;
static Ui ui={.focus_room=16,.weapon=1};
static void present(int bg,int sub){
 struct {InputView normal;int setup_active,setup_editing;} view={0};input_view_context(&campaign,&ui,&main_menu,&view.normal);view.setup_active=setup.active;view.setup_editing=setup.editing;ds_input_view_begin(&view,sizeof view);
 if(tutorial.active)render_tutorial(&tutorial,top,bottom);
 else if(setup.active)render_crew_setup(&setup,top,bottom);
 else if(hangar.active)render_hangar(&hangar,top,bottom);
 else if(main_menu.active)render_main_menu(&main_menu,top,bottom);
 else render_campaign(&campaign,&ui,top,bottom);
 DC_FlushRange(top,sizeof top);DC_FlushRange(bottom,sizeof bottom);
 dmaCopy(top,bgGetGfxPtr(bg),sizeof top);dmaCopy(bottom,bgGetGfxPtr(sub),sizeof bottom);
 ds_input_view_end();
}
static void save_run(unsigned frame){
 int ok=campaign_save(&campaign,"/ftl-ds-campaign.sav");
 snprintf(campaign.text,sizeof campaign.text,"%s",ok?"Campaign saved. Resume on next launch.":"Save failed. Check writable SD storage.");
 ui_feedback(&ui,ok?"Campaign saved.":"Save failed. Check SD storage.",!ok);
 diagnostics_action(frame,campaign.text);diagnostics_flush();
}
static void new_run(unsigned frame,int layout){
 unsigned seed=(unsigned)time(NULL)^frame^campaign.rng;
 if(!campaign_init_ship(&campaign,seed?seed:1,layout))campaign_init_seed(&campaign,seed?seed:1);
 ui=(Ui){.focus_room=16,.weapon=1};
}
/* Only changes that stop or switch simulation reset elapsed time. Ordinary
 * aiming, weapon and crew-page changes keep the combat clock running. */
static int simulation_context(void){
 if(campaign.reward.kind!=REWARD_NONE||ui.logs||ui.crew_details||ui.retreat||
    (ui.actions&&(campaign.mode!=CAM_COMBAT||campaign.game.outcome==GAME_PLAYING)))return 0;
 if(ui.home_defense)return 3;
 if(campaign.mode==CAM_COMBAT)return 1;
 int management=campaign.mode==CAM_MAP||campaign.mode==CAM_STORE||(campaign.mode==CAM_EVENT&&campaign.event_resolved);
 return !ui.drone_panel&&(ui.page==1||ui.boarding)&&management?2:0;
}
static int management_available(void){return campaign.mode==CAM_MAP||campaign.mode==CAM_STORE||(campaign.mode==CAM_EVENT&&campaign.event_resolved);}
/* DS buttons use the same controller state machine exercised by host tests. */
static void combat_buttons(int down){
 static const int masks[]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_A,KEY_B,KEY_X,KEY_Y,KEY_L,KEY_R,KEY_START};
 static const int actions[]={UI_COMBAT_UP,UI_COMBAT_DOWN,UI_COMBAT_LEFT,UI_COMBAT_RIGHT,UI_COMBAT_A,UI_COMBAT_B,UI_COMBAT_X,UI_COMBAT_Y,UI_COMBAT_L,UI_COMBAT_R,UI_COMBAT_START};
 for(int i=0;i<11;i++)if(down&masks[i])ui_combat_key(&campaign,&ui,actions[i]);
}

int main(void){
 videoSetMode(MODE_5_2D);videoSetModeSub(MODE_5_2D);
 vramSetBankA(VRAM_A_MAIN_BG);vramSetBankC(VRAM_C_SUB_BG);
 int bg=bgInit(3,BgType_Bmp16,BgSize_B16_256x256,0,0);
 int sub=bgInitSub(3,BgType_Bmp16,BgSize_B16_256x256,0,0);
 lcdMainOnTop();diagnostics_init(0);
 int saved_available=campaign_load(&campaign,"/ftl-ds-campaign.sav");
 int profile_status=profile_load(&profile,PROFILE_PATH);
 main_menu_init(&main_menu,saved_available);
 diagnostics_action(0,saved_available?"Valid campaign available in main menu":"Main menu: no valid save or storage unavailable");
 diagnostics_action(0,profile_status==1?"Profile loaded":profile_status==2?"Profile restored from backup":"Profile defaults (Kestrel A)");
 diagnostics_poll(&campaign.game,0);diagnostics_defer_automatic_flush(1);performance_init();
 /* V63: nitroFSInit() (called by audio_init) switches the current directory to the
  * read-only nitro:/ drive; restore the SD card directory so saves, the profile and
  * the log keep writing to the card (found in melonDS with DLDI). */
 {char home[256];int have=getcwd(home,sizeof home)!=NULL;audio_init();if(have)chdir(home);}
 diagnostics_action(0,audio_status());
 ds_input_init();unsigned frame=0;present(bg,sub);pacing_reset(&pacing,ds_input_vblank_count());
 while(1){
  swiWaitForVBlank();audio_service();unsigned steps=pacing_advance(&pacing,ds_input_vblank_count());CapturedInput input;ds_input_take(&input);
  int down=(int)input.keys,input_received=down!=0;touchPosition touch={0};
  if(down&KEY_TOUCH){touch.px=(u16)input.x;touch.py=(u16)input.y;}
  diagnostics_input(frame,(unsigned)down,down&KEY_TOUCH?touch.px:-1,down&KEY_TOUCH?touch.py:-1);
  /* V64 tutorial: its own Game and state; never touches the campaign save. */
  if(tutorial.active){
   static const int masks[]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_A,KEY_B,KEY_X,KEY_Y,KEY_L,KEY_R,KEY_START,KEY_SELECT};
   int action=TUTORIAL_NONE;
   for(int i=0;i<TUT_KEY_COUNT&&action==TUTORIAL_NONE;i++)if(down&masks[i])action=tutorial_key(&tutorial,i);
   if(action==TUTORIAL_NONE&&(down&KEY_TOUCH))action=tutorial_touch(&tutorial,touch.px,touch.py);
   for(unsigned s=0;s<steps&&tutorial.active;s++)tutorial_tick(&tutorial);
   if(input_received)audio_feedback(0);
   if(action==TUTORIAL_EXIT||!tutorial.active){main_menu=menu_before_tutorial;main_menu.confirm=0;main_menu_open(&main_menu,main_menu.live_run);}
   audio_update(&tutorial.game,tutorial.active?(tutorial.combat?AUDIO_COMBAT:AUDIO_MAP):AUDIO_MENU);
   if(input_received||(frame&1)==0)present(bg,sub);
   frame++;continue;
  }
  if(setup.active){
   int action=SETUP_NONE;
   if(down&KEY_UP)action=crew_setup_key(&setup,SETUP_UP);
   else if(down&KEY_DOWN)action=crew_setup_key(&setup,SETUP_DOWN);
   else if(down&KEY_LEFT)action=crew_setup_key(&setup,SETUP_LEFT);
   else if(down&KEY_RIGHT)action=crew_setup_key(&setup,SETUP_RIGHT);
   else if(down&KEY_A)action=crew_setup_key(&setup,SETUP_ACCEPT);
   else if(down&KEY_B)action=crew_setup_key(&setup,SETUP_BACK);
   else if(down&KEY_X)action=crew_setup_key(&setup,SETUP_ERASE);
   else if(down&KEY_TOUCH)action=crew_setup_touch(&setup,touch.px,touch.py);
   if(action==SETUP_LAUNCH){new_run(frame,setup.layout);campaign.flagship_original=main_menu.finale_original;crew_setup_apply(&setup,&campaign.game);}
   else if(action==SETUP_CANCEL){int cls=hangar.cls,variant=hangar.variant;hangar_open(&hangar,&profile);hangar.cls=cls;hangar.variant=variant;}
   audio_update(&campaign.game,AUDIO_MENU);
   if(input_received)present(bg,sub);
   pacing_reset(&pacing,ds_input_vblank_count());frame++;continue;
  }
  if(hangar.active){
   int action=HANGAR_NONE;
   if(down&KEY_UP)action=hangar_key(&hangar,HANGAR_UP);
   else if(down&KEY_DOWN)action=hangar_key(&hangar,HANGAR_DOWN);
   else if(down&KEY_LEFT)action=hangar_key(&hangar,HANGAR_LEFT);
   else if(down&KEY_RIGHT)action=hangar_key(&hangar,HANGAR_RIGHT);
   else if(down&(KEY_Y|KEY_L|KEY_R))action=hangar_key(&hangar,HANGAR_TOGGLE);
   else if(down&KEY_A)action=hangar_key(&hangar,HANGAR_A);
   else if(down&KEY_B)action=hangar_key(&hangar,HANGAR_B);
   else if(down&KEY_TOUCH)action=hangar_touch(&hangar,touch.px,touch.py);
   if(input_received)audio_feedback(0);
   if(action==HANGAR_START){GameLayoutInfo info;game_layout_info(hangar_layout(&hangar),&info);crew_setup_begin_ship(&setup,hangar_layout(&hangar),info.crew_count);}
   else if(action==HANGAR_BACK){main_menu=menu_before_setup;main_menu.confirm=0;main_menu.active=1;}
   audio_update(&campaign.game,AUDIO_MENU);
   if(input_received)present(bg,sub);
   pacing_reset(&pacing,ds_input_vblank_count());frame++;continue;
  }
  if(main_menu.active){
   MainMenu previous_menu=main_menu;
   int action=MENU_NONE;
   if(down&KEY_UP)action=main_menu_key(&main_menu,0);
   else if(down&KEY_DOWN)action=main_menu_key(&main_menu,1);
   else if(down&KEY_A)action=main_menu_key(&main_menu,2);
   else if(down&KEY_B)action=main_menu_key(&main_menu,3);
   else if(down&KEY_TOUCH)action=main_menu_touch(&main_menu,touch.px,touch.py);
   if(input_received)audio_feedback(0);
   if(action==MENU_NEW_RUN){menu_before_setup=previous_menu;profile_load(&profile,PROFILE_PATH);hangar_open(&hangar,&profile);}
   else if(action==MENU_RESUME)campaign_check_unlocks(&campaign);
   else if(action==MENU_TUTORIAL){menu_before_tutorial=previous_menu;tutorial_begin(&tutorial);}
   else if(action==MENU_VOLUME)audio_cycle_volume();
   audio_update(&campaign.game,AUDIO_MENU);
   if(input_received||(frame&1)==0)present(bg,sub);
   pacing_reset(&pacing,ds_input_vblank_count());frame++;continue;
  }
  int clock_before=simulation_context(),was_paused=campaign.game.paused;
  int input_mode=campaign.mode,input_beacon=campaign.current,input_event=campaign.imported_node;
  if(down&KEY_SELECT){ui.logs=!ui.logs;diagnostics_action(frame,ui.logs?"open logs: simulation held":"close logs");diagnostics_flush();down=0;}
  if(ui.logs){if(down&(KEY_TOUCH|KEY_B)){ui.logs=0;diagnostics_action(frame,"close logs");}down=0;}
  Game*g=&campaign.game;
  if(campaign_surrender_pending(&campaign)){
   if(down&KEY_UP)ui_event_key(&campaign,&ui,0);
   if(down&KEY_DOWN)ui_event_key(&campaign,&ui,1);
   if(down&KEY_L)ui_event_text_key(&campaign,&ui,0);
   if(down&KEY_R)ui_event_text_key(&campaign,&ui,1);
   if(down&KEY_A){ui_event_key(&campaign,&ui,2);down&=~KEY_TOUCH;}
  }else if(campaign.reward.kind!=REWARD_NONE){
   if(down&(KEY_UP|KEY_LEFT))ui_reward_key(&campaign,&ui,0);
   if(down&(KEY_DOWN|KEY_RIGHT))ui_reward_key(&campaign,&ui,1);
   if(down&KEY_A){ui_reward_key(&campaign,&ui,2);down&=~KEY_TOUCH;}
   if(down&KEY_B){ui_reward_key(&campaign,&ui,3);down&=~KEY_TOUCH;}
  }else if(campaign.mode==CAM_COMBAT&&g->outcome!=GAME_PLAYING){
   if(down&(KEY_START|KEY_X))g->paused=!g->paused;
  }else if(ui.retreat){
   if(down&(KEY_L|KEY_LEFT|KEY_UP))ui_retreat_key(&campaign,&ui,0);
   if(down&(KEY_R|KEY_RIGHT|KEY_DOWN))ui_retreat_key(&campaign,&ui,1);
   if(down&KEY_A){ui_retreat_key(&campaign,&ui,2);down&=~KEY_TOUCH;}
   if(down&KEY_B){ui_retreat_key(&campaign,&ui,3);down&=~KEY_TOUCH;}
  }else if(ui.actions||(ui.home_defense&&!ui.boarding&&!ui.crew_details)){
   combat_buttons(down);if(down&~KEY_TOUCH)down&=~KEY_TOUCH;
  }else if(ui.drone_panel){
   if(campaign.mode==CAM_COMBAT||ui.home_defense||(ui.page==1&&management_available())){combat_buttons(down);if(down&~KEY_TOUCH)down&=~KEY_TOUCH;}
   else {
   if(down&(KEY_UP|KEY_LEFT))ui_drone_key(&campaign,&ui,0);
   if(down&(KEY_DOWN|KEY_RIGHT))ui_drone_key(&campaign,&ui,1);
   if(down&KEY_A)ui_drone_key(&campaign,&ui,2);
   if(down&KEY_B)ui_drone_key(&campaign,&ui,3);
   if(down&KEY_Y)ui_drone_key(&campaign,&ui,4);
   if(down&KEY_L)ui_drone_key(&campaign,&ui,5);
   if(down&KEY_R)ui_drone_key(&campaign,&ui,6);
   if(down&KEY_X)ui_drone_key(&campaign,&ui,7);
   }
  }else if(!ui.crew_details&&!ui.boarding&&!ui.sale_confirm&&(down&KEY_START)&&(ui.page>1&&management_available())){ui_drone_open(&ui);down&=~KEY_TOUCH;
  }else if(ui.crew_details){
   if(down&KEY_B){if(ui.dismiss_confirm)ui.dismiss_confirm=0;else if(ui.renaming)ui.renaming=0;else ui.crew_details=0;}
   if(!ui.renaming&&!ui.dismiss_confirm&&(down&KEY_X)){game_save_stations(g);ui_feedback(&ui,"Crew stations saved.",0);}
   if(!ui.renaming&&!ui.dismiss_confirm&&(down&KEY_Y)){game_return_stations(g);ui_feedback(&ui,"Returning crew to saved stations.",0);}
  }else if(ui.boarding){
   if(campaign.mode==CAM_COMBAT||ui.home_defense||(ui.page==1&&management_available())){combat_buttons(down);if(down&~KEY_TOUCH)down&=~KEY_TOUCH;}
   else {
    if(down&KEY_B){if(ui.abandon_confirm)ui.abandon_confirm=0;else ui.boarding=0;}
    if(!ui.abandon_confirm){if(down&KEY_X)game_toggle_pause(g);if(down&KEY_A)ui_board_action(g,&ui,0);if(down&KEY_Y)ui_board_action(g,&ui,1);}
   }
  }else if(campaign.mode==CAM_COMBAT){
   combat_buttons(down);if(down&~KEY_TOUCH)down&=~KEY_TOUCH;
  }else if(ui.page==1&&management_available()){
   combat_buttons(down);if(down&~KEY_TOUCH)down&=~KEY_TOUCH;
  }else if(ui.page&&management_available()){
   if(ui.page==4&&(down&KEY_A)){audio_cycle_volume();ui_feedback(&ui,audio_volume_label(),0);}
   if(down&KEY_B){if(ui.sale_confirm)ui.sale_confirm=0;else ui.page=0;}
   if((down&KEY_Y)&&!ui.sale_confirm)ui.page=ui.page%4+1;
   if(down&(KEY_START|KEY_X)){game_toggle_pause(g);ui_feedback(&ui,g->paused?"Paused.":"Resumed.",0);}
  }else if(campaign.mode==CAM_MAP){
   if(down&(KEY_R|KEY_RIGHT))ui_map_key(&campaign,&ui,1);
   if(down&(KEY_L|KEY_LEFT))ui_map_key(&campaign,&ui,0);
   if(down&KEY_DOWN)ui_map_key(&campaign,&ui,2);
   if(down&KEY_UP)ui_map_key(&campaign,&ui,3);
   if(down&KEY_B)ui_map_key(&campaign,&ui,4);
   if(down&KEY_A){if(campaign.fuel){if(campaign_can_jump(&campaign,campaign.selected))campaign_jump(&campaign,campaign.selected);else ui_feedback(&ui,"Cannot jump: select a linked beacon.",1);}else campaign_rescue(&campaign);}
   if(down&KEY_Y){ui.page=1;ui_feedback(&ui,"Ship management opened.",0);}
   if(down&KEY_START){main_menu_open(&main_menu,1);down=0;}
   if(down&KEY_X)save_run(frame);
   if((down&KEY_TOUCH)&&touch.py>=174&&touch.px<65){save_run(frame);down&=~KEY_TOUCH;}
  }else if(campaign.mode==CAM_STORE){
   if(down&KEY_B)campaign_continue(&campaign);
   if(down&KEY_UP)ui.shop_selected=(ui.shop_selected+3)%4;
   if(down&KEY_DOWN)ui.shop_selected=(ui.shop_selected+1)%4;
   if(down&KEY_A)campaign_touch(&campaign,&ui,80,35+ui.shop_selected*29);
   if(down&KEY_Y){ui.page=1;ui_feedback(&ui,"Ship management opened.",0);}
   if(down&(KEY_L|KEY_R)){int pages=(campaign_offer_count(&campaign)+3)/4;ui.shop_page=(ui.shop_page+pages+(down&KEY_R?1:-1))%pages;}
  }else if(campaign.mode==CAM_EVENT){
   if(down&KEY_L)ui_event_text_key(&campaign,&ui,0);
   if(down&KEY_R)ui_event_text_key(&campaign,&ui,1);
   if(down&KEY_Y){if(campaign.event_resolved)ui.page=1;else ui_home_open(&campaign,&ui);}
   if(campaign_choice_count(&campaign)){
    if(down&KEY_UP)ui_event_key(&campaign,&ui,0);
    if(down&KEY_DOWN)ui_event_key(&campaign,&ui,1);
    if(down&KEY_A){ui_event_key(&campaign,&ui,2);down&=~KEY_TOUCH;}
   }else if(down&(KEY_A|KEY_START)){campaign_continue(&campaign);down&=~KEY_TOUCH;}
  }else if((down&(KEY_A|KEY_START))||((down&KEY_TOUCH)&&touch.py>=168)){
   main_menu_init(&main_menu,campaign_load(&campaign,"/ftl-ds-campaign.sav"));ui=(Ui){.focus_room=16,.weapon=1};down=0;
  }
  if((down&KEY_TOUCH)&&input_mode==campaign.mode&&input_beacon==campaign.current)campaign_touch(&campaign,&ui,touch.px,touch.py);
  if(ui.menu_requested){main_menu_init(&main_menu,campaign_load(&campaign,"/ftl-ds-campaign.sav"));ui=(Ui){.focus_room=16,.weapon=1};}
  if(input_beacon!=campaign.current)audio_jump();
  if(input_mode!=campaign.mode||input_beacon!=campaign.current||input_event!=campaign.imported_node){ui.actions=ui.controller_focus=0;ui.retreat=ui.retreat_confirm=ui.home_defense=0;ui.event_text_page=0;ui.event_selected=0;ui.aiming=0;ui.doors=0;ui.page=0;ui.boarding=0;ui.abandon_confirm=0;ui.crew_details=0;ui.renaming=0;ui.dismiss_confirm=0;ui.sale_confirm=0;ui.drone_panel=ui.drone_sale=0;ui.group_mode=0;ui.beam_pending=0;diagnostics_action(frame,campaign.text);}
  if(main_menu.active){diagnostics_flush();audio_update(g,AUDIO_MENU);present(bg,sub);pacing_reset(&pacing,ds_input_vblank_count());frame++;continue;}
  /* V59: achievements earned this frame join the profile; 2 of a cruiser's 3 unlock its Type B. */
  if(campaign.game.ach.earned){unsigned fresh;int id=-1;profile_load(&profile,PROFILE_PATH);fresh=profile_add_achievements(&profile,campaign.game.ach.earned);campaign.game.ach.earned=0;
   for(int k=0;k<ACH_COUNT;k++)if(fresh&(1u<<k))id=k;
   if(id>=0){int fresh_id=id;
    int ok=profile_save(&profile,PROFILE_PATH);char note[64];snprintf(note,sizeof note,"Achievement: %.40s",achievement_name(fresh_id));ui_feedback(&ui,ok?note:"Achievement earned, but the profile could not be saved.",!ok);diagnostics_action(frame,note);}}
  /* V57: ships unlocked this frame go straight into the SD profile (with backup). */
  if(campaign.unlock_pending){int any=0;profile_load(&profile,PROFILE_PATH);
   int shown=-1;for(int k=0;k<game_layout_count();k++)if(campaign.unlock_pending&(1u<<k)&&profile_unlock_layout(&profile,k)){any=1;shown=k;}
   campaign.unlock_pending=0;
   if(any){int ok=profile_save(&profile,PROFILE_PATH);char note[64];snprintf(note,sizeof note,"NEW SHIP UNLOCKED: %.30s! See the hangar.",game_layout_name(shown));ui_feedback(&ui,ok?note:"Ship unlocked, but the profile could not be saved.",!ok);diagnostics_action(frame,ok?"Profile: ship unlocked and saved":"Profile save failed");}}
  if(was_paused!=campaign.game.paused||clock_before!=simulation_context()||input_mode!=campaign.mode||input_beacon!=campaign.current||input_event!=campaign.imported_node){steps=0;pacing_reset(&pacing,ds_input_vblank_count());}
  if(steps||input_received){
   performance_begin();
   for(unsigned step=0;step<steps;step++){
   int oldmode=campaign.mode,oldoutcome=campaign.game.outcome;ui_tick(&ui);
   /* Menus and narrative freeze time. Servicing runs only on the crew page. */
   ui_campaign_step(&campaign,&ui);
   if(oldmode!=campaign.mode){ui.actions=ui.controller_focus=0;ui.retreat=ui.retreat_confirm=ui.home_defense=0;ui.event_text_page=0;ui.event_selected=0;ui.aiming=0;ui.doors=0;ui.page=0;ui.boarding=0;ui.abandon_confirm=0;ui.crew_details=0;ui.renaming=0;ui.dismiss_confirm=0;ui.sale_confirm=0;ui.drone_panel=ui.drone_sale=0;ui.group_mode=0;ui.beam_pending=0;diagnostics_action(frame,campaign.text);}
   enum AudioMode mode=campaign.mode==CAM_COMBAT?(campaign.boss_phase>=0?AUDIO_FINALE:AUDIO_COMBAT):campaign.mode==CAM_COMPLETE?AUDIO_VICTORY:campaign.mode==CAM_DEFEAT?AUDIO_DEFEAT:AUDIO_MAP;
   audio_update(g,ui.logs?AUDIO_SILENT:mode);
   if(oldmode!=campaign.mode||oldoutcome!=campaign.game.outcome||clock_before!=simulation_context()){pacing_reset(&pacing,ds_input_vblank_count());break;}
   }
   diagnostics_poll(g,frame);present(bg,sub);performance_end();
   if(frame&&frame%600==0){char report[160];performance_report(report,sizeof report);diagnostics_action(frame,report);}
  }
  frame++;
 }
}
