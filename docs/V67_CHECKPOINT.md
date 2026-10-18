# V67 construction artillery

## Completed
- **Flagship under construction now shoots.** BOSS_SPECIAL's two power-1 artillery mounts are set up from the blueprint and reuse the existing artillery code with their original stats:
  - ARTILLERY_BOSS_1: a 3-shot laser;
  - ARTILLERY_BOSS_2: 3 missiles.
- **Crew:** outside the Rebel flagship, artillery needs no crew. The original blueprint has no weapons system, so its guns fire on their own. Its boarding party still teleports aboard.
- **Saves:** the format is unchanged (32).

## Evidence
- **Results:** 132 quick host checks passed. `v66_flagship_construction_smoke` now also checks both mounts: the right weapons, power 1, ready to fire, and damage to an unshielded player within 60 s.
- **ROM:** 43,776,000 bytes. SHA-256 `273a1cb6923c4890ff58e8eff0005a8a992098f090eb4f4dcdbf2eff632420fb`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,285,316 bytes.

Recovery: releases/v66/ftl-ds.nds. Current ROM: releases/v67/ftl-ds.nds. The final package has been updated to V67.
