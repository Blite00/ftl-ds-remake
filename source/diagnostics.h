#ifndef FTL_DS_DIAGNOSTICS_H
#define FTL_DS_DIAGNOSTICS_H
#include "game.h"
#define DIAGNOSTICS_MAX_BYTES (256L * 1024L)
#define DIAGNOSTICS_LINES 64
#define DIAGNOSTICS_LINE_LENGTH 192
/* Frame is the 60 Hz display frame counter, so paused play still flushes.
 * NULL path: /ftl-ds.log on DS, ftl-ds.log on host. Existing logs are appended.
 * Once the 256 KiB file cap is reached, logging continues only in memory. */
void diagnostics_init(const char *path);
void diagnostics_input(unsigned frame,unsigned keys,int touch_x,int touch_y);
void diagnostics_action(unsigned frame,const char *action);
void diagnostics_poll(const Game *game,unsigned frame);
void diagnostics_flush(void);
/* Opt in to memory-only automatic polling; explicit flush still persists. */
void diagnostics_defer_automatic_flush(int defer);
const char *diagnostics_status(void);
unsigned diagnostics_line_count(void);
/* Zero is newest. Returned text remains valid until that ring slot is reused. */
const char *diagnostics_line(unsigned newest_offset);
#endif
