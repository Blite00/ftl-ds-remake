#ifndef FTL_DS_PLAYER_ART_H
#define FTL_DS_PLAYER_ART_H
#include <stdint.h>
/* V57 per-layout player ship art, loaded from NitroFS (ships/<blueprint>.bin,
 * written by tools/import_player_ships.py) so only the chosen layout is in RAM.
 * Kestrel A falls back to the built-in arrays when the file is unavailable;
 * other layouts then draw rooms without hull art. */
#define PLAYER_ART_TACTICAL_W 256
#define PLAYER_ART_TACTICAL_H 88
#define PLAYER_ART_COMBAT_W 110
#define PLAYER_ART_COMBAT_H 70
#define PLAYER_ART_HANGAR_W 140
#define PLAYER_ART_HANGAR_H 90
#define PLAYER_ART_MAX_INTERIORS 16
/* Make layout's art current (no-op when already loaded). Returns 1 with art. */
int player_art_use(int layout);
const uint16_t *player_art_tactical(void);
const uint16_t *player_art_combat(void);
const uint16_t *player_art_hangar(void);
/* Furnishing for a room of the current layout: pixels, size, system code. */
const uint16_t *player_art_interior(int room,int *w,int *h,int *code);
/* Hangar preview of any layout into a caller buffer (140x90). Returns 1 on success. */
int player_art_hangar_preview(int layout,uint16_t *out);
/* Shared, cached hangar preview for the menus; NULL when the layout has no art. */
const uint16_t *player_art_preview(int layout);
#endif
