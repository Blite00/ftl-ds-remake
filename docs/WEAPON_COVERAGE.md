# Weapon coverage register

Run `python tools/weapon_coverage.py` to refresh `generated/weapon_coverage.json` from the owner's read-only installed archive. It does not edit gameplay pools or source assets. The generated file remains local and records source file/hash, every named weapon ID, raw fields, family, explicit selection status and mechanics requiring review.

## Current audited inventory

The installed `data/blueprints.xml` contains **49 named weapon definitions** and **9 unnamed weapon-reference occurrences**. References are recorded separately rather than counted as extra weapons.

| Classification | Count |
|---|---:|
| Selected player catalog | 7 |
| Selected enemy dependency | 1 |
| Not enabled | 41 |

Selected player IDs retain their existing catalog order: `MISSILES_2_PLAYER`, `LASER_BURST_3`, `LASER_BURST_1`, `LASER_HEAVY_1`, `ION_1`, `BEAM_2`, `BOMB_1`. `DRONE_LASER` is separately recognized as the implemented enemy Combat I drone dependency; it is not a purchasable player weapon.

Raw delivery classification gives 13 laser, 5 ion, 14 missile, 10 beam and 7 bomb definitions. Ion is distinguished using its raw ion field even though the source delivery type is LASER. Crystal projectiles retain their source MISSILES classification. Artillery and drone controller requirements are additionally tagged from explicit ID prefixes.

## Remaining coverage work

The register identifies burst queues, shield piercing, fire/breach effects, explicit system/personnel damage, healing, ion lockout, hull multipliers, weapon lockdown and artillery/drone controllers. Some foundations exist, but that does not enable every definition sharing a family. Each unselected entry still needs eligibility, availability, correct parameters/interactions, presentation, equipment economy and save validation before it can enter a pool.

`data/blueprints.xml` is a filename in the installed modern archive, not proof that every entry is historically base-game or obtainable by players. Internal boss/drone entries, rarity zero, post-AE revisions and source stun fields require separate review. The register explicitly leaves historical eligibility and player availability unproven; it does not claim 49 playable base weapons or completion of the original catalog.

Mechanic tags are evidence-driven review requirements, not a fidelity certificate. Full-game completion requires resolving the in-scope catalog and its interactions against the goal, while keeping unsupported entries out of gameplay.
