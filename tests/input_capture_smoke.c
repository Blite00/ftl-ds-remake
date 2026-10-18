#include <assert.h>
#include <stdio.h>
#include "input_capture.h"
int main(void){
 InputCapture q={0};CapturedInput e;
 /* Sampling continues while renderer blocks the consumer. One-sample releases
    and represses are retained, with the coordinates at the original contact. */
 for(int i=0;i<8;i++){input_capture_sample(&q,1,20+i,30+i);input_capture_sample(&q,0,0,0);}
 for(int i=0;i<8;i++){assert(input_capture_take(&q,&e));assert(e.keys==1&&e.x==20+i&&e.y==30+i);}
 assert(!input_capture_take(&q,&e));
 input_capture_sample(&q,1,80,90);
 for(int i=0;i<20;i++)input_capture_sample(&q,1,180,190);
 assert(input_capture_take(&q,&e)&&e.x==80&&e.y==90);assert(!input_capture_take(&q,&e));
 /* A queued press cannot confirm a newly presented modal. A held contact
    remains consumed through its transition; release then press is immediate. */
 input_capture_sample(&q,0,0,0);input_capture_sample(&q,1,30,40);
 input_capture_block(&q);input_capture_sample(&q,0,0,0);input_capture_sample(&q,1,30,40);
 input_capture_resume(&q);input_capture_sample(&q,1,30,40);assert(!input_capture_take(&q,&e));
 input_capture_sample(&q,0,0,0);input_capture_sample(&q,1,31,41);assert(input_capture_take(&q,&e)&&e.x==31);
 input_capture_sample(&q,0,0,0);
 for(int i=0;i<10;i++){input_capture_sample(&q,2,0,0);input_capture_sample(&q,0,0,0);}
 assert(q.count==8&&q.dropped==2);
 while(input_capture_take(&q,&e))assert(e.keys==2);
 input_capture_sample(&q,4,0,0);for(unsigned i=0;i<=INPUT_CAPTURE_MAX_AGE;i++)input_capture_sample(&q,0,0,0);
 assert(!input_capture_take(&q,&e));
 q.sample=~0u;input_capture_sample(&q,8,0,0);assert(input_capture_take(&q,&e)&&e.keys==8);
 puts("Input capture: fast distinct contacts, fixed press coordinates, hold suppression, modal barrier, overflow, stale input and clock wrap passed");return 0;
}
