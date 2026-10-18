# V41 Federation base defense checkpoint

## Completed

The final sector now follows the original StarMap boss rules recovered from FTL 1.6.14:
- The rebel flagship travels a 4–6 beacon route toward the Federation base, moving every second jump.
- Rebel control spreads by one or two beacons per jump.
- Once the flagship reaches the base, the player has three jumps to engage it before the base is destroyed.
- After each defeated phase the flagship jumps away. If beaten at the base, it retreats one beacon and turns back.
- The sector has the original mix: one store, three automatic Federation repair stations and six hostile beacons. The base gives one-time supplies.
- The map shows the flagship, the base, rebel-held beacons, next-turn takeovers and the countdown.

See [V41_FINAL_SECTOR.md](V41_FINAL_SECTOR.md).

The phase battles still use the interim Rebel Command platforms; replacing them with the authentic flagship is the next sub-milestone. Legacy six-sector runs and save-24 runs already inside the final sector keep the interim exit battle.

Save format 25 appends the final-sector state, and saves 2–24 migrate. Existing saves, backups and installed FTL files are untouched.

## Build and evidence

- Full suite passed: 127 host checks (new `final_sector_smoke`) and 45 Python tests (build/v41-check.log). This includes six earned branching eight-sector campaigns and twelve policy-5 campaigns that now intercept and defeat the moving flagship, with all cold loads in format 25.
- `final_sector_smoke` covers 40 generated routes and the exact movement cadence. It also covers base loss on the third jump after arrival, interception, post-phase jumps, reversal at the base, all three phases, rebel spread, repair stations, base supplies, the save-25 round trip and the v24 migration rule.
- Four focused checks also pass with undefined-behavior traps (build/v41-focused.log).
- Test updates:
  - About 20 save tests rebuilt older formats with fixed tail sizes; they now account for the 60-byte V25 tail.
  - Four scripted drivers now chase the flagship, and wait when it shares their beacon.
  - `surrender_run_smoke` no longer requires a declined weapon offer, since its runs no longer fill cargo; `large_campaign_run_smoke` still requires one.
  - The campaign length proxy is now at least 60 jumps (seven full sectors plus the interception).
- ROM: 40,938,496 bytes; SHA-256 `f882fdc1e300ac0bcf8159c16ce22a3d7a57bf308859997b2e39614e89740029`. The build shows only the existing SDK warning.
- All 17 audio files verified. Static ARM9 union 924,940 bytes. Campaign state is 22,216 bytes (+60).
- Previews: `final-sector-map`, `final-sector-overview` and `final-sector-base-attack` (v41) were inspected. They are host renders.

## Decisions

- A fled phase restarts at full hull (the existing DS policy; the original is unverified).
- Neutral beacons are approximated, and rebel-held fights pay the patrol reward.

## Next

V42: imported flagship hull/layout and phase 1 (artillery rooms and cloaking). Then phases 2–3 (drones, invasion boarding, power surges). Physical DS acceptance is pending.

Recovery: releases/v40/ftl-ds.nds, checkpoints/v40-calm-crew.zip. Current ROM: releases/v41/ftl-ds.nds. Snapshot and hashes: checkpoints/v41-base-defense.zip and build/v41-manifest.json.
