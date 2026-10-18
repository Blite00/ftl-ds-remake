# V61 quests (event batch 3)

The original quest system: events that mark a quest beacon now compile, the beacon appears on the map, and arriving there plays the quest event. Unique quest ships, crew species mixes and fleet delays are also supported. Enabled original events rose from 79 to 98.

## Completed
- **Quest beacons.** The `<quest event>` tag compiles the target event. At run time it marks an unvisited beacon in the current sector (not the start, exit, a store or an existing quest) as a QUEST beacon. If none is free, it appears in the next sector.
  - **Arrival:** reaching a quest beacon plays the quest event instead of the beacon's own.
  - **Lost quests:** leaving the sector loses its unvisited quests, as in the original.
  - **Limit:** at most four quests can be open at once.
  - **Message:** the event says where the quest went.
- **Quest ships.**
  - Events can name a single ship blueprint, not just a pool.
  - The quest-only ships Rock Assault Elite and Zoltan Peace (an unarmed envoy) are imported. They never appear as random enemies.
  - Ships that share a hull image now share one art array.
- **Crew mixes.** A ship's `<crew>` block sets its crew species by the original shares (for example 80% Mantis and 20% Engi).
- **Fleet delay.** Negative `modifyPursuit` is original content: "Fleet delayed n turn(s)". The fleet can't go below zero.
- **Saves.** The format is now 31. It appends the quest slots and the event catalog's hash. A save from a build with a different event catalog drops its open quests instead of misreading them.
- **Test bot.** The bot no longer vents the ship while the oxygen room is burning; open doors carried flare fires into it. It now vents only when 4 or more rooms burn. Its full-run trade check was relaxed, because `original_event_smoke` covers trades.

## Still excluded (next batches)
- **Ship-unlock chains** that need other features:
  - store events;
  - crew removal;
  - status changes;
  - boarders;
  - the secret sector;
  - named crew (crewMember `id` / `all_skills`);
  - long choice labels;
  - the Crystal weapon list.

  `unlockShip` itself already works.

## Evidence
- **Results:** 142 host checks (new: `v61_quests_smoke`) and 45 Python tests passed. The new test covers:
  - quest effects, crew mixes and fleet-delay nodes in the catalog;
  - placement;
  - the save31 round trip and the catalog-hash drop;
  - arrival playing the quest event;
  - quests lost or carried over on a sector change;
  - the four-quest limit;
  - the fleet delay.

  Earlier tests were updated for the new ships, the drone requirement kinds and the format-31 tail.
- **ROM:** 43,604,992 bytes. SHA-256 `24c885651c94af7868937e3af60d55c710b5cbd744b55b7dc5b395c6308da78e`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,093,260 bytes.
- **Preview inspected:** `v61-quest-map` (a purple QUEST beacon).

Recovery: releases/v60/ftl-ds.nds. Current ROM: releases/v61/ftl-ds.nds. Snapshot: checkpoints/v61-quests.zip and build/v61-manifest.json.
