/* V45 finale difficulty: main-menu choice for new runs, per-run persistence in
 * save27 (older saves take the DS tuning) and application to flagship fights. */
#include "menu.h"
#include "render.h"
#include "save.h"
#include "audio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static Campaign c,d;
static uint16_t top[256*192],bottom[256*192],tuned[256*192];
static unsigned char raw[16384];
static void reach_flagship(Campaign*k,unsigned seed,int original){
 campaign_init_seed(k,seed);k->flagship_original=original;for(int i=1;i<k->sector_count-1;i++)k->sector_types[i]=SECTOR_REBEL;
 k->sector=k->sector_count-2;k->next_sector[0]=SECTOR_FINAL;k->mode=CAM_EVENT;k->event_id=EVENT_EXIT;k->event_resolved=0;
 assert(campaign_choice(k,0)&&campaign_finale_active(k));
 int boss=campaign_boss_node(k),from=-1;for(int i=0;i<k->node_count&&from<0;i++)if(campaign_linked(k,i,boss)&&i!=k->node_count-1)from=i;
 k->current=k->selected=from;k->visited[from]=1;k->fuel=20;k->boss_move=0;k->fleet_nodes&=~(1u<<boss);
 assert(campaign_jump(k,boss)&&k->mode==CAM_COMBAT&&game_is_flagship(&k->game));
}
int main(void){
 MainMenu m;audio_init();memset(&m,0,sizeof m);main_menu_init(&m,1);
 /* Six items (V64 adds TUTORIAL third); FINALE toggles in place and defaults to the DS tuning. */
 assert(!m.finale_original);for(int i=0;i<MAIN_MENU_ITEMS;i++)main_menu_key(&m,1);assert(m.selected==0);
 render_main_menu(&m,top,bottom);memcpy(tuned,bottom,sizeof tuned);
 assert(main_menu_touch(&m,20,100)==MENU_NONE&&m.active&&m.selected==3&&m.finale_original==1);
 render_main_menu(&m,top,bottom);assert(memcmp(tuned,bottom,sizeof tuned));
 main_menu_key(&m,2);assert(!m.finale_original);main_menu_key(&m,2);assert(m.finale_original);
 /* The preference survives menu reinitialization and reopening. */
 main_menu_init(&m,0);assert(m.finale_original&&m.selected==1);main_menu_open(&m,1);assert(m.finale_original);
 assert(main_menu_touch(&m,20,139)==MENU_VOLUME);main_menu_init(&m,1);assert(main_menu_touch(&m,20,115)==MENU_NONE&&m.page);
 /* New campaigns default to the DS tuning; flagship fights follow the run's choice. */
 campaign_init_seed(&c,5);assert(!c.flagship_original);
 reach_flagship(&c,77,0);assert(!c.game.flagship_authentic);int tuned_shields=c.game.enemy.systems[SYS_SHIELDS].capacity;
 reach_flagship(&c,77,1);assert(c.game.flagship_authentic);assert(c.game.enemy.systems[SYS_SHIELDS].capacity>tuned_shields);
 /* Save27 round trip; range check; save26 migrates to the DS tuning. */
 campaign_init_seed(&c,31);c.flagship_original=1;c.event_policy=5; /* Readable as save26 below. */assert(campaign_save(&c,"tests/finale-choice.sav"));
 campaign_init_seed(&d,1);assert(campaign_load(&d,"tests/finale-choice.sav")&&d.flagship_original==1&&d.seed==31);
 FILE*f=fopen("tests/finale-choice.sav","rb");size_t n=fread(raw,1,sizeof raw,f);fclose(f);assert(raw[8]==32);
 for(int variant=0;variant<2;variant++){
  /* save28-32 append 324 bytes (V53 event bits, V55 cloak, augments, store, V59 achievement progress, V61-62 quests) after the finale word. */
  size_t len=variant?n-324:n;unsigned char b[16384];memcpy(b,raw,n);
  if(variant)b[8]=26;else{b[len-324]=2;b[len-323]=b[len-322]=b[len-321]=0;}
  b[12]=(unsigned char)((len-20)&255);b[13]=(unsigned char)((len-20)>>8);b[14]=b[15]=0;
  unsigned h=2166136261u;for(size_t i=20;i<len;i++)h=(h^b[i])*16777619u;for(int i=0;i<4;i++)b[16+i]=(unsigned char)(h>>(8*i));
  remove("tests/finale-choice.sav.bak");f=fopen("tests/finale-choice.sav","wb");fwrite(b,1,len,f);fclose(f);
  campaign_init_seed(&d,1);d.flagship_original=1;
  if(variant){assert(campaign_load(&d,"tests/finale-choice.sav")&&d.seed==31&&!d.flagship_original);}
  else{Campaign before=d;assert(!campaign_load(&d,"tests/finale-choice.sav")&&!memcmp(&before,&d,sizeof d));}
 }
 remove("tests/finale-choice.sav");remove("tests/finale-choice.sav.bak");
 puts("PASS: V45 finale menu choice, per-run save27 persistence, save26 migration and flagship application.");return 0;
}
