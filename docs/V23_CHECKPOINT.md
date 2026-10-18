# V23 checkpoint: larger campaigns and fleet pressure

October 3, 2026. Software integration verified. Full base-game completion, emulator input/audio/DLDI and physical DS acceptance remain pending.

## Implemented

- New runs use eight sectors and 23 active beacons within 24-slot bounded storage: departure, seven columns of three beacons, exit. Guaranteed forward routes and seeded cross-connections give route choices; anchor stores in columns 3/6 and repair in column 7 coexist with generated events.
- A full-map overview shows links, current location, selected beacon, captured territory and next-turn danger. Touch and L/R selection cover every active beacon without selecting unused storage.
- Fleet pursuit begins after eight sector turns. New fronts advance 14 map pixels per turn (authored half-column pacing); legacy fronts retain their previous layer formula. Travel/wait advances one; rescue advances two.
- Captured stores, supplies and exits trigger patrols before normal content. Captured services/rewards are suppressed; patrol victory gives emergency fuel. Captured exit victory opens onward travel once without the ordinary exit cache or initial finale tender. Existing between-phase boss assistance remains its authored policy.
- Down/WAIT is available on the map. It spends no fuel and may trigger a patrol if the current beacon becomes captured. Mode and away-crew guards prevent invalid waiting/departure.
- Map-only format 16 preserves active node/sector counts, graph, visits, stock and pursuit counters. The original format 15 prefix is retained; a fixed 920-byte appendix holds limits and extra records. Inactive records are validated as zero.
- Old saves retain 12 beacons/six sectors and generation policy after load/resave. New runs receive 23/eight. No existing run is silently rerolled or lengthened.

## Evidence

All 67 registered strict host checks passed (build/v23-checks.txt). Final fleet UI was rerun after fixing pre-arrival shading (build/v23-ui-final-check.txt). Graph tests cover 1,024 seeds with connectivity, symmetry, deterministic generation, coordinate/bounds and inactive-slot checks. Focused fleet tests cover forecast purity, captured stores/supplies/exits, denied rewards, repeated choices, wait/rescue and final-exit tender suppression.

Three ordinary-action eight-sector campaigns used actual graph-selected routes and the existing public-action pilot/economy helpers. Seeds 1/42/8675309 completed 65/64/66 jumps respectively, with final hull 30 and scrap 161/143/47. All 195 map checkpoints were saved and cold-loaded, preserving graph, stock, resources, random state and progression. No wins, resources or damage were injected. These are automated playability checks; human difficulty/balance acceptance remains pending.

Existing six-sector combat/equipment/boarding/species campaigns now explicitly initialize the supported legacy 12/6 policy. They remain regression evidence for those rules. New eight-sector runs are separate tests; this release does not imply every tactical build has completed the new campaign.

Actual archived V22 writer migration (format 15→16) preserved legacy limits, graph, visits, stock, RNG, resources, counters and Defense I ownership. Old payload bytes exactly match the new prefix; backup SHA-256 ac74aee78f8a63fd388d77c40462aa6abafa7edb0016b22bc116e229cc6c6fec. Fixture size 2744→3664 bytes. See build/v23-release-migration.json; fixture funding is compatibility evidence only.

Inspected actual framebuffers: map23-v23.png, fleet-threat-v23.png, fleet-captured-store-v23.png and fleet-legacy12-v23.png. Final map preview no longer shows red territory before pursuit. ROM build passed with the existing executable-stack metadata warning. ARM9 static union 708268 / high-water 708296; Game 13460 / Campaign 15644 bytes. Runtime stack/heap and frame time remain unmeasured.

Release: releases/v23/ftl-ds-v23.nds, 499712 bytes, SHA-256 c05219681b3d0a28e1b0f08963d04b44c138ce5fdb0225d815eb9f38475eb18f.
Recoverable checkpoint: checkpoints/v23-large-campaign.zip with per-file manifest and SHA-256 sidecar. V22 archive hash verified before work; older releases preserved.

## Limitations and exact next steps

1. Add persistent sector identities/types and branching next-sector choices with appropriate supported content pools.
2. Add validated original event conditions/effects/quests and active hazards; the current event dispatcher and geography are authored approximations.
3. Build Federation base defense, moving flagship and original-supported three-phase behavior. Current endpoint remains the authored Rebel Command boss.
4. Complete remaining drone roles/lifecycle, cloaking, augments, artillery, special shields, shared inventory/excess loot and dependent playable ships.
5. Finish hangar/unlocks/run records, full animation/tutorial/audio/lid coverage and release validation on emulator and physical DS.

The map uses a compact whole-sector overview; no panning was needed for 23 beacons in inspected framebuffers. Physical input comfort remains unverified. Fleet pacing and guaranteed service locations are authored balance choices, not recovered original generation behavior.
