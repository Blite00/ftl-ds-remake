#include "menu.h"
void main_menu_init(MainMenu*m,int saved_available){int finale=m->finale_original;*m=(MainMenu){.active=1,.saved_available=!!saved_available,.selected=saved_available?0:1,.finale_original=finale};}
void main_menu_open(MainMenu*m,int live_run){m->active=1;m->live_run=!!live_run;m->selected=live_run?0:1;m->page=m->confirm=m->confirm_yes=0;}
static int accept(MainMenu*m){
 if(m->confirm){if(!m->confirm_yes){m->confirm=0;return MENU_NONE;}m->active=0;m->confirm=0;m->live_run=1;return MENU_NEW_RUN;}
 if(m->selected==0){if(m->live_run||m->saved_available){m->active=0;m->live_run=1;return MENU_RESUME;}}
 else if(m->selected==1){if(m->live_run||m->saved_available){m->confirm=1;m->confirm_yes=0;}else{m->active=0;m->live_run=1;return MENU_NEW_RUN;}}
 else if(m->selected==2){m->active=0;return MENU_TUTORIAL;}
 else if(m->selected==3)m->finale_original=!m->finale_original;
 else if(m->selected==4)m->page=1;
 else return MENU_VOLUME;
 return MENU_NONE;
}
int main_menu_key(MainMenu*m,int key){
 if(!m->active)return MENU_NONE;
 if(m->page){if(key==2||key==3)m->page=0;return MENU_NONE;}
 if(m->confirm){if(key==3)m->confirm=0;else if(key==0||key==1)m->confirm_yes=!m->confirm_yes;else if(key==2)return accept(m);return MENU_NONE;}
 if(key==0||key==1){m->selected=(m->selected+(key?1:MAIN_MENU_ITEMS-1))%MAIN_MENU_ITEMS;return MENU_NONE;}
 if(key==3){if(m->live_run){m->active=0;return MENU_RESUME;}return MENU_NONE;}
 if(key==2)return accept(m);
 return MENU_NONE;
}
int main_menu_touch(MainMenu*m,int x,int y){
 if(!m->active||x<12||x>=244)return MENU_NONE;
 if(m->page){if(y>=157&&y<185)m->page=0;return MENU_NONE;}
 if(m->confirm){if(y>=112&&y<142){m->confirm_yes=0;return accept(m);}if(y>=148&&y<178){m->confirm_yes=1;return accept(m);}return MENU_NONE;}
 for(int i=0;i<MAIN_MENU_ITEMS;i++)if(y>=MAIN_MENU_Y+i*MAIN_MENU_STEP&&y<MAIN_MENU_Y+i*MAIN_MENU_STEP+21){m->selected=i;return accept(m);}
 return MENU_NONE;
}
