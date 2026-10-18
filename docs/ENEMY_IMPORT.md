# Original enemy imports

`tools/import_enemy.py` also runs `tools/import_enemy_catalog.py`. The importer opens the installed FTL archive for reading and writes only beneath the requested output directory. Existing legacy Scout headers are retained for compatibility.

## Imported catalog

| Index | Blueprint | Original class | Base hull | Required supported capability |
|---|---|---|---:|---|
| 0 | AUTO_BASIC | Auto-Scout | 6 | Existing laser combat; optional cloak unselected |
| 1 | AUTO_ASSAULT | Auto-Assault | 8 | Offensive combat drone |
| 2 | REBEL_SKINNY | Rebel Fighter | 10 | Living human crew |

The first two are the zero-crew ship blueprints in the base-game `autoBlueprints.xml`. No Advanced Edition or crewed hull has been relabeled as an automated opponent. The Assault's starting drone system is retained in the data and must work before the ship enters a playable pool.

### Imported facts

- Exact room and door geometry, system placement, initial system power, upgrade ceilings, reactor, hull and ammunition/parts quantities.
- Original class names from the localization table.
- Original hull PNGs and their ship XML bounding boxes.
- Original weapon and drone pool membership, retained in the JSON audit.
- The Assault's fixed selected `COMBAT_1` is a member of `DRONES_COMBAT`. Its source fields are power 2, raw cooldown 1000, speed 15 and dodge 0. Its `DRONE_LASER` weapon is one shot for one damage, fire chance 1 and breach chance 0.

The raw drone cooldown is source data, not evidence of an actual one-second firing interval. Drone movement, acquisition and firing timing need an explicit implementation policy. Fire chance is preserved in the audit; a runtime implementation must state how it handles this property while fire mechanics are incomplete.

### Intentional conversion and runtime boundaries

Each hull is downscaled with Lanczos into a centered 90 by 65 canvas. Pixels below alpha 96 become transparent; remaining pixels become RGB555 with bit 15 set. Total bitmap payload is 35,100 bytes for three hulls. Original source dimensions and the occupied scaled rectangle are exported so room markers use the same hull coordinate transform.

The catalog preserves optional Scout shields/cloaking and the Assault's weapon power zero. It does not choose unsupported optional systems, reproduce original difficulty scaling, select entire random equipment pools, or implement simulation. Any runtime choice of fixed equipment, scaled hull/power or firing period is a documented demake choice. Required-capability checks belong to encounter selection.

## Generated interfaces and evidence

- `generated/enemy_catalog.h`: three bounded geometry/system records, capability flags and the original combat drone fields.
- `generated/enemy_catalog_art.h`: three RGB555 bitmaps, pointer table and scaled occupied rectangles.
- `generated/enemy_catalog.json`: source facts, full pool membership, conversion policy and SHA-256 for every extracted source file.
- `generated/imported_auto_basic.png` and `generated/imported_auto_assault.png`: exact converted asset previews.

Source payloads are checked against their just-written copies. Room count is limited to 32, door count to 48, room identifiers are unique and all door/system references are checked. Repeated generation was compared for deterministic output; both converted hull previews were visually inspected. Asset conversion and data validation do not establish gameplay or DS performance fidelity.

Original extracted data, generated headers, previews and ROMs remain local under the project's ignored `generated/` and release paths. Do not redistribute these original assets as a source release.

## V11 runtime integration

Assault is enabled from sector two with original base geometry/system placement. Hull uses base8 +2 per zero-based sector. Its zero-power weapon system stays unarmed. One Combat I drone is deployed per encounter, using the imported2-bar requirement and one-damage shot. A fixed10-second firing period and interpreting source fireChance1 as10percent are demake choices. Shields/evasion intercept these shots through the common projectile path. Damage to DRN reduces control power immediately; charge resets while disabled, and automated repair restores one bar every12seconds. There is one deployment and no in-battle drone replacement in this subset; the imported parts stock is sufficient and not a persistent player resource. Player drone control and other drone types remain unimplemented.

Fire behavior is explicit: up to four fires per room,25 oxygen consumed per fire/second,3 crew HP damage per fire/second, one system bar per3seconds, one extinguished fire per3 crew-seconds, vacuum extinction below10percent oxygen, and10percent spread attempts each second through open player doors. Fires do not spread on automated enemy ships because they have no oxygen. Fire hull damage, breaches and richer hazard rules remain later work.

## V12 living Rebel Fighter import

Index 2 preserves original `REBEL_SKINNY`, localized as Rebel Fighter, using `rebel_long` layout and hull art. The original roster starts with three humans, with a source maximum of six. Seven rooms and eight doors include a starboard airlock. Its source hull is 10 and reactor is 7. Oxygen is room 2, pilot room 0, shields room 6, engines room 5 and weapons room 4.

The medbay in room 3 and teleporter in room 1 both have `start=false` in the blueprint. The supported fixed encounter explicitly selects the optional medbay for living-crew healing; it leaves the unsupported teleporter unselected. The catalog preserves the original optional flags rather than rewriting the blueprint.

The fixed laser `LASER_BURST_1` is verified as a member of the original `WEAPONS_REBEL` pool. Its imported localized title is Defense Laser Mark I; values are 10-second cooldown, one damage, one shot, power 1, no missile consumption or shield piercing, fire chance 1 and breach chance 0. The source pool's beams, bombs and other unsupported equipment remain excluded. The separately exported `imported_rebel_weapon` preserves these values. Crew AI, equipment selection, reactor allocation and sector scaling are runtime policies rather than imported behavior.

`ENEMY_CAP_CREW` (2) gates this hull. `ImportedEnemy` now includes trailing `crew_count`, `crew_max` and `crew_species` fields. Automated indices 0 and 1 are unchanged. The converted `generated/imported_rebel_long.png` was visually inspected; the narrow vertical original silhouette is intentionally preserved in the shared canvas.

## V13 optional Rebel boarding loadout

ENEMY_REBEL_BOARDER now selects the original Fighter's optional teleporter in room 1. It shares the same imported geometry, art, three-human roster and laser as the ordinary Fighter, which leaves teleportation unselected. Runtime power allocation, launch/recall tactics, melee and cooldown timings are documented demake choices in COMBAT.md. No additional original assets are needed for this loadout.
