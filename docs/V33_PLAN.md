# V33 ship artwork, main menu and touch response

Baseline: releases/v32/ftl-ds.nds and verified checkpoints/v32-surrender-map.zip. Preserve user saves and installed FTL; extracted artwork stays local.

- [x] Replace rectangular enemy placeholders with original hull artwork and matching room/target transforms.
- [x] Preserve authored encounter strengths; clearly document reused hulls and the interim finale.
- [x] Add boot main menu with Continue/New Run, controls and sound; touch and buttons supported.
- [x] Keep menu simulation frozen, guard run replacement, preserve existing save until explicit save.
- [x] Investigate slow taps and eliminate missed or unnecessarily delayed input without repeated held-touch actions.
- [x] Add focused art/menu/input and save-preservation checks; inspect actual framebuffers.
- [x] Build ROM, run relevant full regressions and record memory.
- [x] Update roadmap/checkpoint, package release and verify recovery archive.
- [ ] Observe physical DS response/readability and emulator input/storage/audio acceptance (pending unless performed).
