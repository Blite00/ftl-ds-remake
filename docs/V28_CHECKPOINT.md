# V28 player retreat checkpoint

## Implemented

Player FTL charge now leads to an atomic departure and normal campaign arrival. Touch the FTL control or press B from combat crew controls; select a linked beacon and confirm. The destination map holds simulation and explains charge, fuel, fleet capture and boarding consequences. Opening, cancelling and rejected travel preserve campaign state.

Departure spends one fuel and advances travel/pursuit once. It grants no source battle salvage or original destruction/escape callbacks. Live and pending-death home intruders move to carried ownership. Away friendly survivors require named confirmation; abandoned deaths cannot later burst on the home ship. Remote shots, volleys, drones and encounter references are detached.

Unresolved arrival events provide explicit home defense. It advances home crew/hazards while preserving choices, narrative, resources, fleet and charge. Returning restores the event. Home defeat takes precedence over modal views. New enemies do not own carried intruders, and current battle outcomes remain separate from their threat.

Save22 appends the unfinished authored finale phase after the save21 intruder appendix. Old formats default to unstarted; malformed/non-final phase values reject atomically. Returning to the current authored finale restarts the same phase at full enemy hull without duplicate tender or interphase supplies.

## Evidence

- 93 C host checks and 28 original-event compiler tests passed: `build/v28-retreat-checks-release.txt`.
- Three ordinarily earned retreats: 1800 total charging simulation ticks, three complete eight-sector campaigns, 200 jumps and 194 disk cold loads. No injected charge, health, resources or victories. This is campaign API evidence, not observed human input.
- Existing six earned eight-sector regressions: 389 cold loads, 70 original choices and 2 trades. These do not independently prove player retreat.
- Focused tests cover atomic rejection, explicit abandonment, carried pending deaths, all arrival classes, source callback suppression, home defense clock policy, boss retry/persistence, UI cancellation/confirmation and framebuffer bounds.
- Final retreat UI check passed strict compilation and undefined-behavior trap instrumentation, including home defeat and restart. Fixture state is used for isolated UI/boarding edge cases and is not counted as earned campaign evidence.
- Renderer previews inspected: charging map, ready/captured destination, abandonment list (including maximum roster stress fixture), combat controls, home defense, arrival narrative and four-choice original event. These are actual host renderer output, not emulator screenshots.
- Save tests preserve historical payload/backup behavior through format21. An archived save21 writer was not independently rebuilt for this milestone; source-backed migration evidence from earlier milestones retains its original scope.
- ROM built: `547840` bytes; SHA-256 `bd2330d48e9765baf37e45de41d30e33623e3e06304ff8be4eff21098ee0fd3a`. Release: `releases/v28/ftl-ds.nds`.
- Static ARM9 union 810412 bytes; high-water offset 810440. Game 17708 / Campaign 21752 bytes. Atomic travel adds a bounded static Campaign staging buffer. This does not measure runtime stack/heap or frame timing.

## Source facts and adaptations

Existing local original imports remain unchanged. Charging is an authored 1200-unit model, advancing with effective engine power while a healthy pilot is present; no skill bonus is claimed. Campaign maps, current loadouts and the interim finale retry rule retain their documented adaptations. Full moving-flagship persistence/base defense remains required.

Installed FTL files and user saves were not used as test fixtures or modified. Extracted content and release assets remain local.

## Limits and next steps

The full base-game objective remains incomplete. Earned boarding-retreat campaigns, observed emulator input/audio/storage, runtime profiling and physical DS/flashcart acceptance remain pending. Existing SDK executable-stack metadata warning persists.

Next: implement surrender offers and their original event outcome callbacks with once-only rewards, boarding/away-crew handling and explicit acceptance UI. Then extend original conditions/quests and active hazards, followed by remaining equipment/ship/finale/progression work from FULL_GAME_CHECKLIST.md. Retain all full-game gates.

Recoverable archive: `checkpoints/v28-player-retreat.zip`; SHA sidecar: `checkpoints/v28-player-retreat.sha256`; file manifest: `build/v28-retreat-manifest.json`.
