# V34 top-screen combat visibility

## Completed

- [x] Replace the opaque enemy armament card over the enemy hull with a single reserved line above the ships. Basic sensors show weapon families; upgraded sensors retain each slot's countdown, OFF, EMPTY and FIRING state. Individual charge bars are replaced by numeric countdowns.
- [x] Give hull destruction a clean top-screen presentation. Original hull artwork is centered vertically, with only a short result heading and optional pause hint. Target cards, combat statistics, weapon panels, old projectiles and feedback cannot cover the animation.
- [x] Preserve the existing animation timing, pause controls and automatic report transition. Crew-only outcomes and surrender keep their separate behavior. Targeting still uses the detailed target card during live combat.
- [x] Build the ROM and inspect normal combat at both sensor levels, enemy destruction with stale panels and player destruction.
- [x] Preserve the previous release, launcher icon and user saves. Gameplay/save formats are unchanged.
- [ ] Physical DS readability, animation and input acceptance.

## Evidence

Four focused host checks pass: enemy_threat_ui_smoke, target_card_smoke, enemy_hull_smoke and feedback_smoke (build/v34-ui-checks.log). The threat fixture also checks every animation frame for both hull results: the top framebuffer is identical with and without stale targeting/drone/actions/retreat/boarding/crew/feedback UI. It verifies campaign purity and framebuffer guards. Sensor privacy and individual weapon states remain tested in the relocated status line. The full campaign suite was not rerun for this rendering-only change.

Actual renderer previews: build/combat-clear-v34.png, combat-clear-sensors-v34.png, explosion-enemy-modal-v34.png and explosion-player-v34.png. These are host framebuffers, not emulator captures. Build: build/v34-build.log; existing SDK executable-stack metadata warning remains. No physical DS validation was performed.

## Release and recovery

Current ROM: ftl-ds.nds. Preserved copy: releases/v34/ftl-ds.nds. Size: 609,280 bytes. SHA-256: acf2ae6121713512a13c5ca2e5ff0a63e0948e8854aa53a6f13692bb41b32c92.

Recovery: checkpoints/v34-ui-visibility.zip is an overlay on the V33 baseline plus V33 launcher update. It contains changed source, tests, preview tooling, documentation, ROM and evidence, with build/v34-manifest.json and a SHA-256 sidecar. The prior V33 launcher ROM remains hash-verified and untouched. Original art stays local.

## Next steps

Play a battle on DS and check the enemy weapons line, then watch both a win and a loss, including a final hit while aiming. Continue the full-game checklist after hardware feedback; authentic Flagship/content and broader controller work remain open.
