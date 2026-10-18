# V33 enemy ship presentation

## Change

The Interceptor, Armored Drone and authored Rebel Command previously used a rectangular procedural hull and a rectangular room grid. Each now uses an existing locally imported original FTL hull, source room geometry and source doors:

| DS profile | Original hull and layout |
|---|---|
| Interceptor | AUTO_BASIC / Auto-Scout |
| Armored Drone | AUTO_ASSAULT / Auto-Assault |
| Rebel Command | REBEL_FAT / Rebel Rigger |

The established original enemy profiles retain their own hulls. `game_enemy_layout_index` selects presentation and geometry separately from `game_enemy_import_index`, which continues to identify imported behavior profiles.

Rooms, system targets, miniature impact positions, beam endpoints and bottom-screen touch selection use the same source geometry. Source doors also replace the authored variants' previously empty adjacency graph. The Auto-Assault retains its original isolated compartments; this is not a connected rectangular boarding floor.

## Provenance and approximations

Art and geometry come from the existing verified local import in `generated/enemy_catalog.json`, `generated/enemy_catalog.h` and `generated/enemy_catalog_art.h`. No installed FTL files were written. No new art was invented or downloaded. Extracted originals and their derivatives remain local.

The three profiles retain their authored hull scaling, weapon damage/cooldown/volley size, shield/engine/weapon capacities and automated behavior. Systems move to their corresponding source rooms. Optional systems, crew and drones from the reused hull blueprint are not silently enabled. The Command ship remains the authored three-phase finale with a reused Rigger hull; this does not implement the original Rebel Flagship, its crew or its phase mechanics.

The new room geometry and door connectivity can affect beam coverage and boarding paths. These are intentional consequences of replacing the temporary floor plan. Save format is unchanged because enemy encounters are transient and cold loads reconstruct encounters; existing user saves are preserved.

## Validation

- `enemy_hull_smoke`: all eight enemy kinds, all three Command phases; exact source rooms/doors, valid source mappings, authored strengths, visible original hull pixels, every room touch target, rendering purity and framebuffer guards. Passes strict host compilation and undefined-behavior traps.
- Focused room damage, enemy loadouts, combat foundations, boarding and enemy escape regressions pass.
- `room_damage_smoke` now selects an actual room without a system instead of hardcoding room 5, which is absent from Auto-Assault. It still verifies hull-only damage without damaging unrelated systems.
- Battle and aiming framebuffer fixtures: `build/enemy-hull-{0..7}-{battle,aim}-v33.png`. Interceptor, Armored and Command views inspected.
- No physical DS acceptance is established by host rendering/tests. Full integration evidence is recorded in the V33 checkpoint.
