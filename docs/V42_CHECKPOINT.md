# V42 authentic flagship checkpoint

## Completed

The final battle now uses the original Rebel flagship instead of the interim platforms. It is built from the imported BOSS_1–3_NORMAL stages: hull art, layouts and doors, 4-layer shields and stage hulls (20/22/20). See [V42_FLAGSHIP.md](V42_FLAGSHIP.md).

- **Artillery:** independent weapon rooms for the laser, shield-piercing missile, beam and ion. Charge speed scales with power (×(1.5 − 0.25 × bars)). Each volley hits distinct random rooms.
- **Cloaking (stage 1):** +60% evasion and a 5 s-per-bar cloak followed by a 20 s ion lockout. While it's cloaked, your weapons stop charging and can't target it.
- **Crew:** 11 humans per the original StartStage, including isolated gunners. Survivors carry over per room, and the gunners of lost weapon rooms disappear.
- **AI takeover:** killing the whole crew no longer wins. The AI takes over and repairs.
- **Stage 3 boarding:** an invasion-mode teleporter.
- **Stage rewards:** stages 1 and 2 pay scrap.

To support 11 crew, current-encounter enemy slots went from 8 to 12. Carried intruders keep their save numbering.

Not done yet: stage 2 drones and the drone surge (V43); the stage 3 super shield and laser barrage (V44). Until then, stages 2–3 are weaker than the original.

Save format 26 (+80 bytes, per-room crew), and formats 2–25 migrate. Legacy six-sector runs and save-24 runs inside the final sector keep the interim platforms. Existing saves and installed FTL files are untouched.

## Build and evidence

- Full suite passed: 128 host checks (new `flagship_smoke`) and 45 Python tests (build/v42-check.log). `flagship_smoke` covers:
  - stage data and crew placement
  - gunner isolation
  - artillery scaling (10 s and 20 s) and distinct targets
  - all four effect types
  - damage, ion and sabotage
  - cloak timing, evasion, charge pause and lockout
  - AI takeover and repair
  - campaign persistence and stage scrap
  - save 26
  - the winnability statistic
- Seven focused checks also pass with undefined-behavior traps (build/v42-focused.log).
- **Difficulty:** all 18 scripted eight-sector fixtures reach and lose to the authentic stage 1, which they now record as a valid run end. A strong attainable DS build wins 16/20 seeded stage-1 fights.
- Fixture pilot changes, all ordinary player orders:
  - Aim at artillery rooms with whole volleys.
  - Spend late-game scrap on the reactor first, then shields.
  - Keep missiles stocked.
  - Repair oxygen first.
  - Run a re-evaluating ship-management loop, and vent the ship when 3 or more rooms burn.
- ROM: 40,986,624 bytes; SHA-256 `1bdffb51dcc48deab6372ac6686a190a7af34fead15c285107747b2a60d96bfa`. The build shows only the existing SDK warning.
- All 17 audio files verified. Static ARM9 union 978,948 bytes (+54 KB: flagship art and data, plus state). Game state is 19,268 bytes and Campaign 23,556.
- Previews: `flagship-stage1..3-v42.png` and `flagship-cloaked-v42.png` were inspected. They are host renders.

## Decision needing the user

Authentic normal difficulty is kept, as the closest to the original. Playtesting decides whether to switch to the original Easy blueprints or adjust the DS economy. See V42_FLAGSHIP.md, "Difficulty decision".

## Next

V43: stage-2 drones and drone surge. V44: stage-3 super shield and barrage. Physical DS acceptance is pending.

Recovery: releases/v41/ftl-ds.nds, checkpoints/v41-base-defense.zip. Current ROM: releases/v42/ftl-ds.nds. Snapshot and hashes: checkpoints/v42-flagship.zip and build/v42-manifest.json.
