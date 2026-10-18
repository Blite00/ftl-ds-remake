#ifndef FTL_DS_TUTORIAL_H
#define FTL_DS_TUTORIAL_H
#include <stdint.h>
#include "game.h"
#include "render.h"
/* Guided Kestrel tutorial reached from the main menu, modelled on the original
 * FTL tutorial (prompts imported from data/text_tutorial.xml by
 * tools/import_tutorial.py). It owns its own Game and Ui, never touches the
 * campaign or its save, and can be skipped at any time (SELECT or SKIP).
 * Logic (this file and tutorial.c) is independent of rendering; the view lives
 * in tutorial_view.inc, included at the end of render.c.
 *
 * Bottom-screen layout is the normal combat view (render_game) so the controls
 * learnt here are the real ones; the bottom row's DRONES slot becomes SKIP
 * (x136-186, y176-191) and read-only steps show NEXT (x2-134) there. */
enum TutorialAction { TUTORIAL_NONE, TUTORIAL_EXIT };
enum TutorialKey { TUT_UP, TUT_DOWN, TUT_LEFT, TUT_RIGHT, TUT_A, TUT_B, TUT_X, TUT_Y, TUT_L, TUT_R, TUT_START, TUT_SELECT, TUT_KEY_COUNT };
/* Key modes outside aiming: START cycles CREW -> POWER -> DOORS. */
enum TutorialMode { TUT_MODE_CREW, TUT_MODE_POWER, TUT_MODE_DOORS };
/* READ: NEXT/A continues. ACTION: advances when the player does the thing. */
enum TutorialKind { TUT_READ, TUT_ACTION };
enum TutorialStep {
    TS_INTRO_SHIP, TS_INTRO_SYSTEMS, TS_INTRO_SUBSYSTEMS, TS_INTRO_HULL,
    TS_POWER_INFO, TS_POWER_ENGINES,
    TS_DAMAGE, TS_SELECT_CREW, TS_SEND_CREW, TS_REPAIR, TS_REPOWER, TS_MANNING,
    TS_FIRE_AIRLOCK, TS_FIRE_OUT,
    TS_PILOT, TS_JUMP, TS_STAR_MAP,
    TS_PAUSE, TS_SHIELDS, TS_WEAPON_POWER, TS_TARGET, TS_SHIELDS_BLOCK, TS_MISSILE, TS_ENEMY_SYSTEMS, TS_DEFEAT,
    TS_DONE, TS_COUNT };
#define TUTORIAL_BEACONS 7
#define TUTORIAL_START_FUEL 16
/* Instruction panel prompt size; tutorial.c checks the importer used the same. */
#define TUTORIAL_PROMPT_COLS 39
#define TUTORIAL_PROMPT_LINES 6
typedef struct {
    int active,done,skipped,step,mode,sys_sel,door_sel,map_sel,beacon,fuel,fire_room,combat;
    int armed[GAME_MAX_WEAPONS],laser_fired,pilot_needed,ticks,step_ticks,note_ticks;
    char note[48];
    Ui ui;
    Game game;
} Tutorial;
void tutorial_begin(Tutorial *tutorial);
/* Both return TUTORIAL_EXIT when the player skips or finishes (active becomes 0). */
int tutorial_key(Tutorial *tutorial,int key);
int tutorial_touch(Tutorial *tutorial,int x,int y);
/* Call once per 30 Hz step; simulates only on steps that allow it. */
void tutorial_tick(Tutorial *tutorial);
/* Queries for the view and tests. */
int tutorial_step_count(void);
int tutorial_kind(const Tutorial *tutorial);
int tutorial_step_complete(const Tutorial *tutorial);
const char *tutorial_title(const Tutorial *tutorial);
const char *tutorial_text(const Tutorial *tutorial);
const char *tutorial_text_id(int step);
void tutorial_goal(const Tutorial *tutorial,char *out,int size);
int tutorial_can_jump(const Tutorial *tutorial);
/* Beacon map: screen position and whether it links to the current beacon. */
int tutorial_beacon(const Tutorial *tutorial,int index,int *x,int *y);
int tutorial_beacon_linked(int a,int b);
/* Bottom-screen rectangle of a room (side 0 player view, 1 enemy aiming view)
 * using the same geometry as render.c; 0 if the room does not exist. */
int tutorial_room_rect(const Tutorial *tutorial,int side,int room,int *x,int *y,int *w,int *h);
int tutorial_door_point(const Tutorial *tutorial,int door,int *x,int *y);
void render_tutorial(const Tutorial *tutorial,uint16_t *top,uint16_t *bottom);
#endif
