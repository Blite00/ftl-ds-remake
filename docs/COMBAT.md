# Combat foundation checkpoint

## Implemented

Game state uses bounded storage for eight crew and four equipped weapons, with explicit active counts. All simulation loops respect active counts. Five reactor-powered systems remain. Piloting is a separate one-health console in imported Kestrel room 0 and needs a living stationary crew member for evasion. Starting crew occupy piloting, engines and weapons.

Disabled weapons consume no power. Allocation follows displayed installed order, left to right. Formats 2–11 migrate their historical slot-1 priority into that order. If a weapon cannot fit the remaining power, the next enabled weapon can still use those bars. Disabling a weapon clears its charge but does not cancel a launched/queued volley. Each weapon may lock an independent room target; -1 follows the shared selected target. Queued volleys keep their launch target.

The catalog contains Artemis and Burst II using imported local statistics, plus Basic Laser (10-second charge, one power, one damage) and Heavy Laser I (nine-second charge, one power, two damage). Added weapon prices are demake balance choices; Heavy Laser I timing and power now match its local blueprint. Slots can be replaced and appended up to four; inventory ownership is handled by campaign code.

`game_begin_encounter` preserves player hull, crew, equipment, activation and system state while clearing encounter shots, charges, autofire and independent target locks. Scout retains its imported layout and baseline; sector scaling and procedural Interceptor, Armored Drone and Rebel Command layouts/loadouts are chosen demake designs. The three-phase command encounter substitutes for the original Flagship; it has no boarding, drones or cloaking mechanics.

`game_service_step` advances player atmosphere, shield recharge, crew movement, repairs, healing and crew skills. While friendly crew remain aboard an intact captured enemy, its atmosphere and fire also advance; enemy attacks, orders and projectile effects stay stopped. Management APIs remain usable after combat victory. Pause stops service. Suffocation can still cause defeat while servicing a vented ship.

## Chosen mechanics, not recovered PC behavior

- Crew skills train while stationary at piloting, engines, weapons or shields. After 60 and 120 seconds at a station the corresponding bonus improves; values cap at 120 seconds.
- Station bonuses are 5/10/15. Weapons add that percentage to charging speed. Piloting and engines each add that percentage-point evasion bonus; evasion requires working piloting, a pilot and powered engines, and caps at 45%.
- A staffed shield station gains one extra shield-recharge tick every five simulation ticks. Skill levels currently do not alter that recharge increment.
- Piloting loses its one health from an unshielded hit to its room; stationary crew repair it in five seconds.
- Existing provisional movement, oxygen, medical healing and automated enemy repairs remain.
- Basic fires, firefighting and an enemy Combat I drone are supported. Species, beams/ion, player drones and cloaking remain in the active goal.

## Validation

`tests/foundation_smoke.c` covers bounded roster/equipment, unmanned/damaged piloting, weapon activation and power priority, repairs/healing after victory, service isolation from enemy/effects, preserving player state on encounter transitions, and independent autofire target locks. Compiled with host GCC using `-std=c99 -Wall -Wextra -Werror`; passed. Existing basic combat smoke test also passed (699 ticks, 30 hull in deterministic baseline encounter).

These are host simulation checks, not DS timing, stylus, audio or storage validation. The integrated DS build and framebuffer previews are recorded in IMPLEMENTATION.md.

## V12 shared crew and enemy rules

Both sides now have bounded rosters, stable identity, names, location and station fields. Door lists and connectivity are separate per ship; side-aware APIs route movement and apply atmosphere, damage, firefighting and healing. Compatibility fields retain the player layout representation while the simulation uses common accessors. Kestrel rendering and equipment installation are not yet generalized to other playable hulls.

The imported Rebel Fighter starts with three humans and a selected optional medbay. Its AI prioritizes injury, fire and damaged systems, then returns to assigned stations. Powered healing takes reactor allocation from engines as needed; total allocation stays within its seven bars. Living crew repair one bar every five seconds, while automated ships retain their separate repair rule. Shared movement is one room per second; medical healing is six HP/second per powered bar. These rates and the AI are authored demake choices. Enemy fire spread follows its imported doors.

Crew details hold simulation. Save stations stores current rooms; return stations issues movement orders, skipping dead/away crew. UI activity frames use existing sprite art plus code-drawn motion/tools; they are not imported original animation sequences. Names permit up to twelve rendered letters/digits/spaces/hyphens/apostrophes. The touch keyboard currently exposes letters and spaces.

Battle outcomes now distinguish player/enemy hull destruction from loss of all living crew. Automated ships are exempt from crew-elimination victory. Campaign rewards are applied once; separate animation/transition gating and stranded-crew handling will accompany boarding. No teleporter, melee or sabotage is playable in this checkpoint.

## V13 boarding and terminal presentation

The teleporter occupies the original Kestrel teleporter room, shares the reactor budget, and can be damaged/repaired. Its one to three powered bars select a 20/15/10-second recharge at activation. Later power changes do not alter that chosen duration; control without functioning power pauses recharge.

Parties contain at most two living crew, stationary together. Deployment requires the home teleporter room; recall requires a shared room aboard the enemy. Teleport arrival limits friendly occupancy to room area. This is not a complete tile-by-tile occupancy simulation. Crew retain faction, identity and roster slot while their ship location changes.

Contested rooms apply six melee damage per fighter per second, resolving accumulated damage together. A stationary uncontested intruder sabotages one system bar every five seconds. These rates are authored approximations. Combat interrupts repairs and firefighting; powered medbay healing continues during combat and sealing. Oxygen, fire, medical rules and destruction use actual ship location.

Enemy parties gather healthy non-pilot crew; defenders intercept intruders before launching. Boarders below 30 health attempt recall when control is ready. The Rebel boarding variant selects its original optional teleporter. Crew aboard a destroyed hull die; surviving hostile boarders remain dangerous aboard the other ship. Player crew loss takes precedence over enemy hull loss, and hostile survivors must be defeated before victory.

Terminal simulation freezes. Hull outcomes run 60 presentation ticks at 30 Hz, with six staggered blasts and six debris samples, before reward/defeat dispatch. Pausing holds the clock. Crew-only outcomes skip explosions. A destroyed enemy hull can remain in active combat while its surviving boarders are fought.

Away survivors block departure after victory. Recall remains available; explicit abandonment kills them. Dead away roster entries are normalized to home coordinates before departure without restoring health or identity. Surrender/escape are not implemented, so their eventual stranded-crew policies remain open.

V13 integration validation is tracked in V13_CHECKPOINT.md; focused tests do not establish complete-run balance or DS hardware performance.

## V14 separate damage and breaches

`GameDamage` carries independent hull, system and crew amounts plus fire/breach chances. Launch captures the descriptor for queued volleys and flying effects; later weapon changes do not rewrite those shots. A shielded impact absorbs the attack before room damage and secondary effects. Unshielded impact applies each channel to the targeted ship/room, including either faction's occupants, piloting, drone control and teleporter as appropriate.

### Imported source fields and authored interpretation

| Current catalog entry | Original blueprint | Raw fire | Raw breach | Explicit sysDamage/persDamage |
|---|---|---:|---:|---|
| Artemis | MISSILES_2_PLAYER | 1 | 1 | Both absent |
| Burst Laser II | LASER_BURST_3 | 1 | 0 | Both absent |
| Basic Laser | LASER_BURST_1 | 1 | 0 | Both absent |
| Heavy Laser I | LASER_HEAVY_1 | 3 | 3 | Both absent |

`tools/import_enemy_catalog.py` now exports `generated/weapon_properties.h` and the matching JSON audit. Presence flags distinguish missing override fields from explicit zero. Repeat extraction and direct source comparisons passed; installed archive metadata remained unchanged. The selected original Rebel laser shares LASER_BURST_1; DRONE_LASER has raw fire 1 and breach 0.

The importer verifies data, not PC engine semantics. Runtime currently maps each chance unit to ten percentage points and rolls fire and breach independently. Missing system overrides inherit hull damage; missing personnel overrides use five crew HP per hull point. These are explicit demake policies, not engine behavior recovered from XML. Stun fields in the installed post-AE archive are excluded from this base-game scope. Heavy Laser I cooldown/power changed from the older authored 13 seconds/two bars to original nine seconds/one bar. Store pricing remains authored.

### Breaches

Each room supports up to its tile area, capped at four breaches. A breached room loses oxygen and marks connected rooms exposed through open doors. Each breach currently drains 12 oxygen units per simulation tick on the 0–1000 scale; this is an authored rate. Friendly stationary crew seal one breach per five crew-seconds, provided their room is not burning or contested. Firefighting and melee take priority over sealing; sealing takes priority over system repair. Without active workers, partial repair progress resets. A sealed room no longer vents from that breach, but open-airlock exposure still applies. Enemy crew choose breached rooms for repair.

Save format 6 appends bounded breach count and repair progress per player room. Formats 2–5 receive no breaches on migration. Map-only save policy and prior-save backup preservation continue. See V14_CHECKPOINT.md for current integration tests, previews, ROM and memory evidence.

## V15 original subsystem data audit

The local Kestrel blueprint (`PLAYER_SHIP_HARD`) places starting level-one doors in room 2 and sensors in room 3. The global original system blueprints cap both at three. Their upgrade-cost lists are doors 35/50 scrap and sensors 25/40 scrap; these schedules are source-derived, not newly selected prices.

The currently imported AUTO_BASIC, AUTO_ASSAULT and REBEL_SKINNY blueprints have no door-control or sensor system descriptors. Their imported door geometry does not imply a powered door-control room. Runtime defaults must preserve that absence rather than invent a targetable subsystem room. Other original enemy blueprints offer optional door control, including REBEL_FAT room 5; their other required mechanics still determine whether they may enter the supported pool.

Original localized descriptions associate upgraded doors with slowing intruders/fire spread and sensors with interior/enemy information. The XML does not establish resistance timing, exact sensor tier visibility, crew manning bonuses or destruction rules. Runtime behavior must be documented separately as verified engine behavior or an explicit demake policy.

## V15 runtime subsystem policies

Door/sensor effective level is healthy capacity, without reactor allocation or manning bonuses. Damage, fire and hostile sabotage can disable these subsystems; friendly stationary crew repair one bar in five seconds. Upgrades add capacity and one health bar but do not fully repair prior damage. Current imported enemies have absent subsystem descriptors; no enemy rooms are fabricated for them.

Friendly crew can cross closed doors. Hostile crew can cross level-zero/one closed doors, while level-two/three doors require two/four crew-seconds of forcing. Multiple active attackers contribute to the shared door counter. Progress resets after one idle second. A forced door jams open for five seconds, cannot be manually closed during that time and closes when the timer ends. Destroyed door control blocks remote toggles and seal-all. Open doors retain their atmosphere/fire-spread effects. These rates and reset rules are authored approximations.

Sensor level one reveals friendly interior rooms; level two reveals enemy interior rooms; level three reveals detailed enemy weapon/system information. Living friendly crew grant local room sight regardless of sensor health. Hidden rooms remain selectable and targetable but occupants/hazards are withheld. Current visibility has no nebula modifier or species-derived sight. Exact tier behavior is an explicit demake policy, not inferred solely from the imported descriptions.

Format 7 saves player subsystem room identity/capacity/health and each player door's force, idle and jam timers. Older formats migrate to original healthy level-one subsystems and zero forcing/jam state. Save policy remains map-only. V15 validation is recorded separately in V15_CHECKPOINT.md.

V15 audit corrections: remote toggles/seal-all preserve active forcing progress; only idle decay or a completed break resets it. Closed-door fire spread checks use authored rates of 3% at effective level 0/1, 1% at level 2 and 0% at level 3; open doors use 10% regardless of level. Static room/system identities remain visible through fog; dynamic occupants, hazards and detailed status remain gated. Raw diagnostic files remain available for debugging, while the in-game diagnostic view is sensor-limited.

## V16 local species source audit

The base blueprint file contains Human, Engi, Mantis, Rock, Slug, Zoltan (`energy`) and Crystal; DLC Lanius (`anaerobic`) is excluded. Original recruitment costs are 45/50/55/55/45/60/60 scrap respectively. Source rarity is not a guarantee of store availability; Crystal acquisition remains a separate campaign requirement.

Localized original power descriptions explicitly state Engi double repair/half combat damage, Mantis 1.5x combat/1.2x movement/half repair, Rock 150 health/fire immunity/half movement, Zoltan 70 health/occupied-system power, and Crystal 125 health/20-percent slower movement. Human faster skill learning, Slug telepathy, Crystal reduced suffocation and active lockdown lack full numeric engine rules in these blueprint fields. The installed archive also mentions Slug mind-control immunity and Zoltan death damage; post-AE text does not by itself establish pre-AE engine behavior. Unsupported AE effects remain excluded.

`tools/import_species.py` extracts one original `walk_down` frame for each species from its declared animation sheet, scaled to 14x14 with nearest-neighbor sampling. `generated/species_art.h` contains seven RGB555 frames, 2744 bytes total. `generated/species_audit.json` records source prose, costs, rarity, crop coordinates and hashes. Regeneration is deterministic; the enlarged contact sheet was visually inspected. Full animation sequences and tint layers remain unimplemented, so existing procedural activity overlays are still presentation approximations.

## V17 ion policy

Ion Blast I uses imported charge/power/ion values and no physical damage. Evasion precedes shield interception. Unshielded hits disable normal power in the targeted installed systems/subsystems. Each ion damage adds one bar and five seconds, capped at eight bars and forty seconds; all bars clear on timer expiry. Allocations remain reserved but adjustable, and Zoltan bonus power remains usable. This is an explicit authored approximation. See V17_CHECKPOINT.md for interaction and campaign evidence.

## V18 beam policy

Halberd uses a bounded line crossing strict room interiors, once per room. Beams ignore evasion. Shield layers sampled at impact subtract one hull/system damage and five authored crew HP per layer. Damage applies at the end of a twelve-tick sweep. Points along an edge or corner alone do not hit a room. The line and payload are snapshotted when firing. See V18_CHECKPOINT.md for the original-data boundaries and validation.

## V19 bomb policy

Small Bomb spends one missile and teleports after twelve ticks, bypassing normal shields. Hostile targets can evade; friendly self-targets do not. The explicit raw personnel value 2 converts to 30 crew HP at an authored 15 HP/unit; raw system damage 2 remains separate from zero hull damage. Both factions in the room can be hit. Queue snapshots preserve side, room and payload. See V19_CHECKPOINT.md for evidence and limits.
