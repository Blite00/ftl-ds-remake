# V55 cloaking and augments

Step 4 of the plan, part 1: the player's cloaking system and the base-game augments. The remaining drones follow in V56.

## Completed
- **Cloaking (store install).**
  - **Buying:** stores offer "Cloaking system (install)" for 150 scrap (offer 15, on the last store page). It installs in the original Kestrel cloaking room.
  - **Upgrades:** on the ship's Upgrade page, the cloak goes to level 2 (45 scrap) and level 3 (60 scrap).
- **Cloaking in combat.**
  - **Controls:** Actions gains CLOAK POWER (left/right or the −/+ buttons) and ACTIVATE CLOAK.
  - **Duration:** each powered bar gives 5 s (5/10/15 s).
  - **While cloaked:**
    - +60% evasion;
    - enemy weapons stop charging (the original behaviour);
    - the Kestrel is drawn faded;
    - the top screen shows `CLOAK nS`.
  - **Firing:** firing a weapon cuts 2 s from the cloak, unless the ship has Stealth Weapons.
  - **Recharge:** afterwards the system is locked (4 ion-style bars, shown as `CLOAK WAIT`).
  - **Damage and repair:** the cloak room takes system damage and ion, crew repair it, and its power counts against the reactor.
- **Augments.** 20 base-game augments in three slots. Duplicates are refused, except for the stackable Scrap Recovery Arm. Each store sells one augment (offer 16, random by store), and events give and test for them. If the slots are full, an event augment is sold for half its cost. Effects:
  - **Scrap Recovery Arm:** +10% scrap.
  - **Automated Re-loader:** faster weapon charge.
  - **Shield Charge Booster:** faster shield recharge.
  - **Weapon Pre-igniter:** weapons start fully charged.
  - **FTL Recharge Booster:** faster FTL charge.
  - **FTL Jammer:** enemies take twice as long to escape.
  - **Adv. FTL Navigation:** you can jump to any explored beacon.
  - **Zoltan Shield:** 5 points of shield at the start of each encounter.
  - **Rock Plating:** chance to ignore hull damage.
  - **Crystal Shards:** when hit, a chance to fire a crystal shard back at the enemy.
  - **Reinforced System Casing:** chance to ignore system damage.
  - **Titanium Ion Armor:** chance to ignore ion.
  - **Engi Med-bot Dispersal:** slow healing everywhere.
  - **Slug Repair Gel:** slowly repairs damaged systems.
  - **Crew Stims:** faster crew.
  - **Stealth Weapons:** firing doesn't shorten the cloak.
  - **Repair Arm:** each scrap gain repairs 1 hull for 15% of that gain.
  - **Drone Recovery Arm:** deployed drones return as parts after a win.
  - **Advanced Scanners:** the enemy interior is visible without sensors.
  - **Drone Reactor Booster:** faster drones.
- **Event hooks.** Augment requirements and rewards from the original events are compiled and enabled.
- **Ship page.** The Upgrade page shows the cloak upgrade button and the installed augments.
- **Saves.** Format 29 appends the cloak level, health, power and room, the three augment slots, and stock for the two new store offers. Saves from formats 2–28 still migrate (no cloak, empty slots).

## Not included
- Advanced Edition augments (hacking, battery and similar) are excluded.
- The Stasis Pod is a quest item with no effect, as in the original.

## Evidence
- **Results:** 136 host checks (new: `v55_cloak_augments_smoke`) and 45 Python tests passed.
- **Fixes from the V55 test run:** the V55 test used an unvisited store beacon, so the save validator correctly rejected it; the cloak message was shortened to fit the status line; augment rows were re-spaced.
- **ROM:** 41,827,328 bytes. SHA-256 `ffa7907cd56679d4c093887b2bd002e9e36a774a1b938e736f72d33bae3ed685`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 1,823,964 bytes.
- **Previews inspected:** `v55-store-cloak`, `v55-store-augment`, `v55-manage-cloak-augments` and `v55-cloaked-combat` (all `-v55`).

## Not yet verified
- **Physical DS:** cloak readability (the faded Kestrel) and timing on hardware.
- **Balance:** augment effects are tuned approximations of the originals.

Recovery: releases/v54/ftl-ds.nds. Current ROM: releases/v55/ftl-ds.nds. Snapshot: checkpoints/v55-cloak-augments.zip and build/v55-manifest.json.
