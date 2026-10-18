#include "render.h"
#include "audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {unsigned head;uint16_t pixels[256*192];unsigned tail;} a,b;
static uint16_t previous[256*192];
static void draw(Campaign*c,Ui*u){a.head=b.head=0x1234;a.tail=b.tail=0x5678;render_campaign(c,u,a.pixels,b.pixels);assert(a.head==0x1234&&b.head==0x1234&&a.tail==0x5678&&b.tail==0x5678);}
int main(void){
 Campaign c;Ui u={.selected_crew=0,.weapon=1};campaign_init_legacy(&c);audio_init();c.mode=CAM_COMBAT;
 campaign_touch(&c,&u,30,122);assert(u.crew_details);draw(&c,&u);
 campaign_touch(&c,&u,100,129);assert(u.renaming);draw(&c,&u);
 for(int i=0;i<12;i++){campaign_touch(&c,&u,120,166);}assert(!u.rename_text[0]);
 campaign_touch(&c,&u,20,184);assert(!u.renaming&&u.crew_details); /* cancel */
 campaign_touch(&c,&u,100,129);for(int i=0;i<12;i++){campaign_touch(&c,&u,120,166);}
 campaign_touch(&c,&u,200,184);assert(u.renaming&&u.feedback_error); /* empty rejected */
 campaign_touch(&c,&u,150,138); /* Z: last keyboard row, column4 */
 campaign_touch(&c,&u,20,70); /* A */
 campaign_touch(&c,&u,200,184);assert(!u.renaming&&!strcmp(c.game.crew[0].name,"ZA"));
 campaign_touch(&c,&u,25,156);assert(c.game.crew[0].station==c.game.crew[0].room);
 int station=c.game.crew[0].station;c.game.crew[0].destination=c.game.player.systems[SYS_MEDBAY].room;
 campaign_touch(&c,&u,190,156);assert(c.game.crew[0].destination==station);
 campaign_touch(&c,&u,120,184);assert(!u.crew_details);
 /* Selecting another crew changes selection first; second tap opens details. */
 campaign_touch(&c,&u,110,122);assert(u.selected_crew==1&&!u.crew_details);
 campaign_touch(&c,&u,110,122);assert(u.crew_details);draw(&c,&u);
 u.crew_details=0;c.game.crew[0].destination=c.game.player.systems[SYS_MEDBAY].room;c.game.crew[0].move_ticks=5;
 c.game.ticks=0;draw(&c,&u);memcpy(previous,b.pixels,sizeof previous);c.game.ticks=6;draw(&c,&u);assert(memcmp(previous,b.pixels,sizeof previous));
 /* Enemy occupants are visible only on the ship they currently occupy. */
 game_begin_encounter(&c.game,ENEMY_REBEL,1,0);c.game.subsystems[0][SUB_SENSORS].capacity=c.game.subsystems[0][SUB_SENSORS].health=2;u.aiming=1;u.tap_ticks=0;
 assert(c.game.enemy_crew_count>0);draw(&c,&u);memcpy(previous,b.pixels,sizeof previous);
 for(int i=0;i<c.game.enemy_crew_count;i++)c.game.enemy_crew[i].ship=0;
 draw(&c,&u);assert(memcmp(previous,b.pixels,sizeof previous));
 puts("Crew UI: retap details, keyboard cancel/empty/name, save/return stations, selection, walking frames and framebuffer bounds passed");
}
