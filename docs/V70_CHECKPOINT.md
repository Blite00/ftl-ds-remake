# V70 damage audit and hit flash

## Damage audit (original FTL rules, weapon data from ftl.dat)
| Case (crewed shields room, 4 bars, two humans in the room) | Hull | Bars | Crew HP each, before V70 | Crew HP each, V70 / original |
|---|---|---|---|---|
| Burst Laser II volley (3 x 1 dmg), no shields | -3 | -3 | 85 | 55 (-45) |
| Burst Laser II volley, 2 shield layers (2 blocked) | -1 | -1 | 95 | 85 (-15) |
| Artemis missile (2 dmg, pierces shields), no shields | -2 | -2 | 90 | 70 (-30) |
| Artemis missile, 2 shield layers | -2 | -2 | 90 | 70 (-30) |

- **Fixed: crew damage.** Direct hits did 5 crew HP per damage point. The original does 15 per point, which bombs, crystal shards and the beam drone already used here. All weapon, drone and flagship sources now use 15, and beams lose 15 crew HP per shield layer. Crew in other rooms are never touched.
- **Already correct:**
  - 1 hull and 1 system bar per point (sysDamage/persDamage weapons keep their own values);
  - each projectile applies once, with no double counting;
  - one shield layer stops one laser, and missiles pierce;
  - per-weapon fire and breach chances.
- **Rates, unchanged:**
  - fire: 2 HP/s per fire (at most 6);
  - suffocation: 5 HP/s below 10% oxygen;
  - human melee: 6 HP/s;
  - health: human 100, Rock 150, Zoltan 70, Crystal 125.
  None of these kills a 100 HP crewmember instantly.
- **Why systems seem to drop fast:** a Burst Laser II volley removes 3 bars, as in the original. Two-bar systems such as a scout's shields go down to one volley.

## Hit visibility
- **The hit room:** it flashes red for about 0.75 s with a floating "-N" damage tag, on your ship's schematic (bottom screen) and on the enemy's visible rooms.
- **Top screen:** the overview outlines and blinks the same room on both ships.
- Display-only state; the save format is unchanged (32).

## Evidence
- **New test:** `v70_damage_audit_smoke` (the table above, flash timing, crew outside the room untouched).
- **Preview:** `v70-hit-flash`.
- **Results:** 136 quick host checks passed. The long bot campaign runs were skipped this round; crew now take the original damage, so they may need re-tuning later.
- **ROM:** 43,926,528 bytes. SHA-256 `68ec02cd47bd429279451d509af6f797fd3dfc32b2b4ebd2d24c0991ba02c609`. Audio is byte-exact.

Recovery: releases/v69/ftl-ds.nds. Current ROM: releases/v70/ftl-ds.nds.
