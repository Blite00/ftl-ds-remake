# V22 selection update

Two player drones are selected: Combat I and Defense I. Combat I also remains an enemy dependency; twelve other definitions stay disabled. generated/drone_catalog.h imports both names, power 2 and cost 50 directly from local XML. Deterministic output evidence is in build/v22-import-check.json. Defense recharge is an authored one-second rule; raw cooldown 1000 is retained without inferring its unit. Missile interception is instantaneous with earliest-arrival selection; turning, blind spots, moving orbits and vulnerability remain unimplemented. No enemy defense pool is enabled.

# V21 runtime selection update

Combat I is now selected for the authored player store package and schematic offer, as well as the existing Auto-Assault dependency. Thirteen other definitions remain disabled. The register records selection, not complete original movement/vulnerability fidelity. Player Combat I uses fixed ten-second attacks; store package85 with two parts, schematic50, parts3for24 and upgrade20+10*capacity are authored balance choices. Drone cargo currently has four separate slots alongside weapon cargo; shared original inventory limits remain to reconcile.

The earlier source audit below remains provenance evidence. Current selection and register hash are recorded in build/v21-drone-import-check.json.

# Drone coverage register

Run `python tools/drone_coverage.py` to regenerate `generated/drone_coverage.json` from the owner’s read-only installed archive. This audit changes no runtime catalog, random pool, store or save. Generated original content must remain local.

## Audited source

The installed `data/blueprints.xml` contains **14 named drone definitions** and no unnamed drone-reference occurrences. The audit records every definition, its localized title, raw fields, full nested data, dependent weapon definition, review requirements and explicit selection status. It also retains the complete drone-control system definition and upgrade-cost sequence. Both blueprint and localization file hashes are recorded.

| Source type | Count | IDs |
|---|---:|---|
| COMBAT | 5 | COMBAT_ION, COMBAT_MISSILE, COMBAT_1, COMBAT_BEAM, COMBAT_2 |
| DEFENSE | 4 | DEFENSE_1, DEFENSE_2, DEFENSE_2_ENEMY, BOSS_DEFENSE_2 |
| SHIP_REPAIR | 1 | SHIP_REPAIR |
| REPAIR | 1 | REPAIR |
| BATTLE | 1 | BATTLE |
| BOARDER | 2 | BOARDER, BOARDER_BOSS |

Only **COMBAT_1** is marked `selected_enemy_dependency`, reflecting the existing Auto-Assault implementation. The other **13** are `not_enabled`, and all entries have `player_enabled: false`. This audit is a V20-baseline inventory; any later player implementation must update selection only after its gameplay integration and verification. No entry is enabled by running this tool.

The current enemy Combat I uses imported damage and weapon properties with an authored ten-second firing interval. The original raw drone cooldown is `1000`; that field does not establish the engine’s firing period or orbital behavior. Existing enemy support is not proof of full original drone fidelity.

## Data and behavior boundaries

- Raw XML attributes, child order, repeated children and text are preserved. Missing fields stay absent/null rather than becoming invented zero values.
- Power, cost, cooldown, speed, dodge, defense target/level and weapon references are recorded as source data. Timing units, defaults, orbital movement, target priorities, drone vulnerability and deployment rules still need behavior verification.
- Drone control has source startPower `2`, maxPower `8`, cost `60`, and upgrade entries `10, 20, 30, 45, 60, 80, 100, 120`. The audit preserves their sequence without asserting which level each entry purchases or what an installation package includes.
- Mechanic tags are review requirements, not claims that the mechanics work. All types require installation/power, parts/deployment, equipment management and save/encounter lifecycle review. Combat drones additionally require targeting and weapon delivery; defense drones require interception; hull repair needs its finite repair lifecycle; interior drones need movement and role-specific actions; boarders need delivery, breach and combat/sabotage handling.
- The modern file name `data/blueprints.xml` does not prove historical base-game eligibility or player availability. Internal variants and rarity-zero definitions remain unreviewed. No claim is made that all 14 belong in the playable base-game roster.

## Verification

Focused verification on October 3, 2026 passed:

- Two generations produced identical bytes.
- All 14 raw drone records and every referenced weapon matched parsed source definitions.
- Exact selection statuses, absent cooldown preservation for REPAIR, Defense II’s LASERS target and the complete upgrade-cost sequence were checked.
- Output aimed at the installed directory, a descendant, or an ancestor containing the archive was rejected before writes.
- The entire installed archive SHA-256 was unchanged before and after generation: `d7c4744f91f96303c0342b30c2dcb63843050e0f489426200c683d02fb8abc7d`.

Initial generated register SHA-256: `e87120e6de1e3b425a70b484fdd48df4095ecb4edc27a34d2dd5a29f4d4e2e6b`.

This is source-audit evidence. It does not validate new gameplay, DS performance or hardware behavior. The remaining full-game work includes reviewed playable drone coverage, control and inventory integration, each lifecycle and interaction, save migration, presentation and hardware acceptance.
