# V25: bounded original event interpreter

## Scope and provenance

The source is the owner's installed `ftl.dat`, opened read-only by `PkgArchive`. `tools/event_compile.py` reads twelve base-source event containers and three English text containers. Generated original data and text remain local in `generated/event_catalog.h`, `event_catalog.json` and `event_coverage.json`; do not redistribute these files or a ROM containing them.

The current installed archive contains later edits inside base-source files. File names alone do not establish pre-Advanced-Edition eligibility. Six familiar base-game economy encounters were individually selected; all other named event definitions remain excluded. The coverage file lists **351 named event definitions**, including named helper events. This is a register of these twelve source containers, not a claim to enumerate every historical base-game root or event in DLC/expansion containers.

Each source file has its byte length and SHA-256 in the generated catalog. Original event IDs, English narrative, choices, resource ranges, hull repair amounts, Engi requirement and references are imported facts. Runtime timing, reward distribution and UI presentation are separate demake policies.

## Enabled complete closures

| Root | Source | Imported behavior |
|---|---|---|
| REFUEL_STATION | events.xml | Four choices: buy 6/3/1 fuel for 12/6/2 scrap, or leave |
| REPAIR_STATION | events.xml | Four choices: repair 20/10/5 hull for 40/20/10 scrap, or leave |
| SELL_MISSILES_STATION | events.xml | Four choices: sell 5/10/15 missiles for 15/30/45 scrap, or leave |
| SELL_DRONES_STATION | events.xml | Four choices: sell 3/6/12 drone parts for 12/24/48 scrap, or leave |
| FUEL_FOR_DRONE | events.xml | Three original introductory text variants; exchange 2-4 fuel for 1-3 drone parts, or decline |
| NO_FUEL_REFUGEE_DAMAGED | events_fuel.xml | Four choices: buy fuel; Engi damage appraisal opens a better offer; refuse resolves a two-entry event list; threaten the refugees triggers immediate surrender and MED fuel reward |

The fuel refuge is restricted to an out-of-fuel context. Its ordinary purchase gives 3 fuel for 10 scrap. Engi negotiation opens 6 fuel for 10 scrap. The refusal list retains both the no-help outcome and a second nested offer; no branch is discarded. Unique flags are imported and must be tracked persistently across the run.

System/equipment blue-option roots are not enabled in this milestone: inspected candidates include unsupported combat, augment/item rewards, quest/unlock effects or AE requirements. They must remain excluded until their full closures work. The current catalog implements the Engi species condition; `IE_REQ_SYSTEM` and `IE_REQ_WEAPON` enum values reserve a future interface and do not claim imported coverage. No quest root is enabled. The earlier authored courier remains separate.

## Compiler contract

The output has 6 roots, 41 nodes and 30 choices. Node text reaches 249 characters; the longest choice reaches 53. Current text fits the paged UI limits without truncation.

- `ImportedEventRoot`: ID, initial node, unique/distress flags, fuel-only context.
- `ImportedEventNode`: full narrative and bounded slices of choices, effects and random alternatives.
- `ImportedEventChoice`: full label, destination, requirement kind/value/level.
- `ImportedEventEffect`: resource/hull kind and inclusive minimum/maximum.
- Effect kinds are fuel=0, scrap=1, missiles=2, drone parts=3, hull=4. Positive hull values repair; imported negative damage becomes positive repair.
- Species values are Human=0, Engi=1, Mantis=2, Rock=3, Slug=4, Zoltan=5, Crystal=6, matching the game. Only Engi occurs in the enabled closures.
- Random nodes have alternatives and no choices/effects. Text lists are expanded into alternatives with complete duplicated choice/effect closures, preserving all text variants.

Hard compile bounds: depth 16, total 512 nodes, four choices per node, eight effects per node, 16 random alternatives, 1,600 characters of narrative and 114 characters per label. Labels must also fit the actual renderer's 38-column word wrapping: at most three rows, or two when an immediate effect/random closure needs the resource hint row. Effects behind later choices do not use the current hint row. References, choices, unknown tags, unknown attributes, invalid integer ranges and cycles fail closed. If any branch fails, its complete root is excluded. Mixed `load` and inline data are rejected. Random-list filters, weights and per-entry requirements are not silently ignored.

The coverage register records the first exclusion reason for each root. A structurally supported tree is still excluded unless explicitly reviewed and whitelisted. This prevents accidentally enabling tutorial/debug/endgame helper events as ordinary encounters.

## Intentional DS adaptations

- Source unique flags use campaign-wide suppression in this demake. The original engine reset scope is not verified; this scope is an explicit DS policy.
- Random event/text-list alternatives are selected uniformly. Inclusive resource ranges are sampled uniformly. This is a defined demake policy, not verified PC engine random weighting.
- Choice inspection consumes no random numbers. Affordability requires enough resources for the worst possible immediate debit, including every immediate random alternative. Future choices are checked when reached. Costs and rewards are committed together once.
- An Engi condition means a living Engi aboard the player ship. Blue conditions stay visible and disabled when unmet; source `hidden` attributes do not hide them. This helps a DS player understand why a choice is unavailable.
- Root narrative/choice wording is imported. Whitespace and typographic punctuation are normalized to the renderer's ASCII glyphs. Decorative `planet` and `back` text-list attributes are omitted; gameplay effects are not omitted.
- Hull repairs cap at current maximum hull. Resource additions obey the campaign's bounds. Outcomes display actual changes, including repair clamping.
- The source comment in `data/events.xml` describes `fuel` automatic rewards as scrap plus that resource, distinguishing `fuel_only`. It does **not** supply the original numeric distribution. The sole enabled automatic reward (`MED fuel`, from the refugee attack/threat branch) is authored as **3-6 fuel plus 15-25 scrap**. These ranges are explicit in `AUTO_REWARDS`, not recovered original reward math. Other automatic reward kinds/tiers remain unsupported.

## Validation and remaining scope

`python tests/event_compile_test.py` passes 17 tests: resource/hull translation; species/nested choices; four-choice preservation; missing/cyclic references; full random alternatives; rejection of an unsupported alternative; complete text-variant effects; missing text; unsupported effects/attributes/resources; malformed ranges/choices; depth/text/choice bounds; complete generated graph/header integrity; the post-recursion node limit; malformed/empty text and unique flags; and exact choice wrapping with immediate random effects. The tests verify every generated node is reachable from an enabled root and all slices/edges are in bounds.

Regeneration was performed against the installed archive. A source-hash catalog plus deterministic repeat generation can verify provenance and reproducibility. These compiler checks do not prove runtime campaign integration, original reward fidelity, human balance or DS acceptance; those need their own evidence in the checkpoint.

Next: original combat event closures, system/equipment conditions, quests and persistent consequences, additional base-source eligibility review, original sector pool mapping, and active hazards. Unsupported roots remain out of the playable pool until their complete dependencies are implemented.
