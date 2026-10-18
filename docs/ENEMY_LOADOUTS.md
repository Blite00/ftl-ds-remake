# Enemy weapon loadouts: source and validation

## What the original data establishes

The installed archive was read without modification. Original XML and generated extracts remain local. Source membership establishes eligible weapon families; it does not recover the executable's pairing, rarity weighting, sector scaling, power provisioning, targeting, or difficulty rules.

| Hull | Source weapon pool | Weapon start / maximum | Missile reserve |
| --- | --- | --- | --- |
| AUTO_BASIC | WEAPONS_AUTO | 2 / 8 | 10 |
| AUTO_ASSAULT | WEAPONS_AUTO | 0 / 6 | 10 |
| REBEL_SKINNY | WEAPONS_REBEL | 2 / 8 | 10 |
| REBEL_FAT | WEAPONS_REBEL | 1 / 6 | 10 |

These are the XML weapon-system `power` and `max` values, not a claim that every original spawned ship has the listed initial allocation. The current legacy DS Assault offense is its combat drone. Assigning weapons to that hull requires explicit authored provisioning.

Both pools include LASER_BURST_1 (Basic Laser), LASER_BURST_2_A, LASER_BURST_3 (Burst Laser II), LASER_HEAVY_1, LASER_HEAVY_2, and enemy MISSILES_1, MISSILES_2, MISSILES_3. Existing supported catalog entries also include Halberd and Small Bomb in both lists, but the initial two-weapon engine scope is lasers and missiles. WEAPONS_AUTO additionally contains Ion Blast; WEAPONS_REBEL does not.

| Source ID | Hull damage / shots | Cooldown | Power | Ammo / shield piercing |
| --- | --- | --- | --- | --- |
| LASER_BURST_1 | 1 / 1 | 10 s | 1 | 0 / 0 |
| LASER_BURST_2_A | 1 / 2 | 11 s | 2 | 0 / 0 |
| LASER_BURST_3 | 1 / 3 | 12 s | 2 | 0 / 0 |
| LASER_HEAVY_1 | 2 / 1 | 9 s | 1 | 0 / 0 |
| MISSILES_1 | 1 / 1 | 9 s | 1 | 1 / 5 |
| MISSILES_2 | 2 / 1 | 10 s | 2 | 1 / 5 |
| MISSILES_2_PLAYER | 2 / 1 | 11 s | 1 | 1 / 5 |

The player catalog's Artemis is MISSILES_2_PLAYER, absent from WEAPONS_AUTO and WEAPONS_REBEL. The new enemy-only import instead uses exact MISSILES_2, exposed as GAME_ENEMY_ARTEMIS (7). The opt-in enemy API rejects player catalog 0. Enemy Artemis is not added to player equipment, stores, or loot. Its source cooldown is 10 seconds and power cost is 2.

## Recommended bounded authored profiles

These recommendations use source-listed components already supported by the DS catalog. The combinations and selection policy are authored, not imported spawn rules:

- Basic Laser + Heavy Laser I: two power, distinct cooldowns. Two independent hits can overcome a one-layer shield when sufficiently close together.
- Basic Laser + Burst Laser II: three power, stronger later encounter. The burst can remove shield layers while the other slot continues charging independently.
- Basic Laser + imported enemy Artemis: three power and a finite shared missile reserve. Both components occur in the source pools; the chosen pairing remains authored.

The implemented two-slot API enforces total weapon power within capacity without changing hull, reactor or system stats. Slots keep independent charge and queued-volley targets and consume shared ammunition once per missile volley. Configuration rejects pending volleys or active outgoing projectiles. Available weapon power is assigned in slot order; a later slot loses power first when the earlier slot can still be powered. Already launched projectiles remain active. Campaign provisioning is a separate staged operation.

## Reproducible enemy-only import

`tools/import_enemy_weapons.py` uses the existing read-only archive parser and emits `generated/enemy_weapon_variants.h` and `.json`. The JSON records exact source paths, SHA-256 hashes, raw payload fields, and stated adaptations. `tests/enemy_weapon_import_test.py` verifies override presence (including explicit zero), malformed/duplicate/unsupported payload rejection, and deterministic generated header reproduction; all 3 tests pass.

The installed missile definition includes `stunChance=1`. This metadata is retained; Advanced Edition stun effects remain deferred, consistently with the existing selected weapon policy. Fire/breach chance conversion and absent crew-damage fallback remain the existing DS rules, not newly verified executable behavior.

## Implemented campaign provisioning

The generated-route campaign (`route_policy == 1`) opts in from sector index 1, the second sector, when event policy is at least 4 (currently policy 4). Sector 0, the flagship, and other enemy kinds keep their previous weapon behavior. Historical event policies below 4 and legacy routes keep the single-weapon path. Authored arrivals and imported-event engagements share this same encounter setup.

- Scout and Rebel Fighter: Basic + Heavy, or Basic + enemy Artemis, selected deterministically by `(seed + current beacon + sector) & 1`.
- Rebel Boarder: Basic + Heavy.
- Auto-Assault and Rebel Rigger: one Basic Laser alongside their existing drone.

Weapon-room identity and maximum capacity come from the imported hull. The weapon system receives exactly the required capacity, health and power, bounded by that hull's source maximum. Engine power is reduced only when the resulting total exceeds the unchanged reactor; no other system is drained. Missing/conflicting rooms, insufficient maximum capacity, or insufficient engine power to cover the excess reject the staged encounter atomically. The profile choice adds no campaign RNG draw. Pairings, deterministic selection, installed starting bars, and engine reallocation are authored DS approximations. The Burst II pairing below is a comparison fixture, not a current campaign profile.

## Threat display and sensor limits

The combat overview displays armament information for configured enemy loadouts while the enemy is alive and present. The targeting screen keeps its selected-room card; the armament panel belongs to the normal combat overview.

Below Sensors level 3, the panel identifies exterior weapon families only: for example, DUAL LASERS, LASER + MISSILE, or LASER + DRONE. Missile equipment adds a BYPASS SHIELDS hint; other families prompt TARGET WEAPONS. These public family labels are a DS presentation choice. They do not expose exact weapon names, current charge, power state, or remaining ammunition, and the missile-family warning remains a property of the mounted equipment even when its reserve is empty.

At Sensors level 3, each weapon slot has its own short name and charge bar. Status priority is FIRING for a queued volley, EMPTY for an exhausted missile reserve, OFF when unpowered, then seconds remaining to recharge. A single configured weapon with a combat drone also shows DRONE SUPPORT and its charge bar. This display reads the independent slot state through the enemy-weapon API; it does not invent a shared charge value. Departed or destroyed enemies have no active armament panel.

## Combat and earned-run evidence

`tests/enemy_loadout_run_smoke.c` passes strict C99 compilation with warnings treated as errors. `build/enemy-loadout-evidence.log` contains full results. Fixed fixtures use the same unmodified starting Kestrel against a sector-index-1 Scout, with explicit enemy weapon-capacity provisioning. Each row spans 32 deterministic seeds and at most 120 seconds. Passive rows use no crew/fire input; ready rows fire charged weapons at enemy Weapons through normal APIs. Timeouts are living, unfinished battles.

| Fixed Scout profile | Input | Wins / losses / timeouts | Mean hull loss | Mean player missiles | Mean enemy missiles |
| --- | --- | --- | --- | --- | --- |
| Legacy single Basic | Ready | 32 / 0 / 0 | 0.00 | 2.00 | 0.00 |
| Legacy single Basic | Passive | 0 / 0 / 32 | 0.00 | 0.00 | 0.00 |
| Basic + Heavy | Ready | 32 / 0 / 0 | 0.72 | 2.44 | 0.00 |
| Basic + Heavy | Passive | 0 / 10 / 22 | 7.31 | 0.00 | 0.00 |
| Basic + Burst II | Ready | 32 / 0 / 0 | 0.00 | 2.03 | 0.00 |
| Basic + Burst II | Passive | 0 / 25 / 7 | 19.59 | 0.00 | 0.00 |
| Basic + enemy Artemis | Ready | 32 / 0 / 0 | 1.75 | 2.53 | 1.00 |
| Basic + enemy Artemis | Passive | 0 / 29 / 3 | 17.94 | 0.00 | 7.94 |

The new profiles can hurt an unattended starting ship; targeting Weapons still makes a substantial difference. These fixtures do not establish original FTL difficulty parity. Some defeats occur through crew loss, so mean hull loss is not a survival score.

Three separate complete eight-sector campaigns use ordinary crew orders, firing, choices, stores, repairs and upgrades, with no injected player resources, health, charge, damage, victories or geometry. The pilot chooses Rebel routes when available and one forward fight after visiting stores when healthy. It inherits reusable ordinary-action helpers from the existing campaign test; that file's separate synthetic fixture main is never called.

| Seed | Jumps | Final hull | Final scrap | New-profile battles | Player missiles consumed | Enemy missiles consumed |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 66 | 30 | 102 | 2 | 29 | 0 |
| 42 | 64 | 30 | 132 | 3 | 31 | 0 |
| 8675309 | 65 | 30 | 153 | 4 | 28 | 1 |

Totals: 195 jumps, 9 new-profile battles including 2 missile profiles, 88 player missiles and 1 enemy missile consumed. Final hull includes legitimately purchased repairs and must not be presented as damage-free runs. Strong weapon suppression often prevents the enemy missile slot from firing. This test provides host simulation evidence, not emulator or physical DS validation.

## Next surrender dependencies

The complete generic REBEL closure still needs RANDOM stuff, HIGH fuel, LOW scrap_only, and chance-based escape without an explicit timer/gotaway callback. DEAD_CREW_DEFAULT must retain all nine weighted alternatives: MED standard three times, HIGH standard twice, HIGH fuel twice, crew1 plus LOW scrap_only once, LOW weapon once. Existing weapon/recruit pending-offer support handles the object primitives only. Full source branches must remain closed; no selective dropping or quest subevents in random pools.

## Read-only source fingerprints

- `data/autoBlueprints.xml`: `0f0929e317f91523e000b4e73c66f4e22fd6a3dff82df7ab94eaba49c32f20f6`
- `data/blueprints.xml`: `42e78e0c664d8f5d3ab22061f57c1bdd969998008b56f4563ec67e081571677b`
