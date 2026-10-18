#ifndef FTL_DS_AUDIO_H
#define FTL_DS_AUDIO_H
#include "game.h"
enum AudioMode { AUDIO_MAP, AUDIO_COMBAT, AUDIO_VICTORY, AUDIO_DEFEAT, AUDIO_SILENT, AUDIO_MENU, AUDIO_FINALE };
/* Imported PCM music/effects via Maxmod; authored synth fallback if NitroFS is unavailable.
   audio_update observes game changes; audio_service MUST run each outer main-loop iteration.
   Imported backend reserves timer2, bounds resident effects to768KiB, stream to2048samples. */
void audio_init(void);
void audio_service(void);
const char *audio_status(void);
void audio_feedback(int error);
void audio_jump(void);
void audio_update(const Game *game, enum AudioMode mode);
void audio_toggle(void);
int audio_enabled(void);
/* Session volume: 0 off, 1 low, 2 medium, 3 high (default). */
int audio_volume_level(void);
const char*audio_volume_label(void);
void audio_cycle_volume(void);
/* V68 original alarms aboard the player ship: fire, breach, O2 below 25%
 * (warning_oxygen), hull at or below 25% (warning_hull_25), living intruders.
 * Pure: audio loops one voice per set bit in combat; render shows the warnings. */
enum AudioAlarm { ALARM_FIRE, ALARM_BREACH, ALARM_OXYGEN, ALARM_HULL, ALARM_INTRUDER, ALARM_COUNT };
unsigned audio_alarm_mask(const Game *game);
#endif
