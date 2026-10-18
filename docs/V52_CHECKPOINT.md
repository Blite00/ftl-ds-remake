# V52 original enemy ships

This is step 2 of Blite's content plan.

## Completed
- **36 original enemy hulls.** `tools/import_enemy_catalog.py` now imports 36 base-game enemy hulls. Each has its original rooms, doors, systems, hull, reactor, crew count and species, weapon list, and hull art. The 32 new ones are:
  - Rebel Elite Fighter;
  - Slug Interceptor, Light-Cruiser and Assault;
  - Rock Scout, Fighter and Assault;
  - Mantis Scout, Fighter and Bomber;
  - Federation Scout and Bomber;
  - Engi Scout and Bomber;
  - Zoltan ("Energy") Fighter and Bomber;
  - Crystal Scout and Bomber;
  - the 14 base pirate variants from `dlcPirateBlueprints.xml`. The `_DLC` versions use Advanced Edition systems and are excluded.
- **New `ENEMY_SHIP` encounter type.** It builds a fight from the imported data:
  - **Systems:** starting systems gain one level every two sectors, up to the blueprint maximum. Optional systems (cloaking, teleporter, drones, medbay, doors) are fitted with a 12% chance per sector.
  - **Systems now on regular ships:** cloaking, boarding teleporters, combat drones, and door and sensor subsystems, which were previously limited to the flagship and the Rebel boarder.
  - **Crew:** the blueprint count plus one every three sectors, up to its maximum, all of the ship's species. They are placed at the pilot, weapons, shields and engines stations.
  - **Weapons:** two weapons from the ship's own original list (for example WEAPONS_MANTIS or WEAPONS_CRYSTAL), within the weapon system's power and the sector's rarity limit.
  - **Hull:** the blueprint hull plus 2 per sector.
- **Sector pools.** From sector 2, current runs meet an original ship 60% of the time, or 30% in Rebel and final sectors. The ships are those of the sector's peoples:
  - **Civilian:** the original SHIPS_CIVILIAN list plus the Zoltan fighter.
  - **Engi:** Engi, Rock and Zoltan.
  - **Pirate:** the pirate variants and Mantis.
  - **Nebula:** Slug, Mantis, Zoltan and Crystal.
  - **Rebel and final:** the Elite Fighter.
  - The other encounters keep the existing kinds, and legacy runs are unchanged.
- **Original quirks kept.** For example, the Slug Interceptor's engine and oxygen rooms have no doors.
- **Rendering.** The aim view, enemy doors, room icons and crew sprites all work on every hull without per-ship code.

## Evidence
- **New test `v52_ships_smoke`.** For every new ship at sectors 0, 3 and 6 it checks:
  - setup succeeds and the weapons fit the weapon system;
  - every crewmember has the ship's species;
  - the weapons room can be reached from the pilot;
  - both views render;
  - the ship fires at an idle player within 60 s (sector 3).
  - Across the set, cloaking, teleporters and drones all appear, and all seven species are seen.
- **Updated test:** `sector_routes_smoke` checks that each sector type's ship comes from that sector's peoples.
- **Results:** 134 host checks and 45 Python tests passed. The eight-sector bot campaigns still complete.
- **ROM:** 41,636,864 bytes. SHA-256 `9ef01ae0df0fa585b9ebc36a7305da345a2cd615f10eed240d63cd28a54cb8f8`.
- **Memory:** the static ARM9 union is 1,631,948 bytes (+421 KB of hull art) of the 4 MiB main RAM. Audio is byte-exact.
- **Previews:** `v52-ship-{slug,rock,mantis,engi,zoltan,crystal,pirate,fed}`.

## Not yet verified
- **Balance:** the sector scaling of systems, crew and optional systems is authored, and hasn't been checked against the original's level tables.
- **Sector types:** there are no Mantis, Rock, Slug or Zoltan home sectors yet. Their ships appear in the nebula, civilian and Engi pools.
- **Physical DS:** not tested on hardware.

## Next
Step 3: original events in batches.

Recovery: releases/v51/ftl-ds.nds. Current ROM: releases/v52/ftl-ds.nds. Snapshot: checkpoints/v52-enemy-ships.zip and build/v52-manifest.json.
