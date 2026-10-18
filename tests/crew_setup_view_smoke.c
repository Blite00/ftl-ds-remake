#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
static uint16_t top[256*192+2],bottom[256*192+2];
static void preview(const char*path){
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 384\n255\n");
 for(int y=0;y<384;y++)for(int x=0;x<256;x++){
  unsigned p=y<192?top[1+y*256+x]:bottom[1+(y-192)*256+x];
  unsigned char rgb[3]={(p&31)*255/31,((p>>5)&31)*255/31,((p>>10)&31)*255/31};fwrite(rgb,1,3,f);
 }fclose(f);
}
int main(int argc,char**argv){
 CrewSetup s,before;crew_setup_begin(&s);
 top[0]=top[256*192+1]=bottom[0]=bottom[256*192+1]=0x1234;
 for(int edit=0;edit<2;edit++)for(int selected=0;selected<4;selected++)for(int key=0;key<32;key++){
  s.editing=edit;s.selected=selected;s.key=key;strcpy(s.draft,"ABCDEFGHIJKL");before=s;
  render_crew_setup(&s,top+1,bottom+1);
  assert(!memcmp(&s,&before,sizeof s));
  assert(top[0]==0x1234&&top[256*192+1]==0x1234&&bottom[0]==0x1234&&bottom[256*192+1]==0x1234);
 }
 crew_setup_begin(&s);render_crew_setup(&s,top+1,bottom+1);if(argc>1)preview(argv[1]);
 crew_setup_key(&s,SETUP_ACCEPT);render_crew_setup(&s,top+1,bottom+1);if(argc>2)preview(argv[2]);
 puts("crew setup renderer bounds and state preservation passed");return 0;
}
