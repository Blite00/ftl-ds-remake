# V27 warning scout content

AUTO_WARNING is root 8 under new-run policy3. Its nine original introductions and exact AUTO_BASIC/AUTO_ASSAULT pool are preserved. The complete closure includes escape warning, gotaway pursuit +1 without salvage, and LOW-standard destruction salvage. Nine roots are enabled among 351 registered named definitions; 342 remain disabled. See [V27_ESCAPE_DESIGN.md](V27_ESCAPE_DESIGN.md).

Raw timer 40 and hull bounds 22/22 are imported. The DS interprets the hull value as an absolute threshold that latches a 40-second operational charge. Pause and disabled/ionized engines or piloting interrupt progress. Timing semantics remain an explicit approximation pending original executable verification. Announced charging remains visible with basic sensors; the stopped status does not reveal hidden system details.

The existing sector_jumps field counts pursuit turns, including WAIT and rescue; the escape consequence adds one while actual travel remains unchanged. Only positive modifyPursuit amounts are supported; negative/zero effects are rejected. Save20 preserves old policies and the existing payload layout; only policy3 admits the ninth root.

Departure removes the remote ship and loses crew aboard it, without species death bursts or hull explosion. Home boarders and hazards continue until resolved. Clearing queued/in-flight attacks at departure is a bounded DS adaptation. New home-target bombs can still be fired at remaining boarders; remote shots are rejected without spending ammunition. Player retreat, surrender and other escape closures remain unfinished.

# Current V26 original event and enemy content

Eight complete original roots (53 nodes, 37 choices) are enabled for new policy2 runs. The new AUTO_REFUEL_STATION and SQUAT_REFUEL_STATION closures retain attack/avoid choices, original ship pools and separate destroyed/deadCrew outcomes with explicit station rewards. See [V26_EVENT_DESIGN.md](V26_EVENT_DESIGN.md). The coverage register still contains 351 named definitions; 343 remain disabled.

REBEL_FAT imports the original Rebel Rigger hull art, eight-room/eight-door topology and system data. Its supported Defense Laser Mark I and Combat I equipment is a fixed selection from original pools; optional medbay selection, hull scaling and drone cadence are authored. Enemy AI accounts for drone power and repairs damaged control. Original randomized loadout generation remains unfinished.

New combat roots appear at optional fights from sector two, excluding fixed opening beacons. Fleet patrols take precedence. Save19 keeps old policy1 runs on their six-root pool; policy0 stays authored. Imported values/text, supported simulation and DS approximations remain distinct. Original content stays local.

Earlier sections below describe their historical versions.

# Current V25 original event content

Six complete original event roots are enabled only in new runs. [V25_EVENT_DESIGN.md](V25_EVENT_DESIGN.md) lists the roots, source hashes, closed references, supported effects and authored adaptations. generated/event_coverage.json registers 351 named definitions in twelve source containers; 345 remain disabled. This is not complete historical base-game coverage.

Original trades replace optional supply/repair encounters; guaranteed fuel/repair anchors remain authored. Out-of-fuel WAIT uses the refugee contact outside fleet capture. Source unique flags persist campaign-wide. Existing save formats 2-17 retain authored event policy when migrated to 18. Original quests, broader conditions/combat outcomes and active hazards are unfinished.

# Current V24 content policy

New runs use eight sectors, 23 beacons and persistent Civilian/Engi/Rebel/Pirate identities with branching, followed by Last Stand. [V24_SECTOR_DESIGN.md](V24_SECTOR_DESIGN.md) defines authored weights and supported enemy pools. Saves through 16 retain their existing generation rules; six-sector saves remain six sectors.

Catalog/mechanics additions through V24 are recorded in checkpoints and generated weapon/drone coverage registers. Earlier content descriptions below are historical policy for legacy runs, not current absence claims. Original XML events, active hazards and authentic flagship/base defense remain incomplete.

# V12 campaign content and rules

## Release subset

This is an authored six-sector Kestrel campaign for the DS demake. Its maps,

choices, economy, scaling and three-stage command-ship finale are original

project designs. They are not an interpretation of the complete PC event XML

or a faithful recreation of the Rebel Flagship's mechanics.

Imported local content remains in `generated/`: Kestrel room/door geometry,

base hull and equipment data, Auto-Scout, Auto-Assault and Rebel Fighter geometry, selected supported weapons/drone and artwork.

The generated quiet-event text from V9 remains in the asset pipeline but is

not used by this authored campaign. Unsupported original events are never

selected. The campaign has no runtime allocations or whole-archive loading.

## Maps and progression

- Six sectors, each with twelve beacons in seven columns.

- Two forward lanes, a randomized crossing per column, and optional links

  between lanes. Links are bidirectional. Every generated beacon and exit is

  reachable. Coordinates and event types vary with the seed.

- The start branches to a fight or supplies. Beacon 6 always has a store,

  beacon 9 a courier relay, beacon 10 repairs, and beacon 12 the exit.

- Jumps consume one fuel. Searching an already visited beacon yields no new

  event reward. A store retains its remaining stock when revisited.

- After eight jumps in a sector, the rebel fleet catches columns behind the

  player. Entering a caught beacon starts a patrol encounter, including on a

  revisit. Patrols yield one fuel and no scrap or ammunition.

- A sector transition gives three fuel and twelve scrap. Fuel exhaustion can

  be resolved from the map RESCUE button: two fuel for ten scrap, or up to two hull if

  scrap is unavailable. Rescue advances pursuit by two jumps.

- Run seed, random state, maps, visited flags, resources, quest, stock and cargo

  are persistent values. Saving is available on the map; active fights are

  outside the save subset.

## Supported event actions

The event dispatcher is a finite set of typed choices with explicit conditions

and one-time effects. It does not parse arbitrary XML or execute scripts.

| Event | Choices and consequences |

|---|---|

| Supplies | Four fuel, or sixteen scrap plus three missiles |

| Distress | Give two fuel for twenty-four scrap; take ten scrap; or fight an escort patrol |

| Repair dock | Free six-hull patch, or full hull repair for eighteen scrap |

| Courier | Accept codes; deliver at another unvisited relay for forty-five scrap/four fuel; or take two fuel |

| Solar flare | Risk three hull for twenty-five scrap; use four installed shield bars to salvage safely; or leave |

| Exit | Enter next sector, or begin the final encounter |

Solar flares are event consequences rather than periodic in-combat effects.

The courier is a two-stage persistent quest. Declining or resolving a choice

cannot be retried for additional rewards. Insufficient fuel, hull, scrap or

shield capacity disables the corresponding choice.

## Equipment and economy

Four equipment IDs are supported: Artemis (0), Burst Laser II (1), Basic Laser

(2), Heavy Laser (3). The first two use imported starting weapon statistics;

the extra laser definitions are chosen demake values. Equipment is limited to

four installed weapons plus four cargo slots. Swapping returns the old weapon

to the selected cargo slot; adding a weapon consumes that cargo item.

Each store has three purchases of three fuel (6 scrap), three missiles (6),

and five hull (8). It also has one human recruit (35), Basic Laser (25), Heavy

Laser (40), and Burst II (60). Selling the first occupied cargo slot yields

12 scrap. These prices deliberately favor a small campaign roster rather than

PC economic fidelity. Crew are humans only, with a maximum roster of six.

Installed systems can be upgraded from management, a store or a resolved event, including between final phases. Shields,

engines and weapons cap at eight bars; oxygen and medbay cap at three. One

bar costs 10 + 8 times current capacity. Reactor bars cost 20 + current

reactor capacity, up to twenty bars. Players must allocate power after upgrading.

Shield strength still requires two powered, healthy bars per layer.

Ordinary victories yield 18 + 3 times the zero-based sector number in scrap,

two fuel and two missiles. These rewards are given once. Repairing systems,

healing and repositioning crew use the player simulation between encounters;

they do not restore hull or consume pursuit steps. The DS input loop advances this servicing only on the Crew management page; other menus and narrative hold time.

## Enemies and finale

Original Auto-Scout, Auto-Assault and Rebel Fighter are playable. The Fighter uses three humans, original doors/rooms, a source laser and its optional medbay. Unsupported optional teleporter equipment stays excluded. Source geometry, loadout selection and chosen scaling are distinguished in ENEMY_IMPORT.md. Procedural Interceptor, Armored Drone and Rebel Command remain authored designs. Enemy pools expand after the first sector.

The three-stage command finale remains simplified. Before phase one a tender restores ten hull and six missiles; between phases it restores four hull and two missiles. Only phase three completes the campaign. Its original boarding, cloaking, power surges and crew behavior are not implemented yet.

Basic fire and enemy Combat I drone rules are supported. Breaches, species, boarding, beams/ion, player drone control, augments, other player ships and full flagship mechanics remain in the active base-game goal. Advanced Edition remains excluded. Crew names, stable identity, stations and chosen activity frames are implemented; skill timings are demake choices. Enemy hull destruction and crew elimination use distinct result reasons but currently yield the same ordinary salvage once.

Save format4 includes player names, IDs, stations and location. V10 format2 and V11 format3 migrate with generated names and current-room station defaults, preserving original progression. Saves remain map-only; no transient enemy crew state is saved. Unknown newer save versions are protected from replacement.

## Host evidence

`tests/campaign_v10_smoke.c` verifies 256 seeded maps for connectivity, symmetry

and screen bounds; it checks event conditions and one-time rewards, cargo

swapping, store stock, recruitment, upgrades, pursuit and emergency fuel.

Three complete six-sector runs (seeds 1, 42 and 8675309) defeat every encounter

and all three final phases with actual weapon charging/firing. The scripts buy

and install equipment, manage power, and repair/heal between encounters using

public APIs. They do not inject victory or replenish resources outside normal

campaign actions during those full runs.

These deterministic host simulations establish that those tested routes are

completable. They do not establish balance for every seed, emulator input

quality, physical DS frame time, or flashcart save durability.

## V13 supported boarding subset

The Rebel boarding encounter uses the imported REBEL_SKINNY layout and three-human roster, selecting its original optional teleporter. It is a supported loadout variant rather than a new hull. Timing, tactics and encounter selection remain authored policies.

Player installation costs 120 scrap at stores; capacity upgrades cost 40 each up to level three. Crew-only victories retain ordinary salvage amounts. Away survivors block departure, and explicit abandonment may cause defeat. Source-derived data and behavioral approximations are detailed in ENEMY_IMPORT.md and COMBAT.md.

Basic fires, enemy Combat I drones, named living crews and boarding are current supported subsets. Earlier release descriptions above are historical for their recorded tests. V13 suite, campaign route, ROM, framebuffer and static-memory evidence is recorded in V13_CHECKPOINT.md; emulator and physical acceptance remain pending.

## V14 current damage content

The same four player weapons now carry independent hull/system/crew damage and original raw fire/breach fields, under the authored conversion policy in COMBAT.md. Heavy Laser I uses original nine-second/one-power values. Prices, procedural enemies and campaign economy remain authored. Breaches and sealing are implemented; species, functional doors/sensors and expanded weapon families are still incomplete. V14 validation is recorded separately from historical full-run evidence.

## V15 subsystem content

Kestrel doors and sensors use original rooms and initial levels. Upgrade prices follow source tables: doors 35/50, sensors 25/40, both capped at level three. Current imported enemy hulls omit these systems; their physical door geometry does not imply door control. Varied future original enemy subsystem configurations remain gated on their complete supporting mechanics. Resistance timing, visibility tiers and crew repair behavior remain authored rules described in COMBAT.md. Current V15 validation is pending.
