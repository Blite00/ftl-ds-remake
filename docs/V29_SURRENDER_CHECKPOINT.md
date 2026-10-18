# V29 surrender foundation checkpoint

## Implemented and scope

Pure game_can_accept_surrender and atomic game_accept_surrender preflight room capacity, return living boarding parties, preserve identities/health/skills and leave unrelated carried intruders aboard. Pending home enemy deaths move to carried ownership without moving their damage. Pending remote deaths are retired before later corpse normalization. Queued attacks, projectile effects, drone deployments and enemy escape countdown stop; both hulls remain intact. Surrender has a separate result/outcome and no destruction animation.

Negotiated automatic party exchange, projectile cancellation and pending-death handling are documented DS adaptations, not behavior proven by the original XML. Existing home service continues carried-hostile combat while the surrendered remote crew remain frozen. Refused or repeated acceptance leaves state unchanged. Acceptance itself grants no resources; the future campaign callback owns rewards.

The renderer has a distinct ceasefire status/result screen with framebuffer coverage. A save22 integration fixture preserves exchanged crew and a pending home Zoltan death across cold loads, applying its damage once.

The original event compiler now imports strict surrender descriptors and complete accepted/refused/nested branches. Catalog schema 4 appends min/max/chance/node fields. Weighted text lists support up to 32 entries without deduplication. Unsupported branches still reject the complete closure. A real archive test compiles ENGI_UNLOCK_2FAKE, but this quest stage remains excluded because its originating quest is unfinished.

**No surrender offer is enabled in the playable campaign yet.** All nine existing original roots retain identical nodes, choices, effects, alternatives and enemy pools. V28 remains the latest completed feature release; the development ROM still displays V28. The full base-game goal remains incomplete.

## Validation

- 95 C checks and 34 Python compiler tests passed: build/v29-surrender-checks.txt.
- Existing six earned eight-sector regressions completed 389 cold loads; three earned retreat campaigns completed 194 cold loads. These verify existing playability, not surrender.
- Core surrender, departure, enemy escape and boarding checks passed strict compilation and undefined-behavior trap instrumentation. The final persistence/UI fixture also passed after shortening a clipped status message.
- All 115 V28 preview images recorded in the previous manifest remain byte-identical. New ceasefire and resumed-crew framebuffers were inspected. Evidence: build/v29-surrender-compatibility.json and build/surrender-*-v29.png.
- Save format remains 22; no persistent fields or migrations were added.
- ROM build passed with the existing SDK executable-stack metadata warning. Development ROM: releases/development/v29-surrender-foundation.nds, 547840 bytes, SHA-256 `4124deb138505ac93ff55a1ba5f9c3b8f7fde5caa89f5b8b269e6e03ffd0b5f2`.
- Static ARM9 union 810732 bytes; high-water offset 810760. Game 17708 / Campaign 21752 bytes. This measures the current ROM; unused core APIs may be removed by link optimization until runtime integration calls them. Runtime memory and frame pacing remain unmeasured.

## Next steps

1. Implement explicit crew recruitment rewards and weapon loot decisions, including full-roster/cargo outcomes. Do not discard rare source branches or silently lose loot.
2. Add required reward families with documented DS numeric policies, and chance-based/default-timer escape closures with absent-callback handling.
3. Enable the complete generic REBEL closure only after all dependencies pass; implement once-only offer triggers, overlapping escape/surrender policy, frozen accept/refuse choices, source callbacks and UI.
4. Add earned surrender campaigns and persistence checks, then continue remaining equipment, original quests/hazards, ships, authentic finale and progression from FULL_GAME_CHECKLIST.md.

Exact source dependencies and exclusions are in V29_SURRENDER_SOURCE.md. The persistent checklist is V29_SURRENDER_PLAN.md. Emulator input/audio/storage, runtime profiling and physical DS acceptance remain pending; independent software work continues.

Installed FTL and user saves were preserved; extracted content remains local. Recoverable archive: checkpoints/v29-surrender-foundation.zip; manifest: build/v29-surrender-manifest.json; hash sidecar accompanies the archive.
