# V29 original reward closures

## Source and scope

Read-only audit of the owner's installed `ftl.dat`. Generated catalog and coverage files record the source file hashes. Original extracted text stays local. XML proves the event structure and symbolic reward family; it does not establish the original executable's payout math, rarity weighting, or random selection algorithm.

This compiler integration appends `FIND_WEAPON` at root 9 and `AUTO_CIVILIAN` at root 10. Root IDs 0 through 8 remain unchanged. Runtime, overflow interaction, persistence policy, and hardware validation are separate integration work; compiler tests alone do not prove those behaviors.

## Complete enabled closures

### FIND_WEAPON

`data/events.xml`: one event with `text load="FIND_WEAPON"` and `autoReward level="LOW">weapon`.

The text list has six entries, `text_FIND_WEAPON_1` through `_6`. All six remain in the compiled closure, each with the complete weapon effect. Existing decorative planet/background omissions also apply here. There are no quests, ships, conditional choices, or environment effects.

### AUTO_CIVILIAN

`data/events_rebel.xml`: nonunique event with two original choices. Intervene loads hostile `REBEL_AUTO_CIVILIAN`; ignore ends with its original text. Neither branch is removed.

`data/events_ships.xml`: `REBEL_AUTO_CIVILIAN` uses `SHIPS_AUTO`, resolved through `data/autoBlueprints.xml` to supported `AUTO_BASIC` and `AUTO_ASSAULT`. There is no surrender or escape descriptor. The destroyed callback grants LOW standard resources; the deadCrew callback grants MED standard resources. Both preserve their original text and explicit Continue choice loading `SAVE_CIVILIAN_LIST`.

`data/events_pirate.xml`: all six `SAVE_CIVILIAN_LIST` alternatives remain:

1. Original recruitment offer with accept and decline choices. Accept grants `crewMember amount="1"`; decline has no effects.
2. MED standard resources.
3. LOW standard resources.
4. Original text and Continue choice granting LOW weapon.
5. Hull repair of 5 (`damage amount="-5"`).
6. Original text with no reward.

No unsupported ship, quest, blue condition, or simultaneous crew/weapon reward is hidden in these closures. Both ship callbacks are compiled even though an automated enemy normally has no killable crew.

## Compiler schema 5

Existing effect kinds 0 through 4 keep their meanings. Appended effects use the existing `(kind, min, max)` record:

- `IE_WEAPON = 5`: LOW weapon becomes `(5, 0, 6)`.
- `IE_CREW = 6`: unspecified `crewMember amount="1"` becomes `(6, 0, 5)`.
- An explicit supported crew `class` becomes `(6, species, species)`, including explicit Crystal class. Other crew amounts, unknown attributes, unknown species, nested children, and nonempty text fail closed.

Each immediate concrete event permits at most one pending object effect. Every random alternative is compiled and checked; a bad alternative rejects the whole closure. A later explicit choice is a separate action after resolving the pending reward. Simultaneous crew plus weapon, or duplicate object rewards, remain unsupported until a reward queue exists. Resources and a single object can coexist.

LOW weapon selects uniformly among the seven current DS catalog IDs: Artemis, Burst Laser II, Basic Laser, Heavy Laser I, Ion Blast, Halberd, and Small Bomb. Unspecified recruitment selects uniformly among Human, Engi, Mantis, Rock, Slug, and Zoltan. Crystal requires an explicit source class. These are intentional bounded DS approximations. In the original blueprints Artemis and Basic Laser have rarity 0; Slug also has rarity 0. The declared DS pool deliberately includes them and must not be described as an exact original random loot pool.

The runtime must reserve the random offer once, including crew identity/name, and retain it through accept, replace, or decline. Pending offers must block subsequent event choices and travel until resolved; those are integration requirements, not properties proven by this compiler.

## Remaining generic REBEL dependencies

The generic `REBEL` ship has surrender chance 0.5 with min 2/max 3, escape chance 0.5 with min 3/max 4, destroyed `DESTROYED_DEFAULT`, and deadCrew `DEAD_CREW_DEFAULT`. Its escape descriptor lacks an explicit timer and gotaway callback, so current timer-based support cannot honestly stand in for it.

`PIRATE_SURRENDER` acceptance grants RANDOM stuff. Its 23 weighted text-list entries remain available to the compiler; refusal is an empty event. Generic REBEL remains excluded until every branch and the escape semantics have support.

`DEAD_CREW_DEFAULT` has nine source alternatives, preserving these duplicate weights:

- Three MED standard alternatives.
- Two HIGH standard alternatives.
- Two HIGH fuel alternatives.
- One unspecified crew member plus LOW scrap_only.
- One LOW weapon.

This milestone adds the last alternative's weapon primitive and the crew primitive, but does not add HIGH fuel, LOW scrap_only, or RANDOM stuff. Future numeric ranges and RANDOM tier probabilities must be marked DS economy approximations unless verified separately. No reward-definition file was found in the installed archive; the event XML supplies the family and level only.

Other candidate exclusions: `FEDERATION_BASE_ASSIST` can load an outcome with both crew and weapon immediately, exceeding the single-pending-object contract; `ZOLTAN_QUEST_PRIMITIVES` includes an unsupported Zoltan enemy branch. Enabling only their convenient subevents would lose source context.

## Evidence

`python tests/event_compile_test.py`: 37 tests passed after local regeneration. Tests cover strict crew parsing, unchanged symbolic reward rejection, duplicate object bounds, failure of a whole random list with an invalid branch, all six weapon intros, both AUTO_CIVILIAN callbacks, every civilian result alternative, and retained recruit decline.

Generated catalog: 11 roots, 98 nodes, 47 choices, 4 ship descriptors. Existing root and descriptor IDs remain stable. Coverage still reviews 351 named events. This note makes no emulator or physical DS claim.
