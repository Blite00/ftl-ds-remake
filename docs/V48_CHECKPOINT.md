# V48 replacement flagship gunners and a station-manning audit

Blite chose option A, with a change: a dead gunner still silences their artillery room, but another surviving flagship crew member can take the room over.

## Completed
- **Replacement gunners.** `artillery_reman` in `source/game.c` runs every combat tick. It handles each artillery room that has no living crewmember standing in it.
  - **Who replaces:** any living flagship crewmember who is aboard and isn't already in or heading to an artillery room. A crewmember already reserved for another room can't be picked twice. Gunners never leave their own room, and the pilot is only chosen when nobody else is left.
  - **Reachable rooms:** if the replacement has a door path, they walk there through doors as usual.
  - **Sealed rooms:** the original flagship artillery rooms have no doors, so the replacement transfers in after a visible 10 s delay (`GAME_ARTILLERY_REMAN_TICKS`). They stay in their current room until the delay ends, then appear in the artillery room as its new station.
    - The delay is reset if the replacement dies, is sent aboard your ship, or ends up in another artillery room. The next tick picks someone new.
    - **Why not add a door:** a door would change the original flagship layout and let your boarders and fire travel between rooms that are sealed in the original. The delay keeps the original layout.
- **When the room stays silent:** only when no eligible flagship crew are left. As before, the AI-takeover ship (no crew at all) counts as manned.
- **What you see:**
  - With sensors, the enemy weapon line shows `<kind> MAN<n>` (for example `LAS MAN8`) while a replacement is on the way, then the usual charge time once they arrive.
  - On the aim screen, the artillery room blinks gold during the transfer, and `UNMAN` is shown when nobody can replace the gunner.
  - New previews: `v48-artillery-reman-v48.png` and `v48-artillery-reman-aim-v48.png`.
- **Balance:** `flagship_final_smoke` scores the strong DS build at 10/9/10 stage wins with TUNED, the same as V47. The authentic-normal row went from 9/0/0 to 8/0/0. In the eight-sector runs, flagship stage wins went from 5 of 11 engagements to 4 of 10.
- **Saves:** no format change. The transfer timer is encounter-local, and a resumed flagship phase restarts with full crew counts as before.

## Tests
- `v46_combat_smoke` covers the replacement:
  - a dead gunner's sealed room has no door path;
  - the countdown starts at 10 s, and the room is manned again after about 10 s by another crewmember, with no crew lost;
  - all other artillery rooms stay manned;
  - with only gunners left, the room stays silent and no transfer starts.
- **Test bot changes** (`campaign_v10_smoke`, used by the eight-sector runs): one seed now hit a ship-wide vacuum.
  - The healthiest crewmember now repairs a broken oxygen or medbay system.
  - Boarders left aboard after a fight are fought during ship management.
  - The gunner leaves an airless weapons room.
- **Results:** 132 host checks and 45 Python tests passed (build/v48-check.log).
- **ROM:** 41,004,032 bytes. SHA-256 `9c2c78c66029c5d4f1f324a37f7fbabfe85cc1fb1d4155eb94b42c71e2521212`. The only build warning is the existing SDK one.
- **Memory:** the static ARM9 union is 998,004 bytes. Game is 19,488 bytes and Campaign 23,780 bytes. All 17 audio files are byte-exact.

## Which rooms need crew today
This is what the code does at V48.
- **Required** means the system stops working without crew.
- **Bonus** means it works unmanned, and crew only improve it.
- Manning always means a living crewmember of the ship's own side, standing in the room with that room as their destination.

### Player ship
| Room | Without crew | With crew | Code |
|---|---|---|---|
| Pilot | **Required:** evasion is 0 and FTL does not charge | Evasion +5 to +15 (rises with pilot skill) | `game_evasion` game.c:171, `game_player_ftl_operational` game.c:1346 |
| Engines | Works: 5% evasion per bar, if piloted | Bonus +5 to +15 evasion | game.c:171, `game_station_bonus` game.c:161 |
| Weapons | Charges, but **cannot fire** (V46 house rule) | Fires; charge +5 to +15% | `game_weapons_manned` game.c:1327, game.c:1419 |
| Shields | Works and recharges | Bonus: recharges about 20% faster | game.c:1411 |
| Sensors | Works at its level | Bonus: +1 level (V46) | `game_sensor_level` subsystems.c:23 |
| Oxygen, medbay, doors, drones, teleporter | Work without crew | No manning effect. The medbay heals whoever is in it. | `crew_healing` game.c:431 |

Crew in any room also repair its system and fight its fires and breaches. Pilot, engines, weapons and shields also train that skill.

### Enemy ship (crewed)
| Room | Without crew | With crew | Code |
|---|---|---|---|
| Pilot | **Required:** evasion is 0 and the ship cannot jump away | +5 evasion | `hit` game.c:1016, `game_enemy_escape_ready` game.c:1360 |
| Engines | 5% evasion per bar, if piloted | +5 evasion | game.c:1016 |
| Weapons | Charges, but **cannot fire** (V46) | Fires; a small charge bonus | game.c:1392, game.c:1436 |
| Flagship artillery | **Cannot fire**; a replacement takes over (V48) | Fires | `game_artillery_manned`, `artillery_reman` game.c:827 |
| Shields, oxygen, medbay, drones, cloaking, teleporter | Work without crew | No manning effect | |

- **Automated ships** (Auto Scout, Auto-Assault, drone ships) need no crew: everything counts as manned, and evasion is 5% per engine bar.
- **The flagship after AI takeover** keeps +10 evasion while its pilot system is intact (game.c:1017–1018).

### Compared with FTL 1.6.14
- **Same as the original:** the pilot requirements (evasion and FTL), and the bonuses for engines, shields and weapons.
- **Different (house rules):** weapons and artillery need a gunner to fire.
- **Not modelled:** doors have no manning effect here.

Recovery: releases/v47/ftl-ds.nds. Current ROM: releases/v48/ftl-ds.nds. Snapshot: checkpoints/v48-replacement-gunners.zip and build/v48-manifest.json.
