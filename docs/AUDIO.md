# Original synthesized audio

`source/audio.c` uses the installed BlocksDS libnds sound API and default ARM7 core.
It plays original sine-wave PCM note patterns for map and combat, victory/defeat
stingers, weapon launch tones and noise for hull damage. These are newly written
sounds, not the PC FTL soundtrack or recovered audio behavior.

## Integration and resource ownership

Call `audio_init()` after normal DS initialization. Call `audio_update(game, mode)`
once per 30 Hz display update, including paused display updates so finite sound
effects can end. Supply AUDIO_MAP, AUDIO_COMBAT, AUDIO_VICTORY, AUDIO_DEFEAT or
AUDIO_SILENT. Diagnostics can use AUDIO_SILENT. `audio_toggle()` immediately stops
all owned channels; `audio_enabled()` exposes the user setting. The toggle is
session-local, not stored in campaign saves.

Sound channels 8 and 9 play music, 10 plays a weapon cue and 14 plays noise. At
most four hardware channels are active. A shared 64-byte, 32-sample looping waveform supplies music and weapon tones. Its cache is flushed once during initialization. There are no streaming buffers, heap allocations, external audio files or ARM9 timer claims. The ARM9 profiler's timers 0/1
remain reserved for the profiler. No extra linker libraries or asset generation
are needed beyond the current BlocksDS ARM9 template/default ARM7.

Sound requests are bounded: a two-note music step every 24 or 36 updates, at most
one weapon cue and one damage cue per update. Simultaneous projectiles share a
cue. Shield-only hits currently have no separate sound. Music continues during
combat pause; use AUDIO_SILENT if a screen should stop all sound.

The API contract is taken from the installed
`libs/libnds/include/nds/arm9/sound.h`, including explicit-channel sample/noise
functions, soundSetVolume and soundKill. Non-ARM9 builds exercise scheduling with silent wrappers.

## Acceptance

Compilation and silent host behavior can establish API/build correctness only.
Audibility, volume balance, music pacing, mute behavior, lid/suspend behavior and
long-run interaction with the default ARM7 still require emulator listening and
physical DS validation. No physical audio validation has been performed.

Validation performed: ARM9 and host module compilation passed with -Wall -Wextra -Werror. A focused silent host scheduling check passed for one-shot expiration, no repeated paused-projectile cue, hull-noise expiration, mute mode, bounded looping music and nonrepeating victory stingers. Audio listening is still pending.

## V29 combat polish

User feedback identified harsh audio in the previous build. Square-wave music/weapon tones have been replaced with a smooth sine PCM loop. Music has a short attack and longer release, slower phrasing, closer stereo placement and lower levels. Weapon and hull cues are quieter and decay; mute and frozen-effect non-retrigger behavior remain. Silent host tests verify envelope progression and sample bounds in addition to scheduling. Listening to the revised mix on emulator/DS remains pending; improved subjective sound quality is not claimed as verified.

## V30 real-DS volume correction

The user reports that V29 polish is too quiet at maximum physical DS volume. V30 keeps the smooth waveform/envelopes but raises master volume from 72 to 127. HIGH applies 4x the prior channel settings (music peak 48, weapon 72, impact noise 32); MED applies 2x and LOW 1x, with each channel clamped to 127. OFF mutes. Actions and management expose these session-local settings; HIGH is the startup default. These numbers describe mixer settings, not measured acoustic output. Clarity, clipping, speaker/headphone balance and physical controls in the revised build still need listening validation.
