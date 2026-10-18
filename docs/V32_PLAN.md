# V32 implementation checklist

Baseline: releases/v31/ftl-ds.nds and verified checkpoints/v31-enemy-loadouts.zip. Preserve user saves and installed FTL files; extracted content stays local.

- [x] Compile complete generic REBEL surrender/dead-crew/escape closure, retaining all source branches and weights.
- [x] Implement missing reward families and document authored reward amounts and default escape timing.
- [x] Integrate once-only offers, accept/refuse, callback priority and save-safe transient cleanup.
- [x] Provide clear touch/button choices and escape/ceasefire feedback with frozen negotiation.
- [x] Make the current-sector beacon map larger, with overview, camera-follow selection, clear ship/reach/fleet and consistent hit-testing.
- [x] Verify focused event/reward/map/persistence tests and complete earned runs; run full regression suite.
- [x] Inspect actual DS framebuffers; build ROM and record static memory.
- [x] Update roadmap/checkpoint, preserve release and verify recovery archive.
- [ ] Physical DS and emulator runtime/input/audio/storage acceptance (pending unless observed).

Original-game source data, tested DS behavior and intentional approximations must be distinguished. Full base-game scope remains in FULL_GAME_CHECKLIST.md.
