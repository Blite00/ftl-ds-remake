# V41 final sector: Federation base defense

## Sources

- FTL 1.6.14 `FTLGame.exe`, read-only disassembly; functions were located with Hyperspace 1.6.9 signatures:
  - `StarMap::UpdateBoss` (0x5f4ed0)
  - `ForceBossJump` (0x5eed40)
  - `ReverseBossPath` (0x5eece0)
  - the base-loss predicate (0x5eee40)
  - the boss-route generator inside map generation (0x5f96eb–0x5f97ad)
  - the countdown display `4 - arrivedAtBase` (0x5ffdf7)
- `ftl.dat`:
  - `sector_data.xml` FINAL: 1 store, 3 BOSS_REPAIR_STATION, 6 BOSS_HOSTILE, 7–10 BOSS_NEUTRAL.
  - `events_boss.xml`: LAST_STAND_START, BOSS_REPAIR_STATION, BOSS_FLEETS_REBEL.
- These rules have not been compared with controlled play sessions.

## Verified rules and the DS implementation

| Rule | DS |
|---|---|
| The flagship follows a route of 4–6 beacons ending at the Federation base (generation retries until the path size is 4–6) | Shortest route from a random beacon 3–5 links from the base, with random ties. The base is the last generated beacon |
| Each player jump or wait runs `UpdateBoss`. A move flag alternates, so the flagship advances one beacon every second update | `boss_update`, called from arrival and wait |
| Entering a non-base beacon turns it rebel-held | `fleet_take` |
| Arriving at the base sets `arrivedAtBase=1`. Each later update adds 1, up to 4 | `base_arrived` |
| The base is lost when the counter is 4 and the player is not at the flagship's beacon. The UI shows `4 - arrived` | Campaign defeat with a base-destroyed message; "BASE UNDER ATTACK: N JUMPS LEFT" |
| After a defeated phase, `ForceBossJump` moves the flagship on the next update. Beaten at the base, it reverses its route and the counter resets | `boss_force_jump` |
| A reversed route lasts one move: once the flag clears, the route is reversed back, so the flagship retreats one beacon and turns for the base | Same |
| At a non-base route end the route reverses | Same |
| Each update, beacons marked last time become rebel-held. Then one random beacon (not the base, not already held) is marked, and a second once the flagship is past index 1 or retreating | `fleet_pending` → `fleet_nodes` |
| LAST_STAND_START on the first base visit: +10 fuel, repair 10 hull | `base_greeted` once |
| BOSS_REPAIR_STATION: repair 15 hull, repair systems, +4 missiles, +5 fuel, +5 drone parts, +22–44 scrap | Automatic on first visit |
| BOSS_FLEETS_REBEL: a hostile Rebel ship on rebel-held beacons, LOW scrap_only reward | Rebel fight paying out like the authored patrol (1 emergency fuel) |

Meeting the flagship (jumping onto its beacon, or it moving onto yours) starts the current phase. Phases 1→2→3 persist in `boss_resume_phase`. Winning phase 3 completes the campaign. The ordinary fleet pursuit front is disabled in this sector; rebel-held beacons replace it.

## DS approximations and open items

- The **phase battles still use the interim authored Rebel Command platforms**. The imported BOSS_1–3 layouts, artillery, cloaking, drone and teleporter phases, and power surges are the next sub-milestones (V42+).
- **Retreat policy:** if the player flees a phase, it restarts at full flagship hull, as in V28. Whether the original keeps damage is unverified.
- Neutral beacons approximate BOSS_NEUTRAL with DS supply, distress and fight events.
- Exact route generation (start choice, distance metric) and slot-level fleet event content are not fully decoded.
- Legacy six-sector runs and save-24 runs already inside the final sector keep the interim exit battle.

## Save format 25

Appended: finale policy, route length/position, move/reversal flags, base counter, first-visit flag, six route beacons, rebel-held and pending masks (60 bytes). Validation covers:
- route linkage
- route end at the base (or start, while reversed)
- mask bounds, with the base never held
- empty state outside the final sector

Saves 2–24 migrate. Runs before the final sector get policy 1; runs already inside keep policy 0. Backups are byte-preserved as before.
