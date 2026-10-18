# V69 per-weapon targets

## Completed
- **Own targets.** As in the original, every weapon keeps its own target room. This was already true for aiming and autofire, and is now visible and controllable:
  - **Markers:** the aim screen shows a filled, numbered marker in each weapon's target room, gold for the selected weapon. The top-screen overview numbers each target too.
  - **Clearing:** B on the aim screen clears the selected weapon's target. That weapon then holds fire, autofire included, while the others keep firing. A second B leaves the aim screen.
  - **Re-arming:** a held weapon re-arms when you pick a room with the D-pad, a tap or A. Selecting it with L/R or W1–W4 no longer re-arms it on its own.
- **Artillery:** unchanged. In the base game the Federation Artillery Beam fires unaimed at random rooms.
- **Saves:** the format is unchanged (32). Targets are combat-only state.

## Evidence
- **New test:** `v69_weapon_targets_smoke`. It checks that two weapons fire at their own rooms on autofire, that B holds one weapon while the other fires, that buttons and touch re-arm it, and that the markers render. `controller_smoke` now expects the two-step B.
- **Targeted checks passed:** controller, tutorial, crew/subsystem/v10 UI, shot path, alarms, main menu, save v10, finale choice and flagship construction.
- **ROM:** 43,926,528 bytes. SHA-256 `9b233b39a85c902eb01c999851e390d84a4b60dd92a113f6ad92728b61d35d55`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,325,396 bytes.
- **Previews:** `v69-weapon-targets` and `v69-weapon-targets-fight`.

Recovery: releases/v68/ftl-ds.nds. Current ROM: releases/v69/ftl-ds.nds.
