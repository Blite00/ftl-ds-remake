# V24 checkpoint: sector identities and branching routes

October 3, 2026. Software integration verified. Full base-game completion, observed emulator input/audio/DLDI and physical DS acceptance remain pending.

## Implemented

- New campaigns begin in Civilian territory. Exits from sectors 1-6 offer two distinct destinations chosen from Civilian, Engi, Rebel and Pirate. Sector 7 leads to Last Stand; sector 8 retains the current authored finale.
- Sector identity changes weighted beacon and ordinary enemy pools. Initial supplies, two store anchors, courier and pre-exit repair preserve the current economy. Exact pool weights and existing enemy IDs are in [V24_SECTOR_DESIGN.md](V24_SECTOR_DESIGN.md).
- Route offers are generated once per sector. Queries and frames do not consume randomness. Chosen identities persist in eight history entries. The fleet pursues whichever route is selected.
- Branching requires explicit choice. Both touch and Up/Down/A work for events, with selected focus, encounter hints and unavailable-action feedback. Repeated confirmation cannot advance twice or duplicate the exit cache. Same-frame touch is suppressed after a controller decision.
- Map-only save format 17 appends 44 bytes for route policy, history and pending offers. Formats 2-16 migrate to policy 0, preserving existing maps, campaign lengths, generation and unbranched progression. New runs use policy 1. Start a new run for branching.
- Current README and roadmap were reconciled against implementation; the historical roadmap is preserved separately.

## Evidence

All **70 registered strict host checks passed** (build/v24-checks.txt). Extended route UI checks passed again after their final additions (build/v24-ui-final-check.txt). Focused tests cover 256 synthetic eight-sector histories and 2,048 weighted map/enemy samples, stable/distinct offers, selection/away-crew/repeat guards and captured cache suppression. Existing graph checks cover 1,024 seeds. Synthetic transitions prove focused rules, not earned playability.

Six ordinary-action eight-sector campaigns completed both branch positions for seeds 1,42,8675309. Branch 0 made 66/64/64 jumps; branch 1 made 64/64/66. All six finished with 30 hull using earned resources and normal combat/economy actions. Across **388 jumps and 388 disk cold loads**, they visited every current sector type and preserved history/offers, graph, resources, stock and RNG. Human balance and other tactical builds remain pending.

An actual archived V23 writer produced a format 16 fixture. Migration to 17 preserved the entire old payload prefix, graph, stock, RNG, resources, counters, Defense I installation and Combat I cargo. Size 3664 to 3708 bytes. Original and backup SHA-256: `7444bd08a39499e17aec6a0078409a14dfa158e6cde61da43a336c6d3779edf5`. Fixture-funded drone purchases are compatibility evidence only. See build/v24-release-migration.json and its retained writer/reader source and data.

Inspected actual renderer previews: sector-routes-v24.png, sector-routes-second-v24.png, sector-identity-v24.png and sector-routes-blocked-v24.png. Labels, summaries, focus and rejection feedback fit both DS screens. These are host framebuffers, not emulator observations.

ROM build passed with the existing SDK executable-stack metadata warning. ARM9 static union 711548 bytes / high-water offset 711576; ARM ABI Game 13460 / Campaign 15688 bytes. Runtime memory/frame timing remain unmeasured.

Release: `releases/v24/ftl-ds-v24.nds`, 502784 bytes, SHA-256 `2d5719da55534c16147b6ae1ee579efc32a5224a56c3dff72b4b2f54e65adb90`.
Recoverable checkpoint: `checkpoints/v24-sector-routes.zip` with per-file manifest and SHA-256 sidecar. V23 baseline hash verified before work; older checkpoints preserved.

## Boundaries and exact next steps

The sector weights are authored rules over supported encounters. Engi territory does not add Engi enemy hulls. Last Stand is an identity; Federation base defense, moving flagship and original phase behavior remain required. Hazard events remain salvage choices, not active combat environments. Other original sector types and event pools remain unfinished.

1. Compile supported original event conditions/effects with an explicit coverage register, resource/equipment/species requirements, blue choices and quest consequences.
2. Add active hazards with pause, visibility, power and persistence interactions, then extend sector pools appropriately.
3. Complete other drone roles/lifecycle, cloaking, augments, artillery/special shields, shared inventory/overflow and original enemy/player hull coverage.
4. Complete authentic finale, hangar/unlocks/difficulty/history, animations, tutorial/audio/lid handling and release validation.

Physical DS and emulator input/audio/storage acceptance remain pending. The full objective remains incomplete.
