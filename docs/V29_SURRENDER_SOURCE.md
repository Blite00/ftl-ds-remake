# V29 surrender: source closure audit and compiler foundation

## Status

The compiler can now represent a closed surrender tree. This does not add surrender to the playable campaign. The selected list remains exactly the nine V28 original roots; their 66 nodes,37 choices and three enemy descriptors remain unchanged except for appended descriptor fields. The full base-game goal remains open.

Installed FTL was read only through `PkgArchive`. The source archive was not edited. Data facts below come from `data/events_ships.xml`, `data/events.xml`, `data/events_rebel.xml`, `data/events_engi.xml`, `data/events_zoltan.xml`, `data/autoBlueprints.xml` and `data/text_events.xml`.

## Smallest mechanically closed existing-hull example

`ENGI_UNLOCK_2FAKE` loads hostile `REBEL_ENGI_UNLOCK_2FAKE`. Its exact pool is `SHIPS_REBEL`: REBEL_FAT and REBEL_SKINNY, both supported hulls.

The ship contains:

- Surrender: min=4, max=4, no explicit chance, load `ENGI_UNLOCK_2FAKE_SURRENDER`.
- Escape: timer=40, min=18, max=18, original warning text and current-ship hostile=true.
- Gotaway: original text, no reward or pursuit effect.
- Destroyed: distinct text and MED standard reward.
- DeadCrew: distinct text and MED standard reward.

The loaded surrender tree has a mandatory information-demand choice, followed by a dialogue with two choices. Letting the ship go executes hostile=false. Attacking leaves hostility unchanged. Neither branch grants resources. All nested dialogue is preserved by the compiler.

This root is not a legitimate random encounter addition. Its introduction explicitly refers to a Rebel base supplied by the Engi. The originating unique root `ENGI_UNLOCK_1` has an Engi requirement and schedules both real and fake quest destinations. The real branch schedules `ENGI_UNLOCK_3`. Until the full quest chain, its destinations and subsequent effects are supported, the fake stage stays outside playable pools. It is compiled only as a source-backed closure test.

## Coherent standalone target: generic REBEL

The original `REBEL` root has ten introduction variants and the same exact supported two-hull SHIPS_REBEL pool. Enabling its complete source closure requires the following additions:

| Source component | Exact source contents | Missing support |
|---|---|---|
| Surrender trigger | chance=0.5, min=2, max=3, load PIRATE_SURRENDER | Once-only trigger and interactive combat suspension |
| PIRATE_SURRENDER | Accept executes hostile=false plus RANDOM stuff reward; refusal is an empty event | Resource reward family, ceasefire versus resume semantics |
| Surrender text list |23 entries with 14 unique IDs; repeated entries provide source weighting | Compiler bound expanded to 32; source duplicates remain weighted |
| Escape trigger | chance=0.5, min=3, max=4, load PIRATE_ESCAPE; no timer attribute | Chance-based escape and a documented default charge policy |
| PIRATE_ESCAPE | Original text and hostile=true | Loaded warning closure support |
| Gotaway | No callback in REBEL descriptor | Explicit supported absent-callback behavior; no invented pursuit or salvage |
| DESTROYED_DEFAULT | Two MED standard alternatives | Already representable |
| DEAD_CREW_DEFAULT | Nine alternatives: three MED standard, two HIGH standard, two HIGH fuel, one recruitment plus LOW scrap_only, one LOW weapon | Additional reward families, unspecified-species recruitment and weapon loot with overflow decisions |

The recruitment branch is `<crewMember amount="1"/>`; the weapon branch is `<autoReward level="LOW">weapon</autoReward>`. These rare alternatives cannot be dropped to make generic REBEL compile. Full crew and inventory outcomes must offer a meaningful decision, including when the existing roster or cargo is full. Any supported equipment reward roster must be explicit and documented; unsupported equipment must remain out of the playable reward pool.

The existing authored numeric standard/fuel rewards are not recovered original reward mathematics. RANDOM stuff, HIGH fuel, LOW scrap_only and LOW weapon need equally explicit provenance and behavior, rather than being silently mapped to an unrelated reward.

Generic REBEL also has overlapping escape and surrender threshold ranges. The data alone does not specify trigger ordering, chance sampling time, pause behavior, in-flight projectiles, post-refusal escape charge, or crew transfer during ceasefire. These require a documented runtime policy and interaction tests.

## Other audited candidates

- SQUAT_PIRATE_MERCENARY has simple LOW-standard destruction/crew rewards but surrender grants MED weapon loot. Its SHIPS_PIRATE pool contains 13 blueprints, most unsupported; substituting the two current Rebel hulls would alter the source pool.
- ZOLTAN_SCIENCE_DOCK has simple standard destruction rewards but a surrender that grants a Zoltan crew member, two unsupported Zoltan hulls and another root branch with MED stuff. It therefore also needs recruitment, rewards and hull/system support.
- Generic ROCK_SHIP and CRYSTAL_SHIP reuse DEAD_CREW_DEFAULT, so they do not avoid its recruitment/weapon branches; their hull pools are additional dependencies.
- Other pirate and quest descriptors include additional loot, quests or unsupported ship pools. None provides a context-faithful standalone surrender root requiring only a new surrender tag on top of the current playable content.

## Implemented compiler schema and rejection rules

Catalog schema 4 appends four fields to ImportedEventShip in this order:

`surrender_min, surrender_max, surrender_chance, surrender_node`

Absent surrender uses 0, 0, 0, -1. Min/max are required integers 0..1000 with min <= max. Chance is optional, accepts decimal 0..1 with at most four fractional digits, and is stored as an integer 0..10000. An absent chance receives 10000 as the explicit DS default. The source's presence or absence of the attribute is a data fact; treating absent chance as certain is a DS interpretation until engine behavior is independently verified.

Loaded and inline trees compile in the existing ship context. Every random alternative and choice continuation is checked. Replacement ships, unsupported effects, cycles, duplicate surrender elements, invalid bounds/chance, reopening combat after explicit acceptance, and resource/pursuit-bearing refusal paths reject the whole closure. At least one terminal path must explicitly execute hostile=false. Nested dialogue and acceptance rewards supported by the existing effect interpreter remain intact. Refusal may leave hostility unchanged or explicitly retain hostile=true.

The compiler does not claim to implement surrender runtime or enable the newly representable quest root. Unsupported surrender/escape/reward content still fails closed; coverage records remain excluded unless a complete root has been explicitly reviewed and enabled.

## Evidence and next coherent integration

All 34 compiler tests pass locally. New tests preserve weighted duplicates across 23 and 32 text entries and reject 33; they also cover decimal probability preservation, default chance, bounds, duplicate callbacks, loaded nested dialogue, all random branches, replacement ships, unsupported recruitment, post-acceptance attacks and rewards leaking into refusal. A real archive test compiles the full ENGI_UNLOCK_2FAKE closure with its original two-hull pool and confirms it remains outside SELECTED. It skips only when the owner's archive is unavailable; it ran in this environment.

The next coherent original playable addition is generic REBEL after its complete reward, recruitment/overflow and chance-escape dependencies are implemented. Surrender engine work can proceed independently with explicit fixtures while that root remains disabled. Do not present fixture success or compiler representation as a playable original surrender encounter.
