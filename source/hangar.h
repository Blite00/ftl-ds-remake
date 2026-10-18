#ifndef FTL_DS_HANGAR_H
#define FTL_DS_HANGAR_H
#include <stdint.h>
#include "profile.h"
/* V57 hangar: choose a ship class (3x3 grid, original order) and its Type A/B
 * layout before a new run. Locked layouts show as silhouettes with their original
 * unlock condition. Reads unlocks from the SD profile; never touches the save. */
typedef struct { int active,cls,variant,shown; Profile profile; char note[64]; } Hangar;
enum HangarAction { HANGAR_NONE, HANGAR_START, HANGAR_BACK };
enum HangarKey { HANGAR_UP, HANGAR_DOWN, HANGAR_LEFT, HANGAR_RIGHT, HANGAR_A, HANGAR_B, HANGAR_TOGGLE };
void hangar_open(Hangar *hangar,const Profile *profile);
int hangar_layout(const Hangar *hangar);
int hangar_unlocked(const Hangar *hangar,int layout);
/* Unlocked and fully supported by this build. */
int hangar_can_start(const Hangar *hangar);
int hangar_key(Hangar *hangar,int key);
int hangar_touch(Hangar *hangar,int x,int y);
void render_hangar(Hangar *hangar,uint16_t *top,uint16_t *bottom);
#endif
