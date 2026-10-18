#ifndef FTL_DS_MENU_H
#define FTL_DS_MENU_H
/* finale_original: flagship numbers for the next new run (0 DS tuning, 1 original
 * normal); kept across menu reopenings, not written to storage. */
typedef struct { int active, saved_available, live_run, selected, page, confirm, confirm_yes, finale_original; } MainMenu;
/* V64 order (as the original): Continue, New Run, Tutorial, Finale, Controls, Sound. */
#define MAIN_MENU_ITEMS 6
#define MAIN_MENU_Y 22
#define MAIN_MENU_STEP 23
enum MainMenuAction { MENU_NONE, MENU_RESUME, MENU_NEW_RUN, MENU_VOLUME, MENU_TUTORIAL };
/* Keys: 0 up, 1 down, 2 accept, 3 back. No campaign or storage writes. */
void main_menu_init(MainMenu*,int saved_available);
void main_menu_open(MainMenu*,int live_run);
int main_menu_key(MainMenu*,int key);
int main_menu_touch(MainMenu*,int x,int y);
#endif
