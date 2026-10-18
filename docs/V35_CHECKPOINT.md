# V35 crew setup, deselection and combat timing

## Completed

- [x] New Run opens three-crew naming before initializing the campaign. Touch and button keyboard support, default names, 12-character limits, empty-name rejection and cancellation. Launch initializes a fresh campaign and applies the draft names; cancel restores the prior main menu/run. No save is written by setup or launch.
- [x] Empty-space taps on the ship view or B deselect crew and groups. Deselected room taps/A cannot order crew. Portraits and L/R reselect. A second tap on a selected portrait opens details. B closes existing panels first; after deselection, B opens retreat or leaves management.
- [x] Combat clock uses elapsed VBlank samples rather than main-loop iteration count. Up to four 30 Hz updates per draw; excessive stalls discard backlog. Pause, frozen panels, setup/menu and scene transitions discard elapsed time. Ordinary targeting/weapon/crew-page changes keep the clock running. No cooldown/balance/save changes.
- [x] Bitmap drawing clips once and directly writes nontransparent pixels, removing repeated per-pixel rectangle clipping. See V35_PACING.md.
- [x] Build, host regressions, actual framebuffer inspection, release and recovery checkpoint.
- [ ] Physical DS combat speed, rendering smoothness, new keyboard and deselection acceptance.

## Evidence

119 host checks and 45 Python tests pass: build/v35-check.log. This includes setup draft/cancel/launch/limits and rendering bounds, deselection/input behavior, elapsed-time scheduler fractional carry/wrap/catch-up/reset, the existing campaign/save suite, and 12 earned policy5 campaigns with 783 jumps/cold loads. Scheduler, input capture, crew setup and bitmap reference checks also pass undefined-behavior trap builds.

Seven combat/target/destruction framebuffers remain byte-identical after the bitmap optimization. An independent reference verifies 144 clipping/transparency cases with framebuffer sentinels (build/v35-bitmap-equivalence.c). New setup/keyboard/deselected host framebuffer previews inspected. These are not emulator screenshots or physical DS timing measurements. Main-loop scheduling received a separate agent review after integration.

ROM build succeeds (build/v35-build.log); existing SDK executable-stack metadata warning remains. Static ARM9 union 901,940 bytes; high-water 901,992. Game 17,876 / Campaign 22,024 bytes. Runtime frame times and remaining heap/stack are unmeasured. See build/v35-memory.txt.

## Release and recovery

Current ROM: ftl-ds.nds. Preserved release: releases/v35/ftl-ds.nds. Size 612,352 bytes; SHA-256 `c37d9e974d95a81a23a852b7c5ed222ed237adee1c7edc111bc2eabe7ccb3a14`.

Recovery: checkpoints/v35-crew-timing.zip with build/v35-manifest.json and SHA-256 sidecar. V34 preserved release hash verified unchanged. Existing saves and installed FTL files were not modified. Save24 and original content policy remain unchanged; extracted art remains local. Launcher icon and V34 clear destruction view retained.

## Exact next checks

1. On DS, choose New Run, edit/cancel names, launch, save and resume; verify names persist. Cancel setup from an existing run and verify Resume restores it.
2. In combat, tap empty space/B, verify no room command while deselected, reselect with a portrait/L/R, and open details with a repeated portrait tap. B again or FTL opens the jump map.
3. Compare weapon charge duration and crew movement in a crowded battle. Pause/open Actions, wait, resume and check there is no time jump. Check final explosions and sound. Capture diagnostic timing if still slow; scheduler correction is not a measured frame-rate guarantee.
4. Continue broader controller-only management/content/balance and authentic Flagship work from FULL_GAME_CHECKLIST.md.
