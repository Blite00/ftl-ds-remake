#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
static Game g,before;
static void tap_key(CrewSetup*s,int k){crew_setup_touch(s,k%8*32+10,58+k/8*30+10);}
int main(void){
 CrewSetup s;game_init(&g);before=g;crew_setup_begin(&s);
 assert(s.active&&!s.editing);
 for(int i=0;i<GAME_CREW_COUNT;i++)assert(!strcmp(s.names[i],g.crew[i].name));
 /* A new-run setup neither ticks nor modifies the existing campaign. */
 crew_setup_key(&s,SETUP_ACCEPT);assert(s.editing);
 for(int i=0;i<4;i++)crew_setup_key(&s,SETUP_ERASE);
 tap_key(&s,25);tap_key(&s,14);tap_key(&s,4);tap_key(&s,31);
 assert(!s.editing&&!strcmp(s.names[0],"ZOE"));assert(!memcmp(&g,&before,sizeof g));
 crew_setup_key(&s,SETUP_DOWN);crew_setup_key(&s,SETUP_ACCEPT);
 for(int i=0;i<3;i++)crew_setup_key(&s,SETUP_ERASE);
 tap_key(&s,26);tap_key(&s,31);assert(s.editing&&s.error);
 crew_setup_key(&s,SETUP_BACK);assert(!s.editing&&!strcmp(s.names[1],"Sam"));
 crew_setup_key(&s,SETUP_ACCEPT);crew_setup_key(&s,SETUP_ERASE);
 crew_setup_key(&s,SETUP_BACK);assert(!strcmp(s.names[1],"Sam"));
 crew_setup_key(&s,SETUP_ACCEPT);
 for(int i=0;i<3;i++)crew_setup_key(&s,SETUP_ERASE);
 for(int i=0;i<30;i++)tap_key(&s,0);
 assert(strlen(s.draft)==12);tap_key(&s,31);assert(strlen(s.names[1])==12);
 assert(crew_setup_touch(&s,20,182)==SETUP_CANCEL);assert(!memcmp(&g,&before,sizeof g));
 assert(crew_setup_key(&s,SETUP_ACCEPT)==SETUP_NONE);
 crew_setup_begin(&s);crew_setup_touch(&s,20,40);
 for(int i=0;i<4;i++)crew_setup_key(&s,SETUP_ERASE);
 /* Controller navigation reaches every keyboard key and wraps safely. */
 s.key=0;crew_setup_key(&s,SETUP_LEFT);assert(s.key==31);
 crew_setup_key(&s,SETUP_RIGHT);assert(s.key==0);
 crew_setup_key(&s,SETUP_UP);assert(s.key==24);
 crew_setup_key(&s,SETUP_DOWN);assert(s.key==0);
 crew_setup_key(&s,SETUP_ACCEPT);s.key=31;crew_setup_key(&s,SETUP_ACCEPT);
 assert(!strcmp(s.names[0],"A"));
 crew_setup_key(&s,SETUP_UP);assert(s.selected==3);
 assert(crew_setup_key(&s,SETUP_ACCEPT)==SETUP_LAUNCH);
 crew_setup_apply(&s,&g);assert(!strcmp(g.crew[0].name,"A"));
 memcpy(before.crew[0].name,g.crew[0].name,GAME_CREW_NAME_SIZE);
 assert(!memcmp(&before,&g,sizeof g));
 crew_setup_begin(&s);assert(crew_setup_touch(&s,20,150)==SETUP_LAUNCH);
 puts("crew setup draft names, cancel, limits, controller and launch passed");return 0;
}
