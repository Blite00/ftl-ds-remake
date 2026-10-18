# V59 ship achievements and Type B unlocks

Step 5 of the plan, part 3: the 27 base-game ship achievements (3 per cruiser) now unlock the Type B layouts, as in the original (2 of 3).

## Completed
- **Achievement list.** `tools/import_achievements.py` imports each achievement's original name and description. Lanius achievements are Advanced Edition and excluded. Each achievement only counts while flying its own cruiser (either layout).
- **Implemented triggers:**
  - **Kestrel:**
    - six different species aboard at once;
    - 11 systems installed;
    - repair to full hull after dropping to 1 HP. The 1-HP mark stays set for the rest of the run and is saved.
  - **Stealth:**
    - destroy a ship that was at full health when you cloaked, during that same cloak;
    - dodge 9 damage during one cloak;
    - reach sector 8 without visiting a hazard beacon.
  - **Mantis:**
    - kill the crews of 20 ships by sector 6;
    - kill 5 crew in one fight without hull damage or losing a crewmember;
    - your last crewmember kills the last enemy aboard their ship.
  - **Engi:**
    - 3 drones working at once;
    - a kill using only drones;
    - 4 enemy systems or subsystems ioned at once.
  - **Federation:**
    - an Artillery-only kill without taking hull damage;
    - four crew (species) blue options by sector 5;
    - reach sector 5 without upgrading Weapons.
  - **Slug:**
    - see every enemy room without working sensors;
    - 30 nebula-sector beacons before sector 8;
    - 3 enemy crew killed by one Anti-Bio Beam shot.
  - **Rock:**
    - your crew kill a burning enemy aboard its own ship;
    - a missiles-only kill while the enemy has a defense drone deployed. Only the flagship carries defense drones for now.
  - **Zoltan:**
    - a kill before the Zoltan shield breaks;
    - 29 power in systems at once;
    - reach sector 5 without upgrading the reactor.
  - **Crystal:**
    - a kill with a Crystal Vengeance shard;
    - 4 enemy crew locked down in one room;
    - beat 10 Rock ships (pirates count).
- **Tracking.** Each player shot now records where it came from (weapon, missile, drone, artillery, shard or Anti-Bio). Crew deaths, hull hits and cloak dodges are tracked for each fight.
- **Profile.**
  - Earned achievements are stored in the SD profile with its backup. The game shows "Achievement: <name>" when one is earned.
  - Two of a cruiser's three unlock its Type B layout straight away.
  - The hangar lists a locked Type B's three achievements with [X]/[ ] progress.
- **Saves.** The format is now 30. It appends 24 bytes: four run counters, the run flags (weapons or reactor upgraded, hazard visited) and the Kestrel's 1-HP mark. Saves from formats 2–29 still migrate with no progress.

## Not included yet
- **Ancestry** (Rock: find the secret sector) needs the Crystal secret sector from the quest batch.
- **Defense drones on regular enemy ships** don't exist yet, so Rock's missile achievement can only be earned against the flagship.

## Evidence
- **Results:** 140 host checks (new: `v59_achievements_smoke`, covering 26 triggers, class gating, the run-progress save round trip and the profile's Type B unlocks) and 45 Python tests passed. Save tests were updated for the format-30 tail (+24 bytes).
- **ROM:** 43,532,288 bytes. SHA-256 `41c9ced71e6344ad300cca59edea58cee26957a6c0553a70ef5ad2a6077701e3`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,019,564 bytes.
- **Previews inspected:** `v59-hangar-kestrel-b-locked` (1 of 3 earned, silhouette) and `v59-hangar-kestrel-b-unlocked` (Red-Tail ready).

Recovery: releases/v58/ftl-ds.nds. Current ROM: releases/v59/ftl-ds.nds. Snapshot: checkpoints/v59-achievements.zip and build/v59-manifest.json.
