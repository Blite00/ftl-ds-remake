# V36 original FTL audio

Original music and sound effects are converted from the user's installed FTL archive by the local importer. Original assets and derived PCM remain local. The installed archive and saves are not modified. See generated/audio manifest for source paths, hashes, processing and peaks.

## Playback and scope

Five full-length tracks: title screen, Civil exploration, Civil battle, Last Stand, and the victory cue. Menu/crew setup use title music; map/events/stores use exploration; regular battles use battle music; authored finale uses Last Stand. Victory plays once. Other music loops. Each mode transition starts its assigned track from the beginning: seamless synchronized explore/battle crossfades are not implemented. Defeat has no substitute music track.

Twelve original effects cover laser, missile, ion, beam, bomb teleport, hull/shield impacts, misses, destruction, jumping, and interface success/failure. Intercepted projectiles use the miss cue. Cue assignments are authored DS behavior, not verified PC event timing. Projectile impacts trigger when result changes; held/paused effects do not retrigger. Destruction triggers only when an outcome ends with a destroyed hull, not crew defeat or escape. Four concurrent effects are mixed; the next rotating slot is replaced when all voices are busy. Stereo direction is downmixed to mono.

## DS implementation

Music: signed PCM16 little endian, mono22050Hz, streamed from embedded NitroFS. Effects: resident signed PCM16 mono16000Hz, linearly resampled during mixing. Music and effect mixing saturates instead of wrapping. Music uses80/127 gain and ducks by half while effects play. Concurrent effects are averaged to retain headroom under overlapping volleys; the imported peaks are limited to-3dBFS. This is an authored mix policy. All mixing/file reads happen in the main loop through manual Maxmod servicing; no filesystem work occurs in VBlank or audio interrupts.

The Maxmod ring is2048samples (~93ms of mono music,4096bytes of sample data). Buffered SFX can therefore have roughly one buffer of delay; scheduling and flashcart reads add uncertainty. This favors control feedback over a larger buffer's underrun tolerance. This is a configured bound, not a measured end-to-end latency. A slow render/storage operation can underrun. Physical DS measurements are pending.

Resident effects have a hard768KiB allocation budget, with exact converted sample sizes in the import manifest. Stream and effects do not load whole music tracks. Maxmod reserves ARM9 timer2 after checking it is unused; performance timing remains on0/1. Maxmod owns the playback output while originals are active; libnds synthesis runs only as fallback, avoiding channel conflicts.

All required music files are size-validated and effects loaded before original playback is enabled. Unavailable NitroFS, busy timer2 or failed sample allocation keeps the authored synth fallback. Diagnostics/UI report the active backend. A later music read error yields silence for that music stream and reports ORIGINAL: READ FAILED; effects remain usable. Missing/truncated files cannot loop forever. Muting immediately sets hardware stream volume to zero, closes the Maxmod stream and music file and cancels effects, discarding queued output. Unmuting restarts the mode's track. LOW/MED/HIGH changes preserve playback position. No setting/save-format change.

## Validation

Portable checks cover exact looping, one-shot EOF silence, incorrect size/missing files, short reads, mute gains, bounded voice replacement, resampling completion and saturation. An SDK-stub backend test additionally checks cue edges, shield/intercept routing, destruction, menu/finale modes, interface cues, immediate mute and volume-position continuity. Existing synth scheduler tests remain applicable to fallback. A ROM build verifies SDK integration; no host check establishes speaker volume, stream underruns, flashcart access or physical DS playback quality.

Physical acceptance: verify title/crew setup; map/battle/finale/victory transitions; repeated taps; pause/resume; mute/unmute and all volume levels; simultaneous volleys/shield hits; both ship destruction outcomes; launch under the actual flashcart menu; SD save during music; several minutes of sustained battle playback and frame timing.
