#ifndef FTL_DS_SETUP_H
#define FTL_DS_SETUP_H
#include "game.h"
#include <stdint.h>
typedef struct {
 int active,selected,editing,key,error;
 int layout,count; /* V57: chosen layout and its starting crew (2-4). */
 char names[GAME_START_CREW_MAX][GAME_CREW_NAME_SIZE];
 char draft[GAME_CREW_NAME_SIZE];
} CrewSetup;
enum SetupAction { SETUP_NONE, SETUP_LAUNCH, SETUP_CANCEL };
enum SetupKey { SETUP_UP, SETUP_DOWN, SETUP_LEFT, SETUP_RIGHT, SETUP_ACCEPT, SETUP_BACK, SETUP_ERASE };
void crew_setup_begin(CrewSetup*);
void crew_setup_begin_ship(CrewSetup*,int layout,int crew_count);
/* Row geometry shared by drawing and touch (3 crew keep the V56 layout). */
int crew_setup_row_y(const CrewSetup*,int index);
int crew_setup_row_h(const CrewSetup*);
int crew_setup_key(CrewSetup*,int key);
int crew_setup_touch(CrewSetup*,int x,int y);
/* Apply only to the newly initialized campaign after SETUP_LAUNCH. */
void crew_setup_apply(const CrewSetup*,Game*);
void render_crew_setup(const CrewSetup*,uint16_t*top,uint16_t*bottom);
#endif
