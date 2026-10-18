#include "setup.h"
#include <string.h>
#include <stdio.h>
void crew_setup_begin_ship(CrewSetup*s,int layout,int count){
 static const char*names[GAME_START_CREW_MAX]={"Alex","Sam","Jordan","Morgan"};
 memset(s,0,sizeof *s);s->active=1;s->layout=layout;s->count=count<1?1:count>GAME_START_CREW_MAX?GAME_START_CREW_MAX:count;
 for(int i=0;i<s->count;i++)snprintf(s->names[i],GAME_CREW_NAME_SIZE,"%s",names[i]);
}
void crew_setup_begin(CrewSetup*s){crew_setup_begin_ship(s,0,GAME_CREW_COUNT);}
int crew_setup_row_y(const CrewSetup*s,int i){return s->count<=3?32+i*34:30+i*27;}
int crew_setup_row_h(const CrewSetup*s){return s->count<=3?30:25;}
void crew_setup_apply(const CrewSetup*s,Game*g){
 for(int i=0;i<s->count&&i<g->crew_count;i++)game_rename_crew(g,i,s->names[i]);
}
static void edit(CrewSetup*s){s->editing=1;s->key=0;s->error=0;memcpy(s->draft,s->names[s->selected],sizeof s->draft);}
static void erase(CrewSetup*s){size_t n=strlen(s->draft);if(n)s->draft[n-1]=0;s->error=0;}
static void save_name(CrewSetup*s){
 int start=0,end=(int)strlen(s->draft);
 while(start<end&&s->draft[start]==' ')start++;
 while(end>start&&s->draft[end-1]==' ')end--;
 if(end==start){s->error=1;return;}
 memset(s->names[s->selected],0,GAME_CREW_NAME_SIZE);
 memcpy(s->names[s->selected],s->draft+start,(size_t)(end-start));s->editing=0;s->error=0;
}
static void type_key(CrewSetup*s){
 size_t n=strlen(s->draft);int k=s->key;
 if(k==29){erase(s);return;}
 if(k==30){s->editing=0;s->error=0;return;}
 if(k==31){save_name(s);return;}
 if(n>=GAME_CREW_NAME_SIZE-1)return;
 s->draft[n]=k<26?(char)('A'+k):k==26?' ':k==27?'-':39;s->draft[n+1]=0;s->error=0;
}
int crew_setup_key(CrewSetup*s,int key){
 if(!s->active)return SETUP_NONE;
 if(s->editing){
  if(key==SETUP_BACK){s->editing=0;s->error=0;}
  else if(key==SETUP_ERASE)erase(s);
  else if(key==SETUP_ACCEPT)type_key(s);
  else if(key==SETUP_UP)s->key=(s->key+24)%32;
  else if(key==SETUP_DOWN)s->key=(s->key+8)%32;
  else if(key==SETUP_LEFT)s->key=(s->key+31)%32;
  else if(key==SETUP_RIGHT)s->key=(s->key+1)%32;
  return SETUP_NONE;
 }
 if(key==SETUP_BACK){s->active=0;return SETUP_CANCEL;}
 if(key==SETUP_UP)s->selected=(s->selected+s->count)%(s->count+1);
 else if(key==SETUP_DOWN)s->selected=(s->selected+1)%(s->count+1);
 else if(key==SETUP_ACCEPT){if(s->selected==s->count){s->active=0;return SETUP_LAUNCH;}edit(s);}
 return SETUP_NONE;
}
int crew_setup_touch(CrewSetup*s,int x,int y){
 if(!s->active||x<0||x>=256||y<0||y>=192)return SETUP_NONE;
 if(s->editing){
  if(y>=58&&y<178){s->key=((y-58)/30)*8+x/32;type_key(s);}
  return SETUP_NONE;
 }
 if(x>=12&&x<244){
  for(int i=0;i<s->count;i++)if(y>=crew_setup_row_y(s,i)&&y<crew_setup_row_y(s,i)+crew_setup_row_h(s)){s->selected=i;edit(s);return SETUP_NONE;}
  if(y>=140&&y<168){s->selected=s->count;return crew_setup_key(s,SETUP_ACCEPT);}
  if(y>=174)return crew_setup_key(s,SETUP_BACK);
 }
 return SETUP_NONE;
}
