#include "input_capture.h"
#include <string.h>
void input_capture_sample(volatile InputCapture*q,unsigned held,int x,int y){
 unsigned down=held&~q->held;q->held=held;q->sample++;
 if(!down||q->blocked)return;
 if(q->count==INPUT_CAPTURE_CAPACITY){q->dropped++;return;}
 unsigned slot=(q->first+q->count)%INPUT_CAPTURE_CAPACITY;
 q->events[slot]=(CapturedInput){down,q->sample,x,y};q->count++;
}
int input_capture_take(volatile InputCapture*q,CapturedInput*e){
 while(q->count){
  *e=q->events[q->first];q->first=(q->first+1)%INPUT_CAPTURE_CAPACITY;q->count--;
  if(q->sample-e->sample<=INPUT_CAPTURE_MAX_AGE)return 1;
  q->dropped++;
 }
 *e=(CapturedInput){0,0,-1,-1};return 0;
}
void input_capture_block(volatile InputCapture*q){q->blocked=1;q->count=0;q->first=0;}
void input_capture_resume(volatile InputCapture*q){q->blocked=0;}
#ifdef ARM9
#include <nds.h>
static volatile InputCapture capture;
/* Exact bounded context bytes supplied by main; no hashing collisions. */
static unsigned char view[1024];
static unsigned view_size;
static int view_valid;
static void capture_vblank(void){
 touchPosition touch={0};scanKeys();unsigned held=keysHeld();
 if(held&KEY_TOUCH)touchRead(&touch);
 input_capture_sample(&capture,held,touch.px,touch.py);
}
void ds_input_init(void){
 capture=(InputCapture){0};view_size=0;view_valid=0;
 irqSet(IRQ_VBLANK,capture_vblank);irqEnable(IRQ_VBLANK);
}
uint32_t ds_input_vblank_count(void){return capture.sample;}
int ds_input_take(CapturedInput*e){
 int previous=enterCriticalSection();int ok=input_capture_take(&capture,e);
 leaveCriticalSection(previous);return ok;
}
void ds_input_view_begin(const void*context,unsigned size){
 if(size>sizeof view){view_valid=0;int previous=enterCriticalSection();input_capture_block(&capture);leaveCriticalSection(previous);return;}
 view_valid=1;
 if(size!=view_size||memcmp(view,context,size)){
  int previous=enterCriticalSection();input_capture_block(&capture);leaveCriticalSection(previous);
  memcpy(view,context,size);view_size=size;
 }
}
void ds_input_view_end(void){if(view_valid){int previous=enterCriticalSection();input_capture_resume(&capture);leaveCriticalSection(previous);}}
#else
void ds_input_init(void){}
uint32_t ds_input_vblank_count(void){return 0;}
int ds_input_take(CapturedInput*e){*e=(CapturedInput){0,0,-1,-1};return 0;}
void ds_input_view_begin(const void*context,unsigned size){(void)context;(void)size;}
void ds_input_view_end(void){}
#endif
