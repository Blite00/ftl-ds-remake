# V26 original combat-event design

## Scope and provenance

Eight reviewed roots retain the original first-six indices. Index 6 is AUTO_REFUEL_STATION; index 7 is SQUAT_REFUEL_STATION. Both original unique flags remain intact. Their source event definitions are in data/events_rebel.xml, ship outcomes in data/events_ships.xml, English strings in data/text_events.xml, and ship pools in data/autoBlueprints.xml. Generated content remains local. These are complete closures, with no discarded surrender, escape or quest branches: neither source ship defines those branches.

The compiler imports all entries of SHIPS_AUTO (AUTO_BASIC, AUTO_ASSAULT) and SHIPS_REBEL (REBEL_FAT, REBEL_SKINNY), preserving order and duplicate weighting if present. Unsupported blueprint entries reject the root. Mapping is ENEMY_SCOUT, ENEMY_ASSAULT, ENEMY_REBEL_FAT and ENEMY_REBEL respectively. Importing a ship pool is not evidence that the original engine's equipment generation has been reproduced.

## Original outcome structure

Both roots first establish a non-hostile ship and offer intervention or avoidance. Intervention changes the existing ship to hostile; avoidance ends without combat or loot.

- REBEL_AUTO_REFUEL destroyed: original salvage text, LOW standard reward, explicit Continue, original station thanks, MED standard reward. There is no deadCrew callback because the pool contains automated ships.
- SQUAT_REFUEL_STATION destroyed: original destruction text, MED standard reward, explicit Continue, station thanks, MED fuel reward.
- SQUAT_REFUEL_STATION deadCrew: original intact-ship salvage text, HIGH standard reward, explicit Continue, the same station thanks, MED fuel reward.

Callbacks use their original loaded or inline event semantics. No generic campaign reward may run in addition to these outcomes. Crew elimination must not animate ship destruction. Each reward commits once, and the station reward waits for its explicit Continue.

## Schema 2

ImportedEventNode appends ship and hostile. ship=-1 means no new descriptor; hostile=-1 means no hostility action, 0 establishes peaceful status and 1 requests combat. ImportedEventShip contains id, first_enemy, enemy_count, destroyed and dead_crew. Missing source callbacks use -1. imported_event_enemies contains symbolic game enemy IDs; JSON also stores the corresponding original blueprint IDs for audit.

Ship loads establish lexical context for descendant choices and loaded events. A hostility action without a ship context, a replacement ship while one is in context, multiple ship actions, malformed/missing hostility, unknown ship attributes and unsupported outcome children reject the whole root. Sibling branches cannot borrow another branch's ship context. Ship-reference cycles and incomplete callback closures reject the root. The compiler maintains existing depth/node/text/choice limits.

## Explicit DS approximations

Source autoReward categories do not supply numeric distributions. This milestone maps standard LOW/MED/HIGH rewards to 1-3 fuel, 1-3 missiles and respectively 10-20 / 20-30 / 30-45 scrap. These are independent inclusive bounded draws, not recovered original reward math. Existing MED fuel stays 3-6 fuel plus 15-25 scrap. No automatic weapon, drone or crew loot category is newly supported.

Decorative img planet/back hints are accepted and omitted from presentation, as existing decorative text-list hints already were. Unknown attributes or nested image effects fail. Original text, choices and outcome distinction remain imported; DS placement, combat parameter scaling and reward distributions remain adaptations.

The generated coverage register still accounts for 351 named event definitions in twelve base-source containers. Additional ship and blueprint sources are hashed as dependencies; those containers alone do not establish historical pre-AE eligibility of every definition.

## Compiler evidence

24 Python tests pass, including all 17 prior tests. New checks cover non-hostile setup and attack branches, separate hull/crew callbacks and nested rewards, rejection of surrender/escape/unsupported callback effects, unknown ship pools, missing context and sibling isolation, loaded callbacks/cycles, and exact installed pool membership. The generated catalog contains 53 nodes, 37 choices and two ship descriptors. C runtime, save, campaign, preview and ROM evidence belongs in the milestone checkpoint after integration.

## Source hashes

- data/events_rebel.xml: `078463f694431318da2bf9258408f63e18ce4978f0b802ae822b69db5829ac1b`
- data/events_ships.xml: `cea6572b5872df401d10829eaf08f990dc93877fa35ce38dacae9f690b608db2`
- data/autoBlueprints.xml: `0f0929e317f91523e000b4e73c66f4e22fd6a3dff82df7ab94eaba49c32f20f6`
- data/text_events.xml: `a2b406e681a3ec85fc842b2c5aba9c6bd50e0b901b8c445926f627f0adfef03f`

## Campaign placement and compatibility

New runs use event policy 2. Policy 1 keeps the original six economy roots and their indices; old saves do not silently gain new station encounters. Policy 2 adds the two unique station-defense roots at optional fight beacons from sector two onward. The fixed opening fight and guaranteed service anchors remain unchanged. Fleet interceptions take priority over beacon events; revisits do not repeat event rewards.

Save format 19 retains format 18's exact payload layout. It expands the permitted new-run policy and unique-root mask, while historical format 18 continues to reject those new values. Active encounter descriptors and phases are transient; saving remains map-only. Historical payload and backup preservation are recorded in build/v26-release-migration.json.

## Integration validation status

All 76 C checks and 24 compiler tests pass. Six earned campaigns cover nine station hull victories, both roots and all four enemies; crew-only outcomes have focused fixture evidence. Actual V25 migration, inspected renderer previews and the V26 ROM build passed. See V26_CHECKPOINT.md for scoped evidence. Physical DS, emulator input/audio/storage and runtime performance acceptance remain pending.

Peaceful ship loads remain narrative context until Engage instantiates the encounter. Precombat text is shown before combat rather than simulated beside a peaceful enemy. Mandatory combat rewards clamp at the one-million resource bound and report actual gains plus a storage-limit notice; this prevents a terminal reward from trapping a run. Choice panels wrap both labels and hints; every compiled choice is checked against the three-row display budget.
