# V29 combat and audio playtest fixes

## User feedback addressed

- Audio sounded rough: music and launch cues now use a shared smooth sine PCM loop instead of square-wave PSG. Music has gradual volume changes, slower phrases and lower volume; hull noise is much quieter and decays.
- Combat text felt busy: aiming replaces ship decoration and repeated hints with one selected-target card; stale encounter/pause chatter is suppressed. Essential FIRE, power, autofire, pause and back controls remain.
- Targeting appeared to affect only hull: the card names the target system, explains its consequence and lists hull/system damage separately. It shows current health/disabled state only when sensors already permit that information. Room taps explicitly say to press FIRE/A. Resolved room-targeted shots briefly show HIT, MISS, SHIELD BLOCK or INTERCEPTED in the card. Confirmed system damage and shield absorption also have distinct short messages at normal sensors.

## Mechanics finding

The combat core already applies room-specific system damage. Actual charged-weapon tests verify shield capacity loss, reduced engines/escape readiness and stopped weapon charging once usable power is insufficient. An empty room takes hull damage without damaging a main system. Shields absorb ordinary lasers; missiles bypass the supported normal shields. A partially damaged weapon system may retain enough power to keep firing. Shots already queued keep their original target when the next volley is retargeted.

The new feedback does not change weapon damage or enemy balance. Save format23 is unchanged. Surrender and full-game work remain incomplete.

## Validation and limits

101 host checks and 37 compiler tests pass. The ROM builds successfully. See build/v29-polish-evidence.json for final checks, ROM hash and static memory report. The target card has framebuffer guards and checks that hidden enemy health stays hidden; affected host previews are visually inspected. Audio tests verify waveform/envelope bounds, cue expiration, no repeated paused projectile cues and nonrepeating stingers. An initial full run caught missing bomb hit/miss feedback after simplifying the aiming screen; the impact cue fixes it and the original regression assertion is retained. Silent tests cannot establish perceived sound quality. Listening and hands-on confirmation of this revised build in melonDS and on physical DS remain pending.

Current ROM: G:/demake/ftl-ds.nds. Preserved polished release: releases/v29-polish/ftl-ds.nds. Prior V29 loot release remains at releases/v29/ftl-ds.nds and checkpoints/v29-loot-recruitment.zip. No user saves or installed FTL files are changed.

Recovery: checkpoints/v29-combat-polish.zip and its SHA-256 sidecar. ROM SHA-256: `c71d8270ffa812da43a13d506128715e74b6812ebf8ffc1b910178c3f1796343`.
