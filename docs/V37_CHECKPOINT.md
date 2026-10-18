# V37 combat AI checkpoint

## Completed

Enemy crew now use species-relative medical thresholds, prioritize repairs and split defense/emergency duties. Nonpilots handle available work before the pilot; workers already beside emergencies are preferred. Power accounting includes drones and teleporters. Boarding parties launch without larger-roster deadlock, preserve home orders and seek useful sabotage targets. See [V37_COMBAT_AI.md](V37_COMBAT_AI.md) for source evidence and intentional approximations.

V36 original audio remains embedded. Save layout is unchanged. Existing saves and installed FTL files remain untouched. V36 release hash verified against its recovery manifest.

## Build and evidence

- Full suite passed: 123 host checks and 45 Python tests, including campaign runs and save/cold-load regressions (build/v37-check.log).

- ROM: 40,927,232 bytes; SHA-256 `8ec7a4bc270dc40acfede7c71a1162da27e1dc0fb9f7758f215011c01a7086bb`.
- Build successful; only the existing SDK executable-stack metadata warning.
- Final focused AI, boarding and crew checks passed with strict warnings and undefined-behavior traps: build/v37-ai-focused.log.
- Two historical healing fixtures now start at 20 HP to exercise the new below 25% threshold; reactor and healing assertions remain in place.
- Host framebuffer previews regenerated and enemy-ai-triage-v37.png inspected. This is a controlled scenario rendered with the actual game renderer, not an emulator capture.
- All 17 embedded audio files verified byte-exact against the import manifest.
- Static ARM9 union 912,284 bytes; high-water 912,336. Game 17,876 and Campaign 22,024 bytes unchanged. Dynamic V36 audio allocation remains additional.

## Limitations and next steps

This is the first AI fidelity pass. Weapon aiming/volley rules, boarding landing/recall choices, calm-ship behavior, automated repair and the interim finale still need comparison and implementation. The scheduler and power policy remain DS approximations. Full parity is not claimed.

Next: validate weapon targeting against original-game observations, then implement/test supported normal-difficulty behavior. On DS, test wounded enemies healing while shield repairs/boarding continue, party deployment, sustained combat pacing, flashcart audio and save/resume. Physical DS and emulator acceptance are pending.

Recovery: releases/v36/ftl-ds.nds and checkpoints/v36-original-audio.zip. Current ROM: releases/v37/ftl-ds.nds. V37 snapshot and hashes: checkpoints/v37-combat-ai.zip and build/v37-manifest.json.
