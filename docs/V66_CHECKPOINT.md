# V66 flagship under construction (Federation unlock)

## Completed
- **Event.** The original FLAGSHIP_CONSTRUCTION event (from the REBEL_SECTOR_MINIBOSS sector) is now a Rebel-sector homeworld special. From sector 2 on, it has a 1 in 3 chance per run, at most once, to sit unmarked at an ordinary beacon.
  - **Winning:** destroying the half-built flagship, or killing its crew, plays FLAGSHIP_CONSTRUCTION_DONE. That event gives a high weapon reward, 5 fuel, 5 missiles and 5 drone parts, delays the fleet by 2, and unlocks the Federation Cruiser (`unlockShip id 4`).
  - **Result:** all eight unlockable cruisers can now be unlocked the original way.
  - **Saves:** the special is appended last, so earlier specials keep their saved bits. The format is unchanged (32).
- **Ship.** The original BOSS_SPECIAL blueprint is imported (boss_3 hull, 10 hull, shields 2, teleporter, crew of 3). It is quest-only.
  - **Deviation:** its two power-1 boss artillery mounts (ARTILLERY_BOSS_1/2) are not run by the generic enemy setup, and the blueprint has no weapons system, so the hull fights without guns. Its boarding party can still teleport aboard.
  - **Importer:** the enemy importer now also reads `data/blueprints.xml`, and falls back to `img/ship/` art for boss hulls, which have no glow variant.
- **Events:** 161 enabled.

## Evidence
- **Results:** 132 quick host checks passed (new: `v66_flagship_construction_smoke`: Rebel special, BOSS_SPECIAL hull, the win unlocking layout 8), plus 46 Python tests and the relevant campaign run (`surrender_run_smoke`).
- **ROM:** 43,776,000 bytes. SHA-256 `b9fb59af2bab4e1643f3e18f5cb460bb11f21a2e8840c9bf4f3456b62823258d`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,285,268 bytes.

Recovery: releases/v65/ftl-ds.nds. Current ROM: releases/v66/ftl-ds.nds. The final package (`releases/final`, zip) has been updated to V66.
