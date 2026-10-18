#include "hangar.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
void hangar_open(Hangar*h,const Profile*p){memset(h,0,sizeof *h);h->active=1;h->shown=-1;if(p)h->profile=*p;else profile_defaults(&h->profile);}
int hangar_layout(const Hangar*h){return h->cls*2+h->variant;}
int hangar_unlocked(const Hangar*h,int layout){return profile_layout_unlocked(&h->profile,layout);}
int hangar_can_start(const Hangar*h){int k=hangar_layout(h);return hangar_unlocked(h,k)&&game_layout_supported(k);}
static void select_class(Hangar*h,int cls){h->cls=(cls+PROFILE_CLASSES)%PROFILE_CLASSES;h->note[0]=0;}
static int start(Hangar*h){
 if(!hangar_unlocked(h,hangar_layout(h))){snprintf(h->note,sizeof h->note,"Locked. See the unlock condition.");return HANGAR_NONE;}
 if(!game_layout_supported(hangar_layout(h))){snprintf(h->note,sizeof h->note,"Federation artillery arrives in a later build.");return HANGAR_NONE;}
 h->active=0;return HANGAR_START;}
int hangar_key(Hangar*h,int key){
 if(!h->active)return HANGAR_NONE;
 switch(key){
 case HANGAR_LEFT:select_class(h,h->cls%3?h->cls-1:h->cls+2);break;
 case HANGAR_RIGHT:select_class(h,h->cls%3==2?h->cls-2:h->cls+1);break;
 case HANGAR_UP:select_class(h,h->cls-3);break;
 case HANGAR_DOWN:select_class(h,h->cls+3);break;
 case HANGAR_TOGGLE:h->variant=!h->variant;h->note[0]=0;break;
 case HANGAR_A:return start(h);
 case HANGAR_B:h->active=0;return HANGAR_BACK;
 default:break;}
 return HANGAR_NONE;}
/* Bottom screen: class grid y24-96, Type A/B y102-121, info y126-164, BACK/START y170-189. */
int hangar_touch(Hangar*h,int x,int y){
 if(!h->active||x<0||x>=256||y<0||y>=192)return HANGAR_NONE;
 if(y>=24&&y<97){int col=(x-4)/84,row=(y-24)/25;if(col>=0&&col<3&&row>=0&&row<3&&(x-4)%84<82&&(y-24)%25<22)select_class(h,row*3+col);return HANGAR_NONE;}
 if(y>=102&&y<122){if(x>=4&&x<128){h->variant=0;h->note[0]=0;}else if(x>=132&&x<252){h->variant=1;h->note[0]=0;}return HANGAR_NONE;}
 if(y>=170&&y<190){if(x>=4&&x<128)return hangar_key(h,HANGAR_B);if(x>=132&&x<252)return hangar_key(h,HANGAR_A);}
 return HANGAR_NONE;}
