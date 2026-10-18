# V28 development checkpoint: player FTL charge foundation

Date: October 3, 2026. Status: verified development integration; full V28 retreat remains incomplete. V27 remains the latest completed feature release.

## Implemented

- `Game.player_ftl_charge`, bounded at 1200 units, and pure operational/readiness queries.
- Charging requires living stationary home crew in an intact, non-ionized cockpit and effective engine power. Shared power rules include Zoltan contributions.
- Provisional DS timing: each effective engine bar adds one unit per simulation tick, capped at eight bars; 40 seconds at one bar and 20 seconds at two bars. This is authored behavior, not a verified original-engine formula. Manning/skill timing refinements remain open.
- Charge holds during power/pilot interruption, pause, service time and terminal outcomes. Impacts and terminal results resolve before charging.
- Successful encounter initialization resets charge; a refused initialization and enemy escape configuration preserve it.
- Charge is transient under the existing map-only save policy. Save20 remains unchanged; no user save paths were touched by tests.

## Evidence

- `tools/check.ps1`: 82 C host checks and 28 Python compiler tests passed; log `build/v28-charge-checks.txt`, isolated run directory `build/check-ee3a60bd2d694292ad2f925ff0f19853`.
- New `player_ftl_smoke` covers engine rates, saturation, power/ion/pilot interruption, species power, actual last-tick impacts, freeze and encounter reset. Focused fixtures are not complete-campaign evidence.
- New `player_ftl_campaign_smoke` covers real starting-ship combat ticks, frozen narrative/pause, no travel/resource side effects, byte-identical save payloads, cold-load charge reset and active-save rejection.
- Six earned eight-sector regression campaigns completed 389 jumps and 389 disk cold loads. They exercise existing campaign behavior, not player retreat, which is still unavailable.
- `tools/preview.ps1`: all 104 PNGs match the V27 manifest exactly (`build/v28-charge-preview-comparison.json`); the actual escape charging framebuffer was visually inspected. No renderer changes were made.
- `build.ps1`: ROM builds; existing SDK `.note.GNU-stack` warning remains.
- `tools/memory_report.py`: static ARM9 union 747684 bytes; high-water 747712; Game 13484 / Campaign 17524. These are static measurements only.
- Development ROM: `releases/development/v28-charge-foundation.nds`, 535552 bytes, SHA-256 `6fbc99f696e33b4c4f84e05e7b179afcf2cfdb1880dab68d0fb7e4650857a438`. The existing map header still identifies V27; no new retreat UI is advertised.
- V27 baseline archive rechecked against its sidecar: `299fdcaca07d60c0a5e2923adeeb29f1c29ed3f92eb455d3c2781ba0042c49e6`.
- Recoverable development source/assets/ROM checkpoint: `checkpoints/v28-charge-foundation.zip`, with manifest and SHA-256 sidecar.

## Remaining work and exact next step

1. Implement separate ownership for carried hostile intruders, proven roster bounds and shared simulation/render iteration. Preserve identity, health, room and timers through repeated departures; prohibit recall by an unrelated enemy ship.
2. Add a validated departure operation and common arrival resolution. Handle fuel, fleet travel, explicit friendly-away abandonment and home hazards without stale rewards or remote resurrection.
3. Extend peaceful-arrival home-defense simulation and save persistence for intruders; validate migration and worst-case save capacity.
4. Integrate charge/readiness feedback, combat destination selection, cancel and named abandonment confirmation with shared touch/button APIs.
5. Cover ordinary/original-event and boss retreats, then run earned retreat campaigns and inspect affected screens before declaring the feature complete.

See `V28_RETREAT_DESIGN.md` for inspected constraints and proposed architecture. Surrender, broader events/hazards, remaining equipment and ships, authentic finale, progression and release acceptance remain in the full checklist. Physical DS and emulator input/audio/storage checks remain pending.
