#include "render.h"
#include "audio.h"
#include "event_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned before;uint16_t p[256*192];unsigned after;} top,bottom;
static uint16_t first[256*192];
static void draw(Campaign*c,Ui*u){top.before=bottom.before=17;top.after=bottom.after=31;render_campaign(c,u,top.p,bottom.p);assert(top.before==17&&bottom.before==17&&top.after==31&&bottom.after==31);}
static int rows(const char*s){int count=0;while(*s){int n=0,last=-1;while(s[n]&&s[n]!='\n'&&n<38){if(s[n]==' ')last=n;n++;}if(n==38&&s[n]&&last>0)n=last;s+=n;while(*s==' '||*s=='\n'||*s=='\r')s++;count++;}return count;}
int main(void){
 Campaign c,before;Ui u={0};int paged=0,four=0,blue=0,disabled=0;audio_init();
 for(int root=0;root<campaign_imported_count();root++){
  campaign_init_seed(&c,42);c.scrap=0;c.fuel=0;c.game.missiles=0;c.game.drone_parts=0;
  assert(campaign_imported_begin(&c,root));u=(Ui){0};before=c;draw(&c,&u);
  assert(!memcmp(&before,&c,sizeof c));
  int pages=ui_event_text_pages(&c),count=campaign_choice_count(&c);
  assert(pages>=1&&count<=6);
  if(count==4){four++;u.event_selected=3;draw(&c,&u);campaign_touch(&c,&u,80,137);assert(!memcmp(&before,&c,sizeof c));
   Campaign expected=c;campaign_choice(&expected,3);campaign_touch(&c,&u,80,151);assert(!memcmp(&expected,&c,sizeof c));c=before;u=(Ui){0};
  }
  for(int i=0;i<count;i++){
   blue+=campaign_choice_blue(&c,i)!=0;
   if(!campaign_choice_enabled(&c,i)){u.event_selected=i;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c)&&u.feedback_error);disabled++;}
  }
  if(pages>1)paged++;

 }

 /* Touch and physical confirm follow the same staged station engagement. */
 for(int root=6;root<8;root++){
  campaign_init_seed(&c,42);assert(campaign_imported_begin(&c,root));u=(Ui){0};
  Campaign expected=c;assert(campaign_choice(&expected,0));
  campaign_touch(&c,&u,80,60);assert(!memcmp(&expected,&c,sizeof c));
  assert(c.imported_battle==1&&campaign_choice_count(&c)==1&&!strcmp(campaign_choice_name(&c,0),"Engage"));
  draw(&c,&u);before=c;ui_event_key(&c,&u,0);assert(!memcmp(&before,&c,sizeof c));
  assert(campaign_choice(&expected,0));ui_event_key(&c,&u,2);assert(!memcmp(&expected,&c,sizeof c)&&c.mode==CAM_COMBAT);
  before=c;ui_event_key(&c,&u,2);assert(!memcmp(&before,&c,sizeof c));
 }

 /* Every compiled choice, including nested combat rewards, fits its box (V62: rows by choice count). */
 for(int node=0;node<IMPORTED_EVENT_NODE_COUNT;node++){
  campaign_init_seed(&c,42);c.mode=CAM_EVENT;c.event_id=EVENT_IMPORTED;c.event_resolved=0;c.imported_node=node;u=(Ui){0};
  for(int choice=0;choice<campaign_choice_count(&c);choice++){
   char hint[96];campaign_choice_hint(&c,choice,hint,sizeof hint);
   {static const int limit[7]={6,6,6,4,3,3,2};int n=campaign_choice_count(&c);assert(rows(campaign_choice_name(&c,choice))+rows(hint)<=limit[n>6?6:n]);} /* V62 choice_rows. */
  }
  draw(&c,&u);
 }
 assert(four&&blue&&disabled);
 /* Synthetic long narrative proves pagination even while selected original
    roots fit one page. Keep the compiled choice state for reset coverage. */
 campaign_init_seed(&c,42);c.fuel=0;assert(campaign_imported_begin(&c,0));u=(Ui){0};c.text[0]=0;
 for(int line=0;line<23;line++){char part[64];snprintf(part,sizeof part,"Narrative line %02d remains readable.\n",line+1);strcat(c.text,part);}
 assert(strlen(campaign_text(&c))>256);assert(ui_event_text_pages(&c)==3);before=c;
 draw(&c,&u);memcpy(first,top.p,sizeof first);campaign_touch(&c,&u,190,35);
 assert(u.event_text_page==1&&!memcmp(&before,&c,sizeof c));draw(&c,&u);
 assert(memcmp(first+62*256,top.p+62*256,107*256*sizeof(uint16_t)));
 for(int p=0;p<5;p++)ui_event_text_key(&c,&u,1);
 assert(u.event_text_page==2);draw(&c,&u);
 u.logs=1;ui_event_text_key(&c,&u,0);assert(u.event_text_page==2);u.logs=0;
 for(int p=0;p<5;p++)ui_event_text_key(&c,&u,0);
 assert(u.event_text_page==0&&!memcmp(&before,&c,sizeof c));ui_event_text_key(&c,&u,1);
 for(int i=0;i<campaign_choice_count(&c);i++)if(campaign_choice_enabled(&c,i)){u.event_selected=i;ui_event_key(&c,&u,2);assert(u.event_text_page==0);break;}
 campaign_init_seed(&c,42);u=(Ui){.event_text_page=5};before=c;ui_event_text_key(&c,&u,1);assert(u.event_text_page==5&&!memcmp(&before,&c,sizeof c));
 printf("Original event UI: %d paged roots, %d four-choice roots, %d blue choices, %d disabled guards; synthetic three-page navigation, bounds and reset passed.\n",paged,four,blue,disabled);return 0;
}
