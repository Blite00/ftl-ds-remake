/* V57 hangar, in the V54 original-style frames. Top: the chosen layout (art or a
 * locked silhouette) and its starting loadout. Bottom: class grid, Type A/B,
 * unlock condition and START/BACK. */
static int wrap_lines(uint16_t*p,int x,int y,int cols,int max_lines,const char*s,int c){
 char line[48];int lines=0;
 while(*s&&lines<max_lines){int n=(int)strlen(s),cut=n<=cols?n:cols;
  if(n>cols){while(cut>0&&s[cut]!=' ')cut--;if(!cut)cut=cols;}
  snprintf(line,sizeof line,"%.*s",cut,s);text(p,x,y+lines*8,line,c);lines++;s+=cut;while(*s==' ')s++;}
 return lines;}
static const char*hangar_class_label(int cls){static const char*const n[]={"KESTREL","STEALTH","MANTIS","ENGI","FEDERATION","SLUG","ROCK","ZOLTAN","CRYSTAL"};return cls>=0&&cls<9?n[cls]:"";}
void render_hangar(Hangar*h,uint16_t*top,uint16_t*bottom){
 int layout=hangar_layout(h),open=hangar_unlocked(h,layout);char s[96];GameLayoutInfo info;
 const uint16_t*art=player_art_preview(layout);int art_ok=art!=0;h->shown=layout;
 game_layout_info(layout,&info);
 space_background(top);space_background(bottom);
 /* Top: name, art or silhouette, loadout. */
 panel(top,4,4,248,30,open?FTL_MINT:EDGE);
 snprintf(s,sizeof s,"%s",open?game_layout_name(layout):"LOCKED SHIP");text_scale(top,(256-(int)strlen(s)*12)/2,9,s,open?FTL_YELLOW:MUTED,2);
 snprintf(s,sizeof s,"%s / TYPE %c",game_layout_class(layout),h->variant?'B':'A');text(top,(256-(int)strlen(s)*6)/2,25,s,WHITE);
 if(art_ok){
  if(open)bitmap(top,58,38,art,PLAYER_ART_HANGAR_W,PLAYER_ART_HANGAR_H);
  else for(int j=0;j<PLAYER_ART_HANGAR_H;j++)for(int i=0;i<PLAYER_ART_HANGAR_W;i++)if(art[j*PLAYER_ART_HANGAR_W+i])top[(38+j)*256+58+i]=(i+j)&1?RGB(24,34,44):RGB(34,46,58);
 }else text(top,92,80,"NO SHIP ART",MUTED);
 panel(top,4,132,248,56,EDGE);
 if(open){char list[96];int n=0;
  snprintf(s,sizeof s,"HULL %d  REACTOR %d  MISSILES %d  PARTS %d",info.hull,info.reactor,info.missiles,info.drone_parts);text(top,10,138,s,WHITE);
  n=snprintf(list,sizeof list,"CREW:");for(int i=0;i<info.crew_count&&n<(int)sizeof list;i++)n+=snprintf(list+n,sizeof list-(size_t)n," %s",species_name(info.crew[i]));text(top,10,148,list,CYAN);
  n=snprintf(list,sizeof list,"WEAPONS:");for(int i=0;i<info.weapon_count&&i<4&&n<(int)sizeof list;i++)n+=snprintf(list+n,sizeof list-(size_t)n,"%s %s",i?",":"",game_weapon_short(info.weapons[i]));if(!info.weapon_count)snprintf(list+n,sizeof list-(size_t)n," NONE");text(top,10,158,list,GREEN);
  n=snprintf(list,sizeof list,"%s",info.drone_count?"DRONES:":"AUGMENTS:");
  if(info.drone_count)for(int i=0;i<info.drone_count&&i<3&&n<(int)sizeof list;i++)n+=snprintf(list+n,sizeof list-(size_t)n,"%s %.14s",i?",":"",drones_name(info.drones[i]));
  else if(info.augment_count)for(int i=0;i<info.augment_count&&i<3&&n<(int)sizeof list;i++)n+=snprintf(list+n,sizeof list-(size_t)n,"%s %.20s",i?",":"",game_augment_name(info.augments[i]));
  else snprintf(list+n,sizeof list-(size_t)n," NONE");
  text(top,10,168,list,GOLD);
  if(info.drone_count&&info.augment_count){snprintf(list,sizeof list,"AUGMENT: %.30s",game_augment_name(info.augments[0]));text(top,10,178,list,GOLD);}
 }else if(h->variant){char line[64];
  snprintf(line,sizeof line,"ACHIEVEMENTS %d OF 3 (2 UNLOCK TYPE B)",profile_class_achievements(&h->profile,h->cls));text(top,10,138,line,WHITE);
  for(int k=0;k<3;k++){int id=h->cls*3+k,done=!!(h->profile.achievements[0]&(1u<<id));snprintf(line,sizeof line,"%s %.34s",done?"[X]":"[ ]",achievement_name(id));text(top,10,150+k*11,line,done?GREEN:MUTED);}
 }else wrap_lines(top,10,140,39,5,"COMPLETE THE UNLOCK CONDITION BELOW TO ADD THIS SHIP TO YOUR HANGAR.",MUTED);
 /* Bottom: class grid, type, unlock info, actions. */
 panel(bottom,0,0,256,20,FTL_MINT);text(bottom,8,7,"HANGAR / D-PAD SHIP  Y TYPE A/B",FTL_YELLOW);
 for(int k=0;k<9;k++){int any=hangar_unlocked(h,k*2)||hangar_unlocked(h,k*2+1);
  button(bottom,4+(k%3)*84,24+(k/3)*25,82,22,hangar_class_label(k),k==h->cls?GOLD:any?FTL_MINT:MUTED);}
 for(int v=0;v<2;v++){int k=h->cls*2+v;snprintf(s,sizeof s,"TYPE %c%s",v?'B':'A',hangar_unlocked(h,k)?"":" LOCKED");
  button(bottom,v?132:4,102,v?120:124,20,s,v==h->variant?GOLD:hangar_unlocked(h,k)?FTL_MINT:MUTED);}
 panel(bottom,4,126,248,40,open?FTL_MINT:EDGE);
 if(h->note[0])wrap_lines(bottom,10,131,39,3,h->note,RED);
 else if(!open){char u[200];snprintf(u,sizeof u,"UNLOCK: %s",game_layout_unlock(layout));wrap_lines(bottom,10,130,39,4,u,GOLD);}
 else if(!game_layout_supported(layout))wrap_lines(bottom,10,131,39,3,"UNLOCKED. FEDERATION ARTILLERY ARRIVES IN A LATER BUILD.",GOLD);
 else{snprintf(s,sizeof s,"READY: %s",game_layout_name(layout));wrap_lines(bottom,10,131,39,3,s,GREEN);text(bottom,10,152,"A START  B BACK  TOUCH TO CHOOSE",MUTED);}
 button(bottom,4,170,124,20,"BACK / B",FTL_MINT);button(bottom,132,170,120,20,"START / A",hangar_can_start(h)?GREEN:MUTED);
}
