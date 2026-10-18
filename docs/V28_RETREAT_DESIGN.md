# V28 persistence update

Save21 now implements the carried-roster appendix described below: live and pending-death records, fixed slot ownership, strict field/identity/location/occupancy validation, old-format migration and backup preservation. Processed corpses are omitted. Restored home defense has no remote target, and continuation matches uninterrupted service across repeated disk reloads. See V28_PERSISTENCE_CHECKPOINT.md.

The temporary save20 rejection is superseded for promoted carried records. Unpromoted current-encounter home boarders still require explicit promotion before saving. Departure/common arrival, friendly abandonment, outcome ownership and player jump controls remain unfinished. The earlier audit sections are historical design notes.

# V28 player retreat: architecture audit and remaining work

## Implemented development update

The charge foundation and partitioned hostile roster are now implemented; see V28_INTRUDER_CHECKPOINT.md. Slots 0–7 belong to the current enemy; slots 8–55 hold carried home intruders. Promotion preserves living records and pending deaths atomically. Encounter replacement, service simulation, species clocks and last-slot rendering are covered by focused checks. Save20 rejects live or pending-death home intruders until the persistence appendix is implemented. No player retreat control is exposed.

Remaining in this plan: the persistent appendix, departure/arrival transaction, explicit friendly abandonment, campaign/boss outcomes and UI/earned retreat acceptance. The constraints and alternatives below are the original audit snapshot; the partitioned roster option was selected.

## Original audit status and scope

This is an implementation plan based on the current source, not proof that retreat exists. V27 is the latest release read during this audit. V28 work currently targets charge foundations and focused tests; the complete roster, campaign transition, persistence and UI flow below remains proposed until integrated and verified. No physical DS or emulator observations were performed by this audit.

The end state is a real departure to a linked beacon, carrying player damage and hostile intruders, with explicit consent before leaving friendly crew behind. A restriction requiring all hostile boarders to die before jumping would not satisfy this goal.

## Current constraints established by source inspection

- `campaign_can_jump` only permits CAM_MAP and refuses living friendly away crew. `campaign_jump` consumes fuel, advances travel/fleet counters and marks the destination visited before resolving its store, revisit, event, patrol or combat behavior.
- `start_combat` calls a void `game_begin_encounter`, then unconditionally changes campaign mode. The game function refuses living hostile home boarders and friendly away crew. Its caller cannot detect that refusal. A transition can therefore claim a new battle without creating it if the guard becomes reachable.
- Encounter construction clears the entire eight-slot enemy crew roster. The same roster currently represents ship defenders and enemies boarding the player. Removing its guard would silently delete intruders.
- Crew ownership and physical location are distinct: faction comes from the array; `Crew.ship` selects physical hull. Friendly ship1 crew must never be silently assigned to the replacement enemy hull.
- Enemy boarding AI recalls any low-health hostile ship0 crew using the current enemy teleporter. Without origin tracking, old intruders would acquire recall rights on an unrelated next enemy ship.
- `game_alive_enemy_crew` counts the entire enemy roster. Enlarging it with prior-encounter intruders without separating ownership would change next-enemy crew-elimination behavior.
- `crew_step(service=1)` excludes hostile movement. `boarding_step(service=1)` returns before melee and sabotage. Species ability clocks also partly skip hostile service activity. Intruders at a peaceful arrival would therefore freeze unless service simulation is extended.
- Main currently simulates noncombat management only on the crew/boarding page, and freezes event narrative and most menus. This is an explicit clock policy to preserve and communicate; changing campaign mode alone does not provide a playable boarder encounter.
- Saves are map-only and serialize player crew, but no hostile crew. Player crew location decodes only ship0. An unresolved retreat cannot be saved by pretending it is an ordinary map checkpoint.
- `boarding_prepare_departure` normalizes dead friendly ship1 records to valid home rooms, but does not itself process their deaths. A new abandon path must prevent delayed Zoltan death damage from appearing on the home ship after normalization.

## Dependency 1: separate hostile ownership with a proven capacity

Use a separate bounded carried-intruder roster, or an explicitly partitioned hostile pool with common iteration helpers. Keep three ownership groups conceptually distinct: player crew; current enemy crew; intruders whose original ship has been left behind. Physical location remains independent of ownership.

A partitioned pool can preserve current enemy slots0..7 and reserve additional slots for carried home intruders. A separate array makes ownership harder to confuse but requires replacing assumptions that all hostiles live in `enemy_crew`. Either is acceptable if all simulation and rendering paths iterate the correct sets.

The imported Kestrel has48 floor cells, verified by summing room width times height. Existing room occupancy limits allow at most one hostile per floor cell within its faction. Therefore48 carried-home slots plus8 current-enemy slots cover all physically reachable Kestrel occupancy, including repeated retreats, without dropping enemies. This proof depends on every event spawn, movement and teleport path enforcing occupancy. Validate future playable hull floor totals against the bound before enabling them; enlarge the bound from content analysis as necessary. Do not truncate a live roster or reject departure merely to hide capacity overflow.

Use stable crew IDs and preserve health, species, name, room, destination and relevant movement/combat/sabotage/ability timers. Reuse dead carried slots safely. IDs of new enemy crew must not collide with carried or friendly IDs.

Provide helpers for current-enemy alive count, all hostile home occupants, same-room hostile lookup, and owned current-enemy teleport selection. Current bitmask operations use `1u << index`; extending their loops beyond31 would be undefined. Keep teleport selection restricted to current enemy slots or replace it with bounded index lists. Resize the melee damage scratch array and audit initializer/species bounds.

Update fire, oxygen, breaches, melee, sabotage, doors, lockdown, species death bursts, sensors and renderer loops together. Carried hostiles cannot repair enemy systems, power the player reactor, occupy a new enemy pilot station, recall through the next enemy teleporter, or count as that enemy's defenders.

## Dependency 2: a departure operation that is safe before arrival

Introduce an explicit game departure operation and a success-returning encounter construction API. Validate destination, fuel, charge/readiness, campaign state and confirmation before any mutation. Failed or canceled departure must leave RNG, resources, crew, counters and the active encounter untouched.

On accepted departure:

1. If living friendly crew are ship1, show the actual names/count and an explicit abandon-and-jump confirmation. Cancel returns to combat; recall remains available. A generic Jump button must not grant abandonment consent.
2. Mark explicitly abandoned friendly crew lost, process/suppress remote death consequences before detaching their hull, and normalize dead records to valid home locations only for persistence. Never heal, recruit over, recall or revive those identities remotely. Replacement recruitment receives a fresh identity.
3. Move surviving current-enemy ship0 crew into the carried-intruder ownership group without changing their physical positions or health. Preserve existing carried intruders. Detach current-enemy ship1 crew and enemy-only state.
4. Clear encounter-local projectiles, enemy weapons, target references, drone deployment, outgoing burst queues and escape clocks. Preserve player hull, systems, fires, breaches, oxygen, doors, home crew and intended persistent charge/cooldown state according to documented rules.
5. Consume one fuel and advance actual travel/fleet exactly once; select and visit the destination once.

Require an explicit policy for all-player-crew-abandoned outcomes. Do not permit a successful ongoing campaign with no living friendly crew. Departure must never dispatch destroyed/deadCrew/gotaway callbacks belonging to the enemy, grant salvage, add station-defense rewards or advance a boss phase.

## Dependency 3: common arrival resolution without transient map tricks

Extract destination resolution from `campaign_jump` so normal travel and retreat share one implementation after validated departure. Do not temporarily change CAM_COMBAT to CAM_MAP merely to bypass guards: that can expose saving/management and reward callbacks in a half-transitioned state.

Clear the abandoned source event's transient ship/node/battle context; preserve its uniqueness bit because it was encountered. The destination then applies ordinary visited, captured, store, event and sector-pool rules exactly once. A revisit must not replay a fled encounter's rewards. A destination combat must successfully allocate the new enemy while retaining carried home intruders.

For a noncombat/event/store arrival, keep the normal narrative/menu clock policy but provide immediate access to a live home-defense view. During that view, simulate both friendly crew and hostile home intruders, including movement, melee, sabotage, oxygen/fire damage and defeat. Do not spawn a fake enemy hull or offer salvage for clearing old intruders. On the map, the player must still be able to depart with surviving intruders once normal travel requirements are met.

A current enemy being defeated while old intruders remain requires two independent states: the encounter result and ongoing home danger. Clear result ownership prevents repeated payouts; continuing intruder simulation must not repeat death callbacks or block a correctly earned enemy outcome forever.

## Dependency 4: finale and source-event semantics

The current finale starts at an exit, uses `boss_phase`, and advances on victory in `campaign_continue`. Retreat must neither award its between-phase repair/ammunition nor increment phase. Reset only abandoned encounter transients. Define revisiting the authored boss separately from the eventual moving flagship/base-defense design; document any temporary restart policy instead of implying persistent authentic flagship state.

Source events that need player-flight callbacks require explicit compiler/runtime support before entering the pool. For currently supported events, fleeing abandons their unresolved combat result without inventing a station reward or enemy-escape pursuit effect. Previously applied effects remain applied; no rollback or second grant.

## Dependency 5: save version and validation

Add an explicit carried-intruder appendix and bump the save version. Encode a bounded live roster with identity/species/location/health and relevant simulation timers. Validate every room and destination against the home layout, per-room occupancy, unique IDs, count bounds, species health limits, timer ranges and checksum. Old saves load with an empty carried roster.

A compact variable-length appendix avoids serializing unused slots. Calculate its worst-case byte size before retaining SAVE_CAP8192;48 full crew records can exceed remaining capacity. Increase the bounded buffer with a static memory report if needed, rather than silently losing state or producing unexpectedly unsavable runs.

Map checkpoints with carried intruders are legitimate only after the new appendix and home-defense resume behavior exist. Before that integration, reject saves with a precise reason; never accept a save that discards them. This is a temporary integration guard, not the full feature. Active combat remains unsavable under the existing policy until encounter persistence is explicitly implemented.

After reload, preserve home invaders and player danger while resetting discarded enemy-hull pointers, encounter-only weapons and all stale crew selections. Migration must preserve all old payload fields and backup bytes under the versioned policy.

## Dependency 6: UI and acceptance gates

Expose player FTL charge and readiness reasons separately from enemy escape. Provide a combat Jump action, linked destination selector, cancel path and explicit friendly-away abandonment confirmation. Selecting destinations or confirming must have one clear simulation pause policy. Route touch and buttons through the same validated API; suppress stale input after arrival.

Required focused cases include charge interruption/resumption, invalid destinations/fuel, cancellation without mutation, retreat with no boarders, mixed friendly-away and hostile-home crew, repeated retreats with multiple hostile origins, maximum occupancy, stale recall prevention, peaceful/event/store/patrol/combat arrivals, ordinary and original-event reward suppression, all-crew loss, Zoltan remote death, and boss retreat without phase rewards.

Add save/cold-load tests with active carried intruders and a source-version writer fixture. Inspect actual DS framebuffers for charge, destination selection, abandonment names, home-defense and arrival text. Run integrated earned campaigns, build the ROM and record static/runtime evidence separately. Physical DS, emulator input/audio/storage and full retreat completion remain pending until actually validated.
