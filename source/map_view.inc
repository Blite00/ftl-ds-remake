/* Presentation-only camera: no graph, pursuit, RNG or save state changes.
   FTL-inspired connected beacon diamonds, ship marker and pursuit front. */
/* Cache is render scratch only. Seeded relocation keeps the original graph's
   broad left-to-right order while breaking repeated rows and aligned columns. */
static struct {unsigned seed;int sector,count,valid;int x[CAM_NODE_COUNT],y[CAM_NODE_COUNT],original_x[CAM_NODE_COUNT],original_y[CAM_NODE_COUNT];} map_layout_cache;
static unsigned map_hash(unsigned v){v^=v>>16;v*=0x7feb352du;v^=v>>15;v*=0x846ca68bu;return v^(v>>16);}
static void map_layout(const Campaign*c){
 int same=map_layout_cache.valid&&map_layout_cache.seed==c->seed&&map_layout_cache.sector==c->sector&&map_layout_cache.count==c->node_count;
 if(same)for(int i=0;i<c->node_count;i++)if(map_layout_cache.original_x[i]!=campaign_node_x(c,i)||map_layout_cache.original_y[i]!=campaign_node_y(c,i)){same=0;break;}
 if(same)return;
 map_layout_cache.seed=c->seed;map_layout_cache.sector=c->sector;map_layout_cache.count=c->node_count;map_layout_cache.valid=1;
 for(int i=0;i<c->node_count;i++){map_layout_cache.x[i]=map_layout_cache.original_x[i]=campaign_node_x(c,i);map_layout_cache.y[i]=map_layout_cache.original_y[i]=campaign_node_y(c,i);}
 for(int pass=0;pass<4;pass++)for(int i=0;i<c->node_count;i++)for(int attempt=0;attempt<12;attempt++){
  unsigned h=map_hash(c->seed^(unsigned)(c->sector+1)*0x9e3779b9u^(unsigned)(i+1)*0x85ebca6bu^(unsigned)(pass*12+attempt+1)*0xc2b2ae35u);
  int x=map_layout_cache.original_x[i]+(int)(h%25)-12,y=46+(int)((h>>8)%87),clear=1;
  if(x<16)x=16;
  if(x>238)x=238;
  for(int j=0;j<c->node_count;j++)if(j!=i){int dx=x-map_layout_cache.x[j],dy=y-map_layout_cache.y[j];if(dx*dx+dy*dy<26*26){clear=0;break;}}
  if(clear){map_layout_cache.x[i]=x;map_layout_cache.y[i]=y;break;}
 }
}
static int map_center(const Campaign*c){
 map_layout(c);int n=c->selected>=0&&c->selected<c->node_count?c->selected:c->current;
 int center=map_layout_cache.x[n];if(center<64)center=64;if(center>192)center=192;return center;
}
static int map_x(const Campaign*c,const Ui*u,int x){return u->map_overview?x:128+2*(x-map_center(c));}
int ui_map_node_position(const Campaign*c,const Ui*u,int n,int*x,int*y){
 if(n<0||n>=c->node_count)return 0;
 map_layout(c);*x=map_x(c,u,map_layout_cache.x[n]);*y=map_layout_cache.y[n];
 return *x>=10&&*x<=246&&*y>=43&&*y<=134;
}
/* V41 final sector: rebel-held beacons replace the pursuit front. */
static int map_taken(const Campaign*c,int i,int next){
 if(campaign_finale_active(c))return campaign_node_rebel(c,i)||(next&&i>=0&&i<c->node_count&&((c->fleet_pending>>i)&1u));
 return campaign_node_captured(c,i,next);
}
/* V50 original-style star map: imported zone backgrounds, beacon/ship/boss/
 * hazard icons, red rebel-control hatching and labelled beacon boxes. Drawing
 * only: layout, links, RNG and saves are untouched. */
#define MAP_INK RGB(222,238,226)
#define MAP_FRAME RGB(196,222,214)
#define MAP_LINE RGB(150,170,168)
#define MAP_PURPLE RGB(176,120,236)
typedef struct {int x0,y0,x1,y1;} MapClip;
static void map_px(uint16_t*p,const MapClip*k,int x,int y,int c){if(x>=k->x0&&x<k->x1&&y>=k->y0&&y<k->y1)p[y*256+x]=(uint16_t)c;}
static void map_art(uint16_t*p,const MapClip*k,int cx,int cy,const uint16_t*c,const uint8_t*a,int w,int h,int fade){
 int x0=cx-w/2,y0=cy-h/2;
 for(int j=0;j<h;j++)for(int i=0;i<w;i++){int v=a[j*w+i]*fade/16,x=x0+i,y=y0+j;if(!v||x<k->x0||x>=k->x1||y<k->y0||y>=k->y1)continue;
  p[y*256+x]=(uint16_t)(v>=15?c[j*w+i]:mix(p[y*256+x],c[j*w+i],v,15));}
}
#define MAP_ART(p,k,x,y,n,f) map_art(p,k,x,y,n,n##_alpha,n##_WIDTH,n##_HEIGHT,f)
static int map_zone(const Campaign*c){return (int)((c->seed+(unsigned)c->sector*7u)%MAP_ZONE_COUNT);}
static void map_background(uint16_t*p,const Campaign*c,const MapClip*k,int nebula){
 const uint16_t*z=map_zones[map_zone(c)];
 for(int y=k->y0;y<k->y1;y++){int sy=(y-k->y0)*MAP_ZONE_HEIGHT/(k->y1-k->y0);for(int x=k->x0;x<k->x1;x++)p[y*256+x]=z[sy*MAP_ZONE_WIDTH+x*MAP_ZONE_WIDTH/256];}
 /* Uncharted (nebula) sectors carry the original purple nebula clouds. */
 if(nebula)for(int i=0;i<4;i++){unsigned h=map_hash(c->seed^(unsigned)(c->sector*31+i+1));int x=k->x0+(int)(h%(unsigned)(k->x1-k->x0)),y=k->y0+(int)((h>>12)%(unsigned)(k->y1-k->y0));
  if(i&1)MAP_ART(p,k,x,y,map_nebula_b,9);else MAP_ART(p,k,x,y,map_nebula,9);}
}
static void map_frame(uint16_t*p,int x,int y,int w,int h,const char*title){
 box(p,x,y,w,h,MAP_FRAME);box(p,x+1,y+1,w-2,h-2,RGB(70,92,90));
 for(int i=0;i<6;i++)rect(p,x+w-7+i,y+h-1-i,1,1,BG); /* Original chamfered corner. */
 line(p,x+w-7,y+h-1,x+w-1,y+h-7,MAP_FRAME);
 if(title){int tw=(int)strlen(title)*6+10;rect(p,x,y-1,tw,10,MAP_FRAME);rect(p,x+tw,y-1,4,1,MAP_FRAME);text(p,x+4,y+1,title,RGB(30,44,46));}
}
static void map_dots(uint16_t*p,const MapClip*k,int x0,int y0,int x1,int y1,int color){
 int dx=x1-x0,dy=y1-y0,n=dx<0?-dx:dx,a=dy<0?-dy:dy;if(a>n)n=a;if(!n)return;
 for(int i=0;i<=n;i++)if(i%6<3){int x=x0+dx*i/n,y=y0+dy*i/n;map_px(p,k,x,y,color);}
}
/* Rebel fleet control: the original's red striped overlay, as a hatched disc. */
static void map_rebel_zone(uint16_t*p,const MapClip*k,int cx,int cy,int r,int pending,int tick){
 for(int y=cy-r;y<=cy+r;y++)for(int x=cx-r;x<=cx+r;x++){int dx=x-cx,dy=y-cy,d=dx*dx+dy*dy;if(d>r*r||x<k->x0||x>=k->x1||y<k->y0||y>=k->y1)continue;
  if(pending){if(d>=(r-1)*(r-1)&&((x+y+tick/4)%6)<3)p[y*256+x]=RED;continue;}
  p[y*256+x]=(uint16_t)mix(p[y*256+x],RGB(170,30,30),((x+y)%6)<3?9:5,16);
  if(d>=(r-1)*(r-1))p[y*256+x]=RGB(225,90,80);}
}
static void map_brackets(uint16_t*p,const MapClip*k,int cx,int cy,int r,int color){
 for(int i=0;i<4;i++){map_px(p,k,cx-r+i,cy-r,color);map_px(p,k,cx-r,cy-r+i,color);map_px(p,k,cx+r-i,cy-r,color);map_px(p,k,cx+r,cy-r+i,color);
  map_px(p,k,cx-r+i,cy+r,color);map_px(p,k,cx-r,cy+r-i,color);map_px(p,k,cx+r-i,cy+r,color);map_px(p,k,cx+r,cy+r-i,color);}
}
static void map_tag(uint16_t*p,const MapClip*k,int cx,int y,const char*s,int color){
 int w=(int)strlen(s)*6+3,x=cx-w/2;if(x<k->x0||x+w>k->x1||y<k->y0||y+9>k->y1)return;
 rect(p,x,y,w,9,RGB(16,24,26));box(p,x,y,w,9,color);text(p,x+2,y+1,s,color);
}
static const char*map_label(const Campaign*c,int i){
 if(i==campaign_boss_node(c))return "FLAGSHIP";
 switch(c->node_kind[i]){case NODE_STORE:return "STORE";case NODE_EXIT:return campaign_finale_active(c)?"BASE":"EXIT";case NODE_QUEST:return "QUEST";case NODE_DISTRESS:return "DISTRESS";default:return 0;}
}
static int map_label_color(const Campaign*c,int i){int k=c->node_kind[i];return k==NODE_EXIT?GREEN:k==NODE_QUEST?MAP_PURPLE:k==NODE_DISTRESS?RED:i==campaign_boss_node(c)?RED:MAP_INK;}
static void map_beacon_draw(uint16_t*p,const Campaign*c,const MapClip*k,int i,int x,int y,int small,int tick){
 if(i==c->current){if(small)MAP_ART(p,k,x,y,map_ship_small,16);else MAP_ART(p,k,x,y,map_ship,16);return;}
 if(map_taken(c,i,0))MAP_ART(p,k,x,y,map_rebel,16);
 else if(c->node_kind[i]==NODE_HAZARD)MAP_ART(p,k,x,y,map_hazard,16);
 else if(c->node_kind[i]==NODE_DISTRESS&&!c->visited[i])MAP_ART(p,k,x,y,map_distress,16);
 else if(c->visited[i]){if(small)MAP_ART(p,k,x,y,map_visited_small,16);else MAP_ART(p,k,x,y,map_visited,16);}
 else {if(small)MAP_ART(p,k,x,y,map_beacon_small,16);else MAP_ART(p,k,x,y,map_beacon,16);}
 if(i==campaign_boss_node(c)){if(small)MAP_ART(p,k,x,y-9,map_boss_small,16);else MAP_ART(p,k,x,y-12,map_boss,16);}
 (void)tick;
}
static void sector_map_view(const Campaign*c,const Ui*u,uint16_t*top,uint16_t*bottom){
 char s[96];map_layout(c);int nebula=c->sector_types[c->sector]==SECTOR_UNCHARTED;
 /* Top screen: the whole sector, as the original's zoomed-out beacon map. */
 rect(top,0,51,256,141,BG);
 MapClip tk={4,62,252,158};map_background(top,c,&tk,nebula);map_frame(top,3,61,250,98,"SECTOR OVERVIEW");
 for(int i=0;i<c->node_count;i++)if(map_taken(c,i,1))map_rebel_zone(top,&tk,map_layout_cache.x[i],map_layout_cache.y[i]+22,8,!map_taken(c,i,0),c->game.ticks);
 for(int a=0;a<c->node_count;a++)for(int b=a+1;b<c->node_count;b++)if(campaign_linked(c,a,b))map_dots(top,&tk,map_layout_cache.x[a],map_layout_cache.y[a]+22,map_layout_cache.x[b],map_layout_cache.y[b]+22,MAP_LINE);
 for(int i=0;i<c->node_count;i++){int x=map_layout_cache.x[i],y=map_layout_cache.y[i]+22;map_beacon_draw(top,c,&tk,i,x,y,1,c->game.ticks);if(i==c->selected)map_brackets(top,&tk,x,y,6,GOLD);}
 if(!u->map_overview){int x=map_center(c)-59;box(top,x,64,119,94,CYAN);}
 if(campaign_finale_active(c)){int turns=campaign_base_turns_left(c);
  if(turns>0)snprintf(s,sizeof s,"BASE UNDER ATTACK: %d JUMP%s LEFT",turns,turns==1?"":"S");
  else snprintf(s,sizeof s,"FLAGSHIP PHASE %d/3  %s",(c->boss_resume_phase<0?0:c->boss_resume_phase)+1,campaign_boss_moves_next(c)?"JUMPS NEXT TURN":"HOLDING");
  text(top,6,163,s,turns>0||campaign_boss_moves_next(c)?RED:GOLD);text(top,6,177,"RED: REBELS / SHIP: FLAGSHIP / BASE",MUTED);}
 else{
  snprintf(s,sizeof s,"HULL %d/%d  CREW %d  FLEET +%d",c->game.player.hull,c->game.player.max_hull,game_alive_crew(&c->game),c->sector_jumps);text(top,6,163,s,c->game.player.hull<10?RED:MAP_INK);
  text(top,6,177,"RED: REBEL CONTROL / DASHED: NEXT TURN",MUTED);}
 /* Bottom screen: the touch map in the original beacon-map frame. */
 MapClip bk={3,34,253,143};map_background(bottom,c,&bk,nebula);
 rect(bottom,0,0,256,32,PANEL);rect(bottom,0,31,256,1,MAP_FRAME);
 snprintf(s,sizeof s,"SECTOR %d",c->sector+1);text(bottom,6,4,s,MAP_INK);
 {int type=c->sector_types[c->sector];const char*name=campaign_sector_name(type);
  text(bottom,6,17,name,type==SECTOR_SECRET?CYAN:type==SECTOR_UNCHARTED?MAP_PURPLE:(type==SECTOR_REBEL||type==SECTOR_PIRATE||type==SECTOR_FINAL)?RED:GREEN);}
 text(bottom,66,4,"CENTER/B",CYAN);
 button(bottom,132,2,60,27,u->map_overview?"ZOOM UP":"ALL UP",CYAN);button(bottom,196,2,57,27,"WAIT DN",GOLD);
 map_frame(bottom,2,33,252,111,0);
 int small=u->map_overview,zone=small?9:13;
 for(int i=0;i<c->node_count;i++){int x,y;ui_map_node_position(c,u,i,&x,&y);if(map_taken(c,i,1))map_rebel_zone(bottom,&bk,x,y,zone,!map_taken(c,i,0),c->game.ticks);}
 /* Dim all routes, then highlight legal outgoing jumps (the authored graph's reach). */
 for(int a=0;a<c->node_count;a++)for(int b=a+1;b<c->node_count;b++)if(campaign_linked(c,a,b)){int ax,ay,bx,by;ui_map_node_position(c,u,a,&ax,&ay);ui_map_node_position(c,u,b,&bx,&by);
  int out=a==c->current||b==c->current;map_dots(bottom,&bk,ax,ay,bx,by,out?GREEN:MAP_LINE);}
 for(int i=0;i<c->node_count;i++){
  int x,y;ui_map_node_position(c,u,i,&x,&y);
  if(i!=c->current&&campaign_can_jump(c,i))map_brackets(bottom,&bk,x,y,small?5:7,GREEN);
  map_beacon_draw(bottom,c,&bk,i,x,y,small,c->game.ticks);
  if(i==c->selected)map_brackets(bottom,&bk,x,y,small?7:10,GOLD);
  const char*tag=map_label(c,i);
  if(tag&&i!=c->current){if(small){char letter[2]={tag[0],0};map_tag(bottom,&bk,x,y+6,letter,map_label_color(c,i));}else map_tag(bottom,&bk,x,y+8,tag,map_label_color(c,i));}
 }
 if(!u->map_overview){if(map_center(c)>64)text(bottom,5,134,"< L",CYAN);if(map_center(c)<192)text(bottom,232,134,"R >",CYAN);}
 panel(bottom,4,146,248,25,EDGE);
 snprintf(s,sizeof s,"%02d %s / %s",c->selected+1,campaign_node_name(c,c->selected),c->visited[c->selected]?"VISITED":"NEW");text(bottom,10,149,s,GOLD);
 text(bottom,10,160,c->selected==campaign_boss_node(c)?"REBEL FLAGSHIP: JUMP HERE TO ENGAGE":map_taken(c,c->selected,0)?"REBEL CONTROL: EXPECT A PATROL":map_taken(c,c->selected,1)?(campaign_finale_active(c)?"REBELS TAKE THIS BEACON NEXT TURN":"FLEET ARRIVES NEXT TURN"):c->selected==c->current?"CURRENT LOCATION / L-R SELECT":campaign_can_jump(c,c->selected)?"IN JUMP RANGE / A TO TRAVEL":"OUT OF RANGE / L-R SELECT",map_taken(c,c->selected,1)||c->selected==campaign_boss_node(c)?RED:MUTED);
 button(bottom,3,174,61,18,"SAVE X",CYAN);button(bottom,67,174,61,18,"SHIP Y",CYAN);button(bottom,132,174,120,18,c->fuel?"JUMP A":"RESCUE",c->fuel?(campaign_can_jump(c,c->selected)?GREEN:MUTED):GOLD);
}
/* V50 next-sector chooser: the original sector map (civilian green, hostile red,
 * nebula purple) with the route taken so far and the two offered sectors. */
static int sector_color(int type){return type==SECTOR_SECRET?CYAN:type==SECTOR_UNCHARTED?MAP_PURPLE:(type==SECTOR_REBEL||type==SECTOR_PIRATE||type==SECTOR_FINAL)?RED:GREEN;}
static void sector_dot(uint16_t*p,const MapClip*k,int x,int y,int type){
 if(type==SECTOR_UNCHARTED)MAP_ART(p,k,x,y,map_nebula_dot,16);else if(sector_color(type)==RED)MAP_ART(p,k,x,y,map_hostile,16);else MAP_ART(p,k,x,y,map_civilian,16);
}
static void sector_choice_view(const Campaign*c,const Ui*u,uint16_t*top){
 MapClip k={6,62,250,186};rect(top,4,52,248,136,BG);
 for(int y=k.y0;y<k.y1;y++)for(int x=k.x0;x<k.x1;x++)top[y*256+x]=(uint16_t)(((x*7+y*3)%23)?RGB(36,16,18):RGB(52,24,26));
 map_frame(top,5,61,246,126,"SECTOR MAP");
 int n=c->sector_count>1?c->sector_count:2,routes=campaign_route_count(c),selected=u->event_selected>=0&&u->event_selected<routes?u->event_selected:0;
 int px=0,py=124;
 for(int s=0;s<n;s++){int x=18+s*(220/(n-1)),y=124;
  if(s<=c->sector){if(s)map_dots(top,&k,px,py,x,y,MAP_INK);sector_dot(top,&k,x,y,c->sector_types[s]);if(s==c->sector)MAP_ART(top,&k,x,y-11,map_ship_small,16);px=x;py=y;}
  else if(s==c->sector+1){for(int r=0;r<routes;r++){int ry=routes==1?124:r?150:98;map_dots(top,&k,px,py,x,ry,r==selected?GOLD:MAP_LINE);sector_dot(top,&k,x,ry,c->next_sector[r]);
     if(r==selected)map_brackets(top,&k,x,ry,7,GOLD);
     const char*name=campaign_sector_name(c->next_sector[r]);int w=(int)strlen(name)*6;int tx=x+10;if(tx+w>248)tx=x-10-w;text(top,tx,ry-3,name,r==selected?GOLD:sector_color(c->next_sector[r]));}}
  else {map_px(top,&k,x,124,MUTED);map_px(top,&k,x-1,124,MUTED);map_px(top,&k,x+1,124,MUTED);map_px(top,&k,x,123,MUTED);map_px(top,&k,x,125,MUTED);}
 }
 text(top,124,66,"CHOOSE NEXT SECTOR",MAP_INK);
 {char s[48];snprintf(s,sizeof s,"SECTOR %d OF %d",c->sector+1,c->sector_count);text(top,150,176,s,MUTED);}
 MAP_ART(top,&k,14,177,map_civilian,16);text(top,20,174,"SAFE",GREEN);MAP_ART(top,&k,54,177,map_hostile,16);text(top,60,174,"HOSTILE",RED);MAP_ART(top,&k,108,177,map_nebula_dot,16);text(top,114,174,"NEB",MAP_PURPLE);
}
