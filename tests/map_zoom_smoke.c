#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Campaign c,before;static Ui u;
static struct{unsigned pre;uint16_t p[256*192];unsigned post;} top,bottom;
static void draw(void){before=c;top.pre=bottom.pre=17;top.post=bottom.post=31;render_campaign(&c,&u,top.p,bottom.p);assert(!memcmp(&before,&c,sizeof c));assert(top.pre==17&&bottom.pre==17&&top.post==31&&bottom.post==31);}
int main(void){audio_init();
 for(unsigned seed=1;seed<=64;seed++){
  campaign_init_seed(&c,seed);u=(Ui){.map_overview=1};
  int positions[CAM_NODE_COUNT][2],moved=0;
  for(int i=0;i<c.node_count;i++){assert(ui_map_node_position(&c,&u,i,&positions[i][0],&positions[i][1]));if(positions[i][0]!=campaign_node_x(&c,i)&&positions[i][1]!=campaign_node_y(&c,i))moved++;}
  assert(moved>=12);
  for(int i=0;i<c.node_count;i++)for(int j=i+1;j<c.node_count;j++){int dx=positions[i][0]-positions[j][0],dy=positions[i][1]-positions[j][1];assert(dx*dx+dy*dy>=26*26);}
  /* Invalidate the scratch cache then reproduce the exact original layout. */
  c.sector++;int px,py;ui_map_node_position(&c,&u,0,&px,&py);c.sector--;
  for(int i=0;i<c.node_count;i++){assert(ui_map_node_position(&c,&u,i,&px,&py));assert(px==positions[i][0]&&py==positions[i][1]);}
  u=(Ui){0};
  for(int mode=0;mode<2;mode++){u.map_overview=mode;
   for(int n=0;n<c.node_count;n++){
    c.selected=n;int x,y;assert(ui_map_node_position(&c,&u,n,&x,&y));draw();
    campaign_touch(&c,&u,x,y);assert(c.selected==n);
    /* All visible targets use exactly the rendering transform, nearest wins. */
    for(int j=0;j<c.node_count;j++){c.selected=n;if(ui_map_node_position(&c,&u,j,&x,&y)){campaign_touch(&c,&u,x,y);assert(c.selected==j);}}
    c.selected=n;ui_map_key(&c,&u,1);assert(c.selected==(n+1)%c.node_count);draw();
   }
  }
  before=c;ui_map_key(&c,&u,3);assert(!memcmp(&c,&before,sizeof c));
  ui_map_key(&c,&u,4);assert(c.selected==c.current&&!u.map_overview);
  c.sector_jumps=30;draw();c.sector_jumps=0;draw();
 }
 campaign_init_seed(&c,42);u=(Ui){.map_overview=1};
 for(int jumps=0;jumps<=20;jumps++){
  c.sector_jumps=jumps;c.current=0;u.feedback_ticks=0;draw();
  for(int i=0;i<c.node_count;i++)if(campaign_node_captured(&c,i,1)){
   int px,py,found=0;assert(ui_map_node_position(&c,&u,i,&px,&py));
   /* V50: rebel control is the original red hatched zone (solid rim when taken, dashed rim next turn). */
   uint16_t color=campaign_node_captured(&c,i,0)?(uint16_t)(0x8000|(225>>3)|((90>>3)<<5)|((80>>3)<<10)):(uint16_t)(0x8000|(245>>3)|((108>>3)<<5)|((97>>3)<<10));
   for(int dy=-9;dy<=-8;dy++)for(int d=-4;d<=4;d++)if(bottom.p[(py+dy)*256+px+d]==color)found++;
   assert(found>0);
  }
 }
 for(unsigned seed=1;seed<=64;seed++){
  campaign_init_seed(&c,seed);c.node_count=12;c.sector_count=6;c.current=0;u=(Ui){.map_overview=1};
  for(int i=0;i<12;i++)c.node_y[i]=(i&1)?67:123;
  c.node_y[0]=c.node_y[11]=94;
  int pos[12][2];for(int i=0;i<12;i++){assert(ui_map_node_position(&c,&u,i,&pos[i][0],&pos[i][1]));campaign_touch(&c,&u,pos[i][0],pos[i][1]);assert(c.selected==i);}
  for(int i=0;i<12;i++)for(int j=i+1;j<12;j++){int dx=pos[i][0]-pos[j][0],dy=pos[i][1]-pos[j][1];assert(dx*dx+dy*dy>=26*26);}
  draw();
 }
 campaign_init_seed(&c,42);u=(Ui){0};int x,y;assert(!ui_map_node_position(&c,&u,-1,&x,&y));assert(!ui_map_node_position(&c,&u,c.node_count,&x,&y));
 int n=c.selected;campaign_touch(&c,&u,254,142);assert(c.selected==n);
 puts("Map zoom: 64 seeds, deterministic organic placement/minimum spacing, both camera modes, every visible beacon touch, all controller selections, recenter, pure rendering, pursuit extremes and framebuffer bounds passed.");return 0;
}
