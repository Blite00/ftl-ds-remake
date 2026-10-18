# V53 original events, batch 1

This is step 3 of Blite's content plan. Later batches will add hazards, quests, stores and boarders.

## Completed
- **Original beacon lists.** New runs (event policy 6) draw beacon events from the original sector beacon lists in `data/sector_data.xml`, by sector type and beacon type:
  - fight beacons draw from HOSTILE_*;
  - supply beacons draw from NEUTRAL_*, ITEMS and NOTHING_*, plus the reviewed station and trade events;
  - distress beacons draw from DISTRESS_BEACON_*;
  - quest beacons draw from QUESTS_*.
- **Which lists each sector uses:**
  - **Civilian:** the standard lists plus the Zoltan lists.
  - **Engi:** the Engi and Rock lists.
  - **Rebel:** the Rebel lists.
  - **Pirate:** the pirate and Mantis lists.
  - **Nebula:** the nebula and Slug lists.
  - **Final sector:** keeps its own Last Stand flow.
- **How events are compiled.** `tools/event_compile.py` compiles each list member on its own and rolls back on failure. A member that uses an unsupported feature is left out of the list instead of breaking it. Every exclusion reason is recorded in `generated/event_coverage.json`.
- **Now supported:**
  - **Enemy ships:** all imported hulls (V52), including pirate pools of up to 16 ships.
  - **Blue options:**
    - system requirements: shields, engines, weapons, oxygen, medbay, sensors, doors, teleporter, drones and pilot, each at the required level;
    - a specific weapon, or any weapon from an original weapon list.
  - **Shown but disabled for now:** blue options that need augments or drones not built yet. They will work once those exist (step 4).
  - **Advanced Edition-only choices:** blue options for hacking, mind control, clone bay, Lanius and backup DNA are left out of base-game events.
  - **Rewards and damage:**
    - `<weapon>` rewards, either a named weapon or a random one of rarity 1–3;
    - every `autoReward` kind except augments. This is an authored DS economy; drone rewards give drone parts until more drones exist.
    - event system damage, plus fires and breaches in a chosen or random room.
  - **Event screen:** up to six choices on the screen, with compact buttons.
  - **No visible effect on the DS:** decorative fleets, and map reveal (the map is always fully revealed here).
  - **Extra source:** `newEvents.xml`, where the generic lists live. Base-file definitions win any duplicate.
- **Coverage.** 57 of 405 named events are enabled, up from 12. That count is the 12 reviewed V29 events, which keep their first slots, plus the playable events from the sector lists. Together they fill 142 beacon-list entries.
  - **Main exclusions:** environmental hazards (48 list members), quests (21), unusual ship outcomes, and events needing more than six choices or long labels.
- **Saves.** Format 28 adds 256 seen-bits for unique events beyond the first 31. Saves 2–27 still migrate. Policies 1–5 keep their historical event sets, so older runs replay unchanged.

## Test bot and test updates
- **Shared fight/service bot.** The original ships are tougher, so the bot now plays more like a player:
  - shoots shields first against two or more layers;
  - always repairs oxygen first;
  - shelters hurt crew in the room with the most air;
  - invests in shields and reactor from sector 4;
  - allows 10 minutes for flagship fights.
  - A crew loss in the final sector now counts as a finale defeat.
- **Encounter mix and scaling.** Rebel and final sectors keep their existing Rebel/Auto ships; the Elite Fighter belongs to the Last Stand lists. Imported ships now scale more gently: +1 system level per 3 sectors and +1 hull per sector.
- **Updated tests:** the save tests (format 28 and the 32-byte tail), the policy tests (new runs are policy 6; historical tests opt into policy 5), the ship and enemy masks, and the compiler unit tests.
- **New test `v53_events_smoke`.** It walks every pooled event: 45 start, 22 lead to fights, and 18 of those use imported hulls. It also checks requirement kinds and the save28 seen bits.

## Evidence
- **Results:** 135 host checks and 45 Python tests passed.
- **ROM:** 41,718,784 bytes. SHA-256 `12bfe4ebebbfce7be0b3eaf64b349706adf08684f6ff086b2249f90c5fe907e9`.
- **Memory:** the static ARM9 union is 1,714,132 bytes. Audio is byte-exact.
- **Preview:** `v53-event-blue`.

## Next
Per Blite's request: an original-style UI redesign (V54), then cloaking, augments and drones, then the hangar and unlocks, then later event batches.

Recovery: releases/v52/ftl-ds.nds. Current ROM: releases/v53/ftl-ds.nds. Snapshot: checkpoints/v53-events.zip and build/v53-manifest.json.
