#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "render.h"
#include "save.h"
#include "audio.h"
static Campaign c,before;
static uint16_t top[256*192+2],bottom[256*192+2],title[256*192],repeat_bottom[256*192];
int main(void){
 MainMenu m={0};audio_init();main_menu_init(&m,0);
 assert(m.active&&m.selected==1);m.selected=0;
 assert(main_menu_key(&m,2)==MENU_NONE&&m.active);
 assert(main_menu_key(&m,3)==MENU_NONE&&m.active);
 assert(main_menu_touch(&m,20,65)==MENU_NEW_RUN&&!m.active);
 main_menu_init(&m,1);assert(main_menu_key(&m,2)==MENU_RESUME&&!m.active);
 main_menu_open(&m,1);m.selected=1;assert(main_menu_key(&m,2)==MENU_NONE&&m.confirm&&!m.confirm_yes);
 assert(main_menu_key(&m,2)==MENU_NONE&&!m.confirm&&m.active);
 main_menu_key(&m,2);main_menu_key(&m,1);assert(main_menu_key(&m,2)==MENU_NEW_RUN&&!m.active);
 main_menu_open(&m,1);assert(main_menu_key(&m,3)==MENU_RESUME);
 main_menu_init(&m,1);main_menu_touch(&m,20,115);assert(m.page);main_menu_key(&m,3);assert(!m.page);
 assert(main_menu_touch(&m,20,139)==MENU_VOLUME);
 campaign_init_seed(&c,123);before=c;
 assert(!campaign_load(&c,"tests/missing-directory/no-save.sav"));assert(!memcmp(&c,&before,sizeof c));
 assert(campaign_save(&c,"tests/main-menu.sav"));c.scrap=999;
 assert(campaign_load(&c,"tests/main-menu.sav"));assert(c.scrap==before.scrap);
 assert(campaign_save(&c,"tests/main-menu.sav"));
 FILE*f=fopen("tests/main-menu.sav","wb");assert(f);fputs("corrupt",f);fclose(f);
 assert(campaign_load(&c,"tests/main-menu.sav")); /* Valid backup remains selectable. */
 before=c;main_menu_init(&m,1);
 top[0]=top[256*192+1]=bottom[0]=bottom[256*192+1]=0x1234;
 render_main_menu(&m,top+1,bottom+1);memcpy(title,top+1,sizeof title);
 for(int page=0;page<2;page++)for(int confirm=0;confirm<2;confirm++)for(int selected=0;selected<MAIN_MENU_ITEMS;selected++){
  m.page=page;m.confirm=confirm;m.selected=selected;
  render_main_menu(&m,top+1,bottom+1);
  assert(!memcmp(title,top+1,sizeof title));
  memcpy(repeat_bottom,bottom+1,sizeof repeat_bottom);
  render_main_menu(&m,top+1,bottom+1);
  assert(!memcmp(title,top+1,sizeof title));
  assert(!memcmp(repeat_bottom,bottom+1,sizeof repeat_bottom));
  assert(top[0]==0x1234&&top[256*192+1]==0x1234&&bottom[0]==0x1234&&bottom[256*192+1]==0x1234);
 }
 assert(!memcmp(&c,&before,sizeof c));
 /* Opening/operating menus never invokes save: loaded checkpoint survives new-run decision. */
 main_menu_init(&m,1);main_menu_touch(&m,20,65);assert(m.confirm);
 assert(main_menu_touch(&m,20,160)==MENU_NEW_RUN);campaign_init_seed(&c,999);
 assert(campaign_load(&c,"tests/main-menu.sav"));assert(c.seed==before.seed);
 puts("main menu actions, save preservation, storage failure and framebuffer bounds passed");return 0;
}
