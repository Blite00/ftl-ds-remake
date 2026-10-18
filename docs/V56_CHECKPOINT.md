# V56 remaining drones

Step 4 of the plan, part 2: every base-game player drone now works. Before V56 only Combat I and Defense I did.

## Completed
- **Drone list.** The imported catalog now has 9 drones, with their original names, power, cost and rarity. Combat I and Defense I keep their save ids.
  - **Combat Drone Mark II** (power 4): fires a laser every 6 s instead of 10 s. The original moves almost twice as fast as Mark I (speed 28 vs 15).
  - **Defense Drone Mark II** (power 3): shoots down missiles, lasers and ion shots. Its cooldown is 88% of Mark I's (880 vs 1000 in the original).
  - **Anti-Ship Beam Drone** (power 2): every 10 s it sweeps a short beam across two neighbouring enemy rooms, doing 1 damage per room. As in the original, any shield layer stops it.
  - **Hull Repair Drone** (power 2): repairs 1 hull every 2 s, 3–5 hull per drone part. Then it is spent and needs a new part.
  - **System Repair Drone** (power 1): moves to the first damaged system and repairs it at crew speed (5 s per bar). It needs no oxygen. Intruders in its room attack it.
  - **Anti-Personnel Drone** (power 2, 150 HP): hunts intruders aboard and fights them at human strength. They fight back.
  - **Boarding Drone** (power 3, 150 HP): flies for 3 s, then breaches a random enemy system room. It fights any crew in that room, otherwise sabotages the system at crew speed. When that system is destroyed it moves to another system room. Suspending it loses the drone, as in the original.
- **Display.** Interior and boarding drones show as a small cyan drone body in their room:
  - green dot: repair drone;
  - red dot: anti-personnel drone;
  - yellow dot: boarding drone, shown on the enemy ship.

  The drone panel shows short names and what each drone is doing: hull repairs left, repairing, guarding or boarded room, and in-flight. It also shows a health bar for drones that can be damaged.
- **Store.** The second drone offer (offer 14) is now a store-seeded drone schematic matching the store's rarity, instead of always Defense I. Combat I is still always sold. Runs on the legacy fixed route (route policy 0) keep Defense I.
- **Events.**
  - Original drone rewards now give a drone schematic in drone cargo, sold for half price if cargo is full, instead of drone parts. 10 outcomes in the enabled events changed.
  - Drone blue options (System Repair, Anti-Personnel, Boarding, Hull Repair and the drone lists) now check the drones you own instead of always failing.
- **Build.** `build.ps1 -FtlArchive` now runs the weapon/augment import and the drone import before the event compiler that depends on them. `tools/drone_coverage.py` writes `drone_catalog.h` and `.json`.
- **Saves.** The format is unchanged (29). Drone ids up to 8 are valid. Unit state lasts only for the encounter and is cleared on load.

## Not included
- Advanced Edition drones are excluded: Anti-Combat (anti-drone), Fire Beam, Ion Intruder and Shield drones. The "anti-drone" on the plan list is AE-only.
- Drones work only during combat, as before. In the original, the internal and hull repair drones also work between fights.
- Enemy defense drones don't shoot down the player's boarding drone yet.
- Interior drones move room to room on a timer rather than walking door paths.

## Evidence
- **Results:** 137 host checks (new: `v56_drones_smoke`) and 45 Python tests passed.
- **ROM:** 41,832,448 bytes. SHA-256 `3f0d645e18e019c04dd81401c30f4eaef1418826326f8bcf44da17abf4487839`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 1,828,700 bytes.
- **Previews inspected:** `v56-store-drone`, `v56-interior-drones`, `v56-boarding-drone` and `v56-drone-panel` (all `-v56`).

## Not yet verified
- **Physical DS:** how readable the 6 px drone bodies are on hardware.

Recovery: releases/v55/ftl-ds.nds. Current ROM: releases/v56/ftl-ds.nds. Snapshot: checkpoints/v56-drones.zip and build/v56-manifest.json.
