# V62 event features and ship-unlock quests (event batch 4)

The remaining common original event features, the homeworld "special" events that start the ship-unlock chains, and taller choice boxes for long original labels. Enabled original events rose from 98 to 130.

## Completed
- **Store events.** `<store/>` turns the beacon into a store. After the event, Continue opens the store, and revisits open it too.
- **Crew loss.** `<removeCrew>` removes a random crewmember aboard, never the last one. The base game has no clone bay.
- **Boarders.** `<boarders min max class>` spawns intruders of that species aboard; "random" picks the species at random.
  - **With an enemy ship:** they beam in when the fight starts.
  - **Without one:** they appear at once and are fought during ship management.
  - **Excluded:** "Ghost" boarders are not base-game species, so those events stay excluded.
- **Status effects.** `<status type target system amount>` sets system power limits (limit / divide / loss / clear) on your ship or the enemy until you jump, for the main systems, sensors and doors.
  - **Your ship:** power is cut at once, and adjusting power refuses to exceed the cap.
  - **The enemy:** caps are applied when the fight starts.
  - **Ignored:** hacking, clone bay, mind control and battery (Advanced Edition) and pilot statuses are no-ops.
- **Named crew.** `crewMember id / all_skills` gives story characters with their original names and skill level, for example Kazaaak (the Mantis thief) or the Zoltan Envoy.
- **Long choice labels.** Choice boxes now use the space the choices leave:
  - 1–2 choices: 56 px, 6 rows;
  - 3 choices: 4 rows;
  - 4–5 choices: 3 rows;
  - 6 choices: 2 rows.

  The compiler's limit (`CHOICE_ROWS`) matches. 26 original events that were rejected for long labels now fit.
- **Homeworld specials (ship-unlock chains).** The original `*_HOME` sector descriptions place a story event that starts each unlock chain:
  - `ENGI_UNLOCK_1` → Stealth;
  - `ROCK_UNLOCK1` → Rock;
  - `MANTIS_NAMED_THIEF` → Mantis;
  - `NEBULA_SLUG_FIGHT_UNLOCK` → Slug;
  - `ZOLTAN_PEACE_QUEST` → Zoltan.

  The DS has one sector type per people group, so from sector 2 on, each matching sector has a 1 in 3 chance per run (at most once) to be that people's homeworld. The special then sits, unmarked like in the original, at an ordinary beacon (never the scripted first-fight, supply or repair beacons). Its roll and beacon come from a hash of the seed and sector, so the run's random sequence is unchanged. All five chains compile, along with their `unlockShip` effects.
- **Saves.** The format is now 32. It appends the unmarked-quest flags and the homeworld specials already placed this run. Older saves still load.
- **Fix (weapon catalog).** Re-running the enemy importer used to overwrite `weapon_catalog.json` with the old 7-weapon list, so V61's event compile rejected rewards and requirements for later weapons. Nothing was mis-mapped (fail-closed), but fewer events were enabled. Now:
  - only `import_weapons.py` writes it, and `build.ps1` runs that importer before the enemy importer;
  - the legacy weapon audit checks the original 7 as a prefix.
- **Hazard balance.** A solar flare now starts one fire on each ship (was 1–2).
- **Test bot.**
  - It jumps away below 10 hull when the FTL drive is ready.
  - `surrender_run_smoke` now tolerates at most two ordinary (non-flagship) losses from sector 3 on among its 12 fixed-seed runs. The original boarder and hazard events can defeat a scripted crew that doesn't use doors or venting. One loss occurred in this run.

## Still excluded
- **Crystal secret sector** (`<secretSector/>`): in progress as a separate patch.
- **Flagship construction quest:** its special boss ship blueprint is missing.
- **Smaller tags:** `upgrade`, `damage all`, unique text lists, choice `max_group`/`max_lvl` (Advanced Edition), `item_modify steal`.
- **Anti-ship battery (PDS) events:** Advanced Edition.

## Evidence
- **Results:** 143 host checks (new: `v62_event_features_smoke`) and 45 Python tests passed. The new test covers status limits and their clearing on a jump, intruders fought in ship management, and real catalog events for a store, a crew loss and boarders. Save tests were updated for the format-32 tail.
- **ROM:** 43,699,200 bytes. SHA-256 `b16aeee56e6d58cf79a6ad42f18b39cbd9da730e2d02630e840bb609b57b8906`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,187,644 bytes.
- **Previews inspected:** `v62-long-choices` and `v62-intruders`.

Recovery: releases/v61/ftl-ds.nds. Current ROM: releases/v62/ftl-ds.nds. Snapshot: checkpoints/v62-events.zip and build/v62-manifest.json.
