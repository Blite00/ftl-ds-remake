/* Draft names and input only; campaign time and storage remain untouched. */
void render_crew_setup(const CrewSetup*s,uint16_t*top,uint16_t*bottom){
 space_background(top);space_background(bottom);
 {const uint16_t*art=player_art_preview(s->layout);char title[40];
  snprintf(title,sizeof title,"%s",game_layout_name(s->layout));text_scale(top,(256-(int)strlen(title)*12)/2,16,title,CYAN,2);
  if(art)bitmap(top,58,54,art,PLAYER_ART_HANGAR_W,PLAYER_ART_HANGAR_H);}
 {static const char*const counts[]={"ONE","TWO","THREE","FOUR"};char line[40];snprintf(line,sizeof line,"NAME YOUR %s CREW MEMBERS",counts[s->count-1]);text(top,(256-(int)strlen(line)*6)/2,153,line,WHITE);}
 text(top,29,171,"OR KEEP THE DEFAULT NAMES",MUTED);
 if(s->editing){
  text(bottom,8,7,"EDIT CREW NAME",CYAN);
  panel(bottom,8,22,240,29,CYAN);text_scale(bottom,16,29,s->draft,WHITE,2);
  for(int i=0;i<32;i++){
   char letter[2]={(char)('A'+i),0};const char*label=letter;
   if(i==26)label="SPC";else if(i==27)label="-";else if(i==28)label="'";
   else if(i==29)label="DEL";else if(i==30)label="BACK";else if(i==31)label="OK";
   button(bottom,(i%8)*32,58+(i/8)*30,31,28,label,s->key==i?CYAN:MUTED);
  }
  text(bottom,5,183,s->error?"NAME CANNOT BE EMPTY":"A TYPE  X ERASE  B CANCEL",s->error?RED:MUTED);
 }else{
  text(bottom,12,12,"TAP A NAME TO CHANGE IT",CYAN);
  for(int i=0;i<s->count;i++){
   char line[32];int y=crew_setup_row_y(s,i),h=crew_setup_row_h(s);snprintf(line,sizeof line,"%d  %s",i+1,s->names[i]);
   panel(bottom,12,y,232,h,s->selected==i?CYAN:MUTED);
   text_scale(bottom,24,y+(h-14)/2,line,WHITE,2);
  }
  button(bottom,12,140,232,28,"LAUNCH",s->selected==s->count?CYAN:GOLD);
  text(bottom,18,179,"B / TAP HERE: BACK TO MENU",MUTED);
 }
}
