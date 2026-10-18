# V37 enemy combat behavior

- [x] Species-relative healing and safe accessible medical routing.
- [x] Concurrent prioritized repair, healing and defense without boarding order overrides.
- [x] Bounded parties, useful sabotage targets and legal reactor power budgets.
- [x] Verify and implement original projectile/volley targeting (V38, normal difficulty; see [V38_WEAPON_AI.md](V38_WEAPON_AI.md)).
- [x] Verify boarding landing/recall/specialization (V39; see [V39_BOARDING_AI.md](V39_BOARDING_AI.md)).
- [x] Calm-ship healing and original manning-task priorities (V40; see [V40_CHECKPOINT.md](V40_CHECKPOINT.md)).
- [ ] Physical DS combat balance, pacing and streamed audio validation.

See [V37_COMBAT_AI.md](V37_COMBAT_AI.md); this is partial fidelity, not full parity.

# V36 audio checkpoint

- [x] Import a bounded local original audio roster: five tracks and twelve effects.
- [x] Stream music, mix bounded effects, preserve synth fallback and expose active backend.
- [x] Verify import reproducibility, embedded ROM payloads and host audio behavior.
- [ ] Validate physical DS volume, latency, sustained playback and flashcart loading.
- [ ] Add synchronized explore/battle transitions after runtime profiling.

See [V36_CHECKPOINT.md](V36_CHECKPOINT.md). Existing gameplay parity items below remain open.

## V33 software milestone

- [x] Replace all active rectangular enemy hulls with matching original art/rooms/doors, retaining explicit authored profile strengths.
- [x] Boot and in-run main menu, save-aware Continue/Resume, confirmed New Run, controls and session sound.
- [x] VBlank press capture, no held repeats, coordinate/context guards and immediate input presentation.
- [x] Defer routine diagnostic SD writes during play; retain explicit save/log/menu flush.
- [x] Focused tests, complete host regression suite, framebuffer inspection and ROM build.
- [ ] Physical DS input latency, frame-time, main-menu/readability/audio/storage acceptance.
- [ ] Authentic Flagship/base defense and remaining full-game scope below.

## V32 completed software milestone

- [x] Complete generic Rebel source closure, including all weighted surrender and crew-defeat reward branches.
- [x] Once-only offers, frozen decisions, safe acceptance, same-battle refusal and chance-based escape.
- [x] Save24 policy5 with historical policy scopes retained.
- [x] Zoomed current-sector view, irregular beacon placement, overview, shared touch/controller coordinates and accurate pursuit rings.
- [x] Focused source/compiler/combat/UI/persistence and earned-campaign validation.
- [ ] Broader difficulty/economy calibration and remaining full-game content below.
- [ ] Human controller-only campaign, emulator runtime/storage/audio and physical DS acceptance.

Earlier milestone sections below are historical task lists; V32 resolves the generic REBEL surrender dependencies recorded under V31.

## V31 enemy loadout milestone

- [x] Independent bounded enemy weapon slots, finite ammunition and enemy-only Artemis source import.
- [x] Supported campaign profiles, weapon disruption, sensor-gated charge/status feedback.
- [x] Focused combat, renderer, save and complete earned campaign checks.
- [ ] Complete surrender reward families, chance-based escape and original encounter callbacks before exposing offers.
- [ ] Broader original difficulty/economy calibration, additional loadouts and remaining full-game features below.
- [ ] Emulator runtime/input/audio/storage and physical DS acceptance of the revised build.

## V30 DS playtest fixes

- [x] Add portable D-pad combat/crew controls and Actions menu with focused tests.
- [x] Expose software volume presets with louder HIGH default after real-DS feedback.
- [x] Make later opening encounters respond to target choice and delayed input; retain reproducible measurements.
- [ ] Complete all management/confirmation/controller paths and human controller-only campaign acceptance.
- [ ] Confirm revised speaker loudness, clipping, readability and controls on a real DS.
- [ ] Match broader original FTL difficulty/economy/loadout behavior.


Higher software volume with four settings; D-pad crew/room navigation and Actions menus; larger selected menu text and preserved hazard feedback; later first-sector policy4 encounters now use supported burst profiles. See [V30_CHECKPOINT.md](V30_CHECKPOINT.md), [CONTROLS.md](CONTROLS.md) and [BALANCE.md](BALANCE.md). The user's real-DS report establishes that the prior mix was too quiet; revised hardware acceptance remains pending. Full-game work remains open.

## V29 combat and audio polish

User playtest fixes: softer synthesized music/effects with volume envelopes, less repeated aiming text, a selected-system card and distinct system-hit/shield-absorption feedback. Room targeting already caused system damage; new actual-weapon tests verify its consequences. Save23 and combat balance are unchanged. Current ROM: `ftl-ds.nds`; preserved copy: `releases/v29-polish/ftl-ds.nds`. Revised audio listening and physical DS checks remain pending. See [V29_COMBAT_POLISH.md](V29_COMBAT_POLISH.md).

## V29: original loot and civilian rescue

Original FIND_WEAPON and AUTO_CIVILIAN events are now playable, with all supported source alternatives retained. Weapon loot and named recruits are held until accepted or declined; replacing cargo or crew requires confirmation. Pending rewards hold simulation and block travel, event choices and saving. New runs use event policy 4; safe map checkpoints write save23 and preserve older save policies.

The current ROM is `ftl-ds.nds`; the packaged copy is `releases/v29/ftl-ds.nds`. The map header reads V29. See [V29_CHECKPOINT.md](V29_CHECKPOINT.md) for validation and limitations. Surrender foundations remain unexposed pending complete reward and chance-based escape support. The full base-game goal and hardware acceptance remain incomplete.

# Full base-game DS execution checklist

## Goal and status rules

Implement a complete, replayable base-game FTL demake for original Nintendo DS hardware. Preserve tactical interactions, progression, ship variety and the campaign structure while adapting controls, presentation and bounded simulation to the DS. Advanced Edition is outside this goal. The six-sector V10 subset is a baseline, not the definition of completion.

This checklist follows the accepted goal attachment `goal-objective.md` (October 3, 2026). It supersedes the old roadmap's descriptions of base-game features as optional or indefinitely deferred. Historical V10 completion evidence remains valid only for that subset.

Unchecked means incomplete or not yet proven against the full requirement. Code present, build success, a host test, an inspected framebuffer, emulator behavior and physical DS behavior are distinct evidence. Record each separately. A milestone is complete only after its implementation and stated gate pass; retain unresolved hardware gates explicitly.

## Baseline inspected for this plan

- [x] Read the accepted objective, README, ROADMAP, HARDWARE, IMPLEMENTATION and relevant current source. No applicable root/project/docs AGENTS.md was found.

- [x] Locate recoverable `checkpoints/v9-baseline.zip` and `checkpoints/v10-campaign.zip` with SHA-256 sidecars.

- [x] Inspect current source limits: six sectors, twelve nodes, six human crew, four weapons, five powered systems, player-only crew/doors/pathfinding, one enemy weapon.

- [x] Inspect recorded V10 evidence in `build/v10-evidence.json`: twelve host checks, 256 map seeds, three full runs and three resumed runs with 108 cold-load checkpoints. This is retained evidence, not a fresh rerun by this planning pass.

- [x] Record observed gaps: no persistent crew identity/species, living enemy crew, boarding, distinct battle-ending reasons, general ship layouts or complete original content interpreter.

- [x] Fresh baseline checks, previews and ROM build for the next milestone; retain logs and exact ROM hash.

- [x] Verify checkpoint archive hashes before the next release checkpoint.

V10 recorded ROM: 389120 bytes, SHA-256 `3071fa90c061a43c64e71a5f282e0654e112133ffa44ba0052ae3c24baf45465`. Recorded ARM9 static sections: 571916 bytes. Neither figure proves runtime memory safety or frame rate.

## Rules applied throughout

- [ ] Preserve installed FTL files and existing user saves; use read-only extraction from the owner's archive.

- [ ] Keep extracted assets and generated original content local. Record source IDs/hashes, imported values, verified behavior and authored approximations separately.

- [ ] Maintain a content coverage register: each ship, weapon, event, species, augment and hazard lists required mechanics and playable/tested status. Exclude unsupported combinations from every random pool, store, reward and hangar.

- [ ] Use subagents for independent ownership boundaries; integrate shared game/render/save interfaces deliberately.

- [ ] After meaningful integration: focused behavior tests, affected framebuffer inspection, ROM build, memory accounting when state/assets change, and emulator exercises when available.

- [ ] After every milestone: recoverable source/assets/ROM checkpoint plus hash; update README, roadmap, content rules and this checklist with evidence, limitations and exact next steps.

- [ ] Regularly run complete campaigns and disk save/cold-load campaigns using ordinary game actions. Do not use injected victories or free resources as evidence of playability.

- [ ] Keep pending hardware work separate and continue independent implementation when hardware/input is unavailable.

## Milestone 1 â€” Actions, shields and original enemy ships

Start here, as explicitly requested. Dependencies: baseline and existing V10 combat.

- [x] Immediate visual acknowledgement for touch and physical buttons, selection changes and successful actions.

- [x] Rejected actions give a short, accurate reason; feedback remains visible long enough to read without being overwritten by simulation chatter.

- [x] Feedback continues to age while paused and does not itself advance combat; touch targets and button navigation agree.

- [x] Player/enemy shield layer counts, maximum active capacity, recharge progress and impact feedback are unambiguous.

- [x] Show weapon charge, queued attacks, target locks and hull/system impacts clearly at DS resolution.

- [x] Import additional original enemy hull/layout data and suitable art; validate room/system/door references and coordinate transforms.

- [x] Introduce only enemy configurations supported by existing mechanics. Retain unsupported crewed/special-system originals outside the playable pool until later milestones enable them.

- [x] Add supported enemies to campaign pools with appropriate scaling and documented loadouts.

Gate: action rejection/acceptance tests; shield loss/recharge/power-damage tests; original enemy initialization/targeting tests; inspect both 256x192 framebuffers for each new layout; build ROM; verify a complete existing campaign still works. Input comfort on emulator/DS remains a separate gate.

## Milestone 2 â€” Ship, crew and outcome foundations

Dependencies: milestone 1. This foundation enables boarding, hazards and playable hulls without separate player/enemy rule sets.

- [ ] Explicit ship ownership and current crew location; bounded independent friendly/enemy rosters and stable crew identity.

- [ ] Per-ship doors, room connectivity, atmosphere, piloting and system definitions; remove Kestrel-only assumptions from simulation accessors.

- [ ] Bounded persistent names, rename/details UI, health, species, skills and current orders.

- [ ] Base-game species traits, health/speed/repair/combat/oxygen exceptions and relevant active/passive abilities.

- [ ] Crew capacity based on supported ship rules; recruit, dismiss, replace, group selection and group orders.

- [ ] Save/return stations, valid station fallback after roster/layout changes, visible skill progress.

- [ ] Separate hull, system and crew damage properties in attacks; implement fires, spread, firefighting, breaches, sealing and local hazards.

- [ ] Doors have appropriate resistance/control; functional sensor levels determine visible enemy information.

- [ ] Explicit result reasons for hull destruction, crew elimination, surrender and retreat, independent of presentation.

- [ ] Freeze terminal simulation, resolve rewards once, and handle simultaneous destruction/death deterministically.

- [ ] Player/enemy destruction sequences with blast/debris/fade; no destruction on surrender, escape or crew-only victory.

- [ ] Named crew animations for idle, directional movement, repairing, fighting, healing and dying; bounded animation assets.

- [ ] Update save schema with bounded explicit fields and migration or separate files before exposing persistent new state.

Gate: disconnected room/path tests; faction/location checks; damage/fire/breach/oxygen interactions; species exceptions; stable name/station round trips; repeated-input and terminal-result reward tests; six-to-full-roster UI previews; complete campaigns and ROM build.

## V13 progress against milestones 2â€“3

Implemented under integration validation: installable/upgradable powered teleporter, bounded parties, boarding both ways, faction melee/sabotage, recall, away-survivor departure guard, explicit abandonment, hull-only blast/debris presentation and save format 5 migration. Complete milestone gates remain open, including escape/surrender cases, species, resistant doors, sensor visibility and full original animation coverage. Six-human rosters and two-person parties remain subset limits.

The combined 27-check suite passed, including an earned-teleporter boarding campaign test; evidence is in build/v13-checks.txt. Earlier full-run drivers were updated to issue defender orders against hostile boarders. Final evidence belongs in V13_CHECKPOINT.md before closing a new gate.

## Milestone 3 â€” Living enemies and boarding

Dependencies: milestone 2; system installation UI can be integrated with milestone 4.

- [ ] Enemy living crews operate stations, repair, heal, fight and choose useful routes; automated ships retain appropriate behavior.

- [ ] Installable teleporter, power rules, cooldown, boarding party selection, deployment and recall.

- [ ] Boarding both ways; room occupancy and movement, melee, sabotage and faction-aware attacks.

- [ ] Doors/sensors/species affect boarding as intended; enemy boarders and impending boarding are readable when observable.

- [ ] Crew-only victory and its rewards; boarding automated ships remains possible with their atmosphere consequences.

- [ ] Correct dead/stranded/returning crew handling on escape, surrender, hull destruction and encounter transition.

- [ ] No duplication between ships or resurrection on recall/load; surviving crew identity and skills persist.

Gate: both sides board/recall, mixed factions in a room, medbay combat, door obstruction, oxygenless ships, teleporter damage, last-crew-away, simultaneous terminal outcomes and stranded-crew tests; inspect both ship views; complete a boarding-focused run.

## Milestone 4 â€” Full equipment and tactical systems

Dependencies: milestone 2; integrate with milestone 3 for boarding loadouts.

- [ ] Lasers, missiles, beams, ion weapons and bombs have distinct targeting, shield, evasion and damage interactions.

- [ ] Original-supported weapon catalog values, shield piercing, fire/breach chances, disabling effects and different hull/system/crew damage.

- [ ] General system installation/upgrade UI, reactor allocation, subsystem upgrades and per-hull slot/capacity limits.

- [ ] Piloting, engines, shields, weapons, oxygen, medbay, doors and sensors work consistently on both ships.

- [ ] Drone control, drone parts and supported offense/defense/repair/boarding drones with lifecycle and targeting rules.

- [ ] Cloaking, its timing/evasion/weapon interactions and counterplay.

- [ ] Augments and their relevant combat/economy/event effects with stacking/slot rules.

- [ ] Artillery and special shields for ships that require them.

- [ ] FTL charging, player retreat, enemy escape and surrender decisions; persistent consequences and pursuit effects.

- [ ] Complete equipment UI: install, unequip, reorder, swap, choose exact sale item and resolve excess loot without silent loss.

- [ ] Equipment ordering controls allocation priority; queue/charge state cannot transfer incorrectly when swapping or selling.

- [ ] Store stock supports systems, crew, weapons, drones and augments with relevant availability and ship limits.

Gate: interaction tests covering beams/shields, ion recovery, missiles/defense drones, cloak/charge, artillery/special shields and retreat/boarders; economy conservation and full-inventory tests; complete multiple tactical builds and save/resume them.

## Milestone 5 â€” Eight-sector exploration and original events

Dependencies: milestones 2â€“4 for supported event effects. Larger maps/fleet visuals may proceed independently after milestone 1.

- [ ] Eight sectors; larger generated maps (initial target 20â€“24 beacons), readable routes, map pan/overview and controller selection.

- [ ] Varied sector types and branching sector selection, persistent sector identity and appropriate content pools.

- [ ] Visible advancing Rebel front, next-jump threat indication, captured beacons, interception and exit pressure.

- [ ] Fleet advances/delays from travel, waiting, applicable events and sector rules; captures suppress normal services/rewards.

- [ ] Fuel, event density, stores and pursuit balance fit larger graphs; revisits cannot duplicate rewards.

- [ ] Supported original event interpreter/compiled representation with an explicit coverage register, bounded nesting and validated references.

- [ ] Meaningful resource, equipment, system and species conditions; blue choices, quests, consequences and chained outcomes.

- [ ] Stores and distress encounters use supported original content; unsupported events never enter playable pools.

- [ ] Active environmental hazards: solar flares, asteroids, nebula visibility and ion-storm power constraints, with consistent pause/reset rules.

- [ ] Persist graph, fleet state, quests, stock, event state and random generator; deterministic load continuation.

The user's earlier word "storm" meant the chasing fleet. Environmental hazards are now included by the full-game objective as a separate requirement.

Gate: many-seed graph connectivity and reachability, fleet/capture/exit tests, event condition/effect coverage, hazard interaction tests, no-repeat-loot checks; eight-sector full runs with alternative routes and cold loads; inspect large-map screens.

## Milestone 6 â€” Hangar and base-game ship coverage

Dependencies: milestones 2â€“4, enabling ships only when their defining mechanics work.

- [ ] Generalize player hull geometry, doors, furniture, cockpit/system positions, crew rendering, targeting, controls and save layout identity.

- [ ] Local import manifest enumerates all base-game playable ships and variants, excluding Advanced Edition-only entries.

- [ ] Hangar selection, ship descriptions, starting equipment/crew and supported loadout previews.

- [ ] Enable each base-game ship/variant after its defining systems pass: conventional weapons, boarding, drones, cloaking, artillery, species and special shields as applicable.

- [ ] Ship-specific equipment/system/crew limits; complete campaign/save compatibility per layout.

- [ ] Difficulty settings affect the documented economy/combat/campaign rules.

- [ ] Unlock quests, achievements, persistent unlock profile, scores and run history; profile updates do not overwrite run saves.

Gate: coverage register accounts for every in-scope ship/variant; each starts, plays its defining mechanic, traverses all screens and cold-loads correctly; representative complete runs for each play style and campaign acceptance for every supported layout. No unsupported placeholder loadouts count as completed ships.

## Milestone 7 â€” Final sector and authentic supported finale

Dependencies: campaign, boarding, drones, cloaking, special shields, weapon families and outcome handling.

- [x] Federation base defense with a moving flagship, pursuit schedule, interception routes and base-loss condition (V41; see [V41_FINAL_SECTOR.md](V41_FINAL_SECTOR.md)).

- [x] Imported flagship layout/art and phase-specific original-supported systems, weapons, crew and special behavior (V42 layouts/art, artillery, cloaking, crew, teleporter; V43 drones and drone surge; V44 super shield and laser surge). Default numbers are DS-tuned; the original normal numbers are a per-run menu choice (V45).

- [ ] Three phases with correct transitions, damage/crew persistence policy and phase hazards/surges.

- [x] Flagship behavior when crew are eliminated; prevent crew-only victories bypassing required phases (V42 AI takeover).

- [ ] Finale supports boarding, fleeing, return visits and repairs without duplicated rewards or phase resets.

- [ ] Campaign victory/defeat, score/history/unlocks and new-run flow resolve once.

Gate: win/loss/base-loss and all phase transitions, crew/boarding/escape edge cases, save policy enforcement, multiple ordinary complete campaigns at supported difficulties; inspect each phase and terminal animation.

## Milestone 8 â€” DS usability, audio and durable persistence

Dependencies: integrate throughout, then complete after full systems/content coverage.

- [ ] Enemy intent/alerts show charging weapons, escape, boarding and special attacks according to sensor visibility.

- [ ] Original local audiovisual assets where practical, with explicit DS adaptations and bounded loading/streaming.

- [ ] Audio effects/music and volume/mute behavior; audible verification on emulator and DS.

- [ ] Useful context help, tooltips and tutorial/practice encounter covering movement, power, shields, weapons, repairs, boarding and escape.

- [ ] Complete controller-only menu navigation plus touch; selection/focus and invalid action feedback remain consistent.

- [ ] DS lid/suspend/resume behavior freezes the right clocks and preserves input/audio/storage safety.

- [ ] Explicit encounter-save policy is documented and visible: either full bounded encounter persistence or clearly enforced safe checkpoints, without ambiguous resume behavior.

- [ ] Versioned save/profile migration or separate filenames preserves all existing V9/V10/user data; foreign files remain protected.

- [ ] Validate/checksum saves, atomic staged writes where supported, valid backup recovery and failed-write handling.

- [ ] Cold-load persistence of every supported state and corrupt/truncated/mismatched versions; user saves never used as test fixtures.

Gate: scripted input navigation and framebuffer audits across all pages, tutorial playthrough, suspended/paused behavior tests, storage failure/recovery tests and real listening. Emulator/physical claims require actual observation.

## Milestone 9 â€” Release acceptance and completion audit

- [ ] Run `tools/check.ps1`, `tools/preview.ps1`, `build.ps1` and `python tools/memory_report.py`; record exact outputs/ROM hash and fix application errors.

- [ ] Inspect affected previews from actual renderer, including every supported hull, maximum crew/equipment, long names, hazards, boarding and all finale phases.

- [ ] Integrated full campaigns cover alternative routes, difficulties, ship play styles, crew losses, equipment changes and save/resume combinations.

- [ ] Human playtesting covers balance and whether the DS controls communicate important choices in time.

- [ ] Measure original DS update-work timing and long-session frame pacing; test worst supported combat with effects, hazards, drones and crew.

- [ ] Measure stack/heap high-water and main-RAM/VRAM/asset/audio budgets; static size alone does not prove fit.

- [ ] Observe emulator boot, screen/input/audio and writable DLDI save/resume.

- [ ] Observe physical DS/flashcart boot, stylus/buttons, audio, lid handling, long sessions, SD interruption and recovery.

- [ ] Produce a recoverable release checkpoint, reproducible build instructions, content coverage report and explicit remaining approximations.

- [ ] Audit each objective requirement against current source plus appropriately scoped evidence; no unverified requirement is marked complete.

Current hardware status: emulator process startup was recorded for V10, but screen/input/audio/storage were not independently verified. Physical DS checks, runtime memory/frame timing and interrupted-SD durability are pending. A host preview is not an emulator screenshot; emulator timing is not physical DS timing.

## Milestone record template

For each completed integration, append date/version, files/features changed, test commands and outcomes, inspected screenshots, ROM hash/checkpoint, imported-versus-authored behavior, remaining limitations and the exact next implementation step. Keep the full scope above active until all requirements and release gates are satisfied.

### October 3, 2026 â€” Fire foundation prepared for integration

New `source/fire.c`, `source/fire.h` and `tests/fire_smoke.c` provide bounded room fires, ignition, oxygen consumption, vacuum extinction, crew damage, firefighting, system/piloting/enemy-drone-control damage and spread across open player doors. The strict standalone host check passes. Timings and spread probability are authored demake rules. Enemy spread is deferred until enemy door topology exists; there is no guessed connectivity between touching rooms. Fire rendering, integration into simulation/save/weapon hits, ROM build and integrated checks are separate pending evidence at this preparation point. This does not complete the fire/hazard or crew milestone.

## V11 checkpoint evidence

First software integration gate passed:17 focused host checks,256 generated maps, three full six-sector runs and three resumed runs with108 cold-load checkpoints. DS ROM built; affected Auto-Assault targeting and feedback framebuffers inspected. V10 saves migrate to format3 with fire defaults and byte-preserved old backup. Baseline archive hashes verified. See V11_CHECKPOINT.md for ROM identity and limitations. Milestone1 physical input comfort remains pending; full-goal hardware gates are unchanged. Foundational fire support and an enemy offensive drone are partial progress toward milestones2/4, not completion of those milestones.

## V12 progress against the full goal

Implemented and host-checked: separate enemy/player topology access, side-aware crew location/damage/atmosphere/fire, stable crew IDs/names/stations, rename/details UI, living original Rebel Fighter AI, result reasons and one-time crew-only rewards, save format4 with v2/v3 migration. Crew simulation and per-layout initialization have a common path, while player hull rendering remains Kestrel-specific.

Still incomplete within milestones2/3: species, eight-person roster, dismissal/group orders, door resistance/sensors, breaches/damage properties, directional/combat/death animations, destruction presentation, teleporter installation and boarding/recall/sabotage/stranded cases. No entire later milestone is marked complete from these foundations. Full evidence is in V12_CHECKPOINT.md.

## V13 progress against the full goal

Implemented and validated within the current six-human Kestrel subset:

- [x] Installable, upgradeable teleporter with power, damage/repair and cooldown; two-person selection, gather, deploy and recall controls.

- [x] Both factions board, fight and sabotage; living defenders pursue intruders and surviving hostile boarders remain after enemy hull loss.

- [x] Guard encounter departure while friendly survivors remain away; explicit confirmed abandonment and dead-away cleanup preserve identity without resurrection.

- [x] Hull-only destruction presentation, pause, one-time salvage and terminal-screen priority over stale menus.

- [x] Map-only save format 5, migration from formats 2-4, teleporter/timer validation and backup preservation.

- [x] 27 host checks, 256 generated graphs, three complete and three resumed campaigns with 108 cold loads, plus one complete campaign with an earned-scrap teleporter purchase and successful deployment/recall.

- [x] Affected framebuffer previews inspected; ROM built; static memory recorded; recoverable V13 archive and release created and hash-verified. See V13_CHECKPOINT.md.

These checks do not complete whole milestones 2 or 3: species/eight crew, resistant doors, sensors, full animation sets, escape/surrender and their stranded-crew cases remain pending. Runtime performance, emulator input/audio/storage and physical DS validation remain pending. Next implementation: separate hull/system/crew damage and breaches/sealing, then doors/sensors and species/roster expansion; fleet/maps can proceed independently.

### V14 verified software integration

- [x] Separate hull/system/crew damage and queued property snapshots; invalid projectiles reject without mutation.

- [x] Original raw fire/breach imports; corrected Heavy Laser I timing/power with catalog regression checks.

- [x] Breaches, connected-room vacuum, sealing priorities and medical/fire/melee interactions on both ships.

- [x] Captured-ship atmosphere and fire continue while friendly away crew remain; enemy AI and attacks stay stopped.

- [x] Save format 6, malformed breach rejection, formats 2-5 migration and exact backup preservation.

- [x] 30 host checks, 256 graph seeds, three complete and three resumed campaigns with 108 cold loads, plus the earned-teleporter campaign.

- [x] Affected DS framebuffer previews inspected, ROM built, static memory recorded, source/assets/ROM checkpoint hash-verified. See V14_CHECKPOINT.md.

No entire full-game milestone is declared complete. Functional doors/sensors are next, followed by species and eight-crew support; larger sectors/fleet may proceed independently. Original combat probability/damage calibration, full weapon/system/content coverage, runtime and hardware gates remain open.

### V15 verified software integration

- [x] Original Kestrel subsystem rooms, starting levels and upgrade costs; no reactor allocation or fabricated enemy descriptors.

- [x] Subsystem damage, repairs and sabotage; reinforced door forcing, temporary jams and idle recovery.

- [x] Repeated remote commands cannot reset forcing progress; upgraded closed doors reduce fire spread, with seeded comparison tests.

- [x] Sensor levels/local crew sight gate dynamic information while preserving static targeting identities; hidden-state framebuffer invariance and live charge tests pass.

- [x] Format 7 stores subsystem/door state; formats 2-6 migrate with exact old-save backups and malformed-state rejection.

- [x] 33 host checks, 256 map seeds, three full and three resumed campaigns with 108 cold loads, plus the earned-teleporter campaign.

- [x] Affected previews inspected, ROM built, static memory recorded and recoverable release checkpoint verified. See V15_CHECKPOINT.md.

Full-game acceptance remains incomplete. Next: species, eight-crew roster, their saves/animations/boarding interactions, then wider equipment and ship coverage. Nebula/species sensor interactions, manning behavior and runtime/hardware validation remain open; larger sectors/fleet can proceed independently.

## V16 integration evidence

Eight-crew and seven species foundations, six-species recruitment, group orders, dismissal, original species frames and save format 8 passed 36 host checks, renderer inspection and a ROM build. See V16_CHECKPOINT.md. Crystal acquisition, full animation sets, ordinary mixed-species campaign coverage and hardware validation remain open. This does not close the broader milestone gates.

## V17 integration evidence

Ion Blast I is now playable through stores; temporary power locks, shield interception, evasion, UI countdowns and save format 9 pass focused and campaign checks. 41 distinct host checks passed, including an earned-ion full run with 36 cold loads and two mixed-species recruited runs. See V17_CHECKPOINT.md. Beams, bombs, broader equipment/content, full eight-sector campaign and hardware acceptance remain open.

## V18 integration evidence

Halberd Beam now has bounded line targeting, per-room damage, shield attenuation, independent queued targets, store acquisition and format-10 persistence. 45 distinct software checks passed, including an earned-beam full run and 36 cold loads; actual V17 writer migration preserves the backup exactly. See V18_CHECKPOINT.md. Broader beam content, bombs, systems, campaign, ship coverage and hardware gates remain incomplete.

## V19 integration evidence

Small Bomb adds shield bypass, both-ship targeting, explicit system/crew damage and ammunition checks. All 49 host checks pass, including an earned-bomb campaign with 36 cold loads and actual V18-save migration. Source weapon coverage is now enumerated with unsupported definitions disabled. See V19_CHECKPOINT.md. One selected example in each of five weapon families does not complete the full equipment/content milestone.

## V20 equipment evidence

Unequip, reorder, exact-item sale confirmation and zero-weapon save/resume passed 53 combined checks, ROM build and inspected equipment previews. The earned equipment campaign completed six sectors with 38 cold loads. Legacy priority migrates in format 12. See V20_CHECKPOINT.md. Milestone 4 remains open: pending loot, drones, augments, cloaking and remaining tactical systems are incomplete. The eight-sector campaign and hardware acceptance remain open.

## V21 drone foundation progress

Player control, Combat I deployment/parts/power/damage/repair and format-13 persistence pass 55 checks plus strengthened integration tests. Store/UI/inventory and earned drone campaign are still pending; no player drone pool is enabled. See V21_DRONE_PROGRESS.md. Drone, equipment and full-game milestones remain open.

## V21 player drone checkpoint

Combat I is available through stores with parts and control installation/upgrades. Two installed slots, four separate drone cargo entries, exact sales and touch/controller deployment passed 59 host checks, final UI test, framebuffer inspection and ROM build. An earned six-sector drone run made five deployments and 36 cold loads. Save14 includes stock/cargo; actual archived format13 migration preserves ownership and backup. See V21_CHECKPOINT.md.

Full drone coverage remains open: defense, repair, boarding/interior roles, movement/vulnerability and unified player/enemy lifecycle. Shared original cargo limits need reconciliation. Eight-sector campaign, other systems/ships, original finale and hardware acceptance remain unfinished.

## V22 defense drone evidence

Defense I purchase/fitting, power and parts, missile interception, visible result and save format 15 passed 63 host checks, strengthened focused checks, inspected previews and ROM build. Earned campaign observed one actual missile interception across three deployments and 36 cold loads. Other projectile families are excluded; multiple defense drones and full projectile pools are covered in focused tests. Full drone mechanics, other systems, campaign/ships/finale and hardware gates remain open. See V22_CHECKPOINT.md.

## V23 campaign and fleet evidence

New campaigns now contain eight sectors with 23 beacons each, all visible in a compact map overview with touch/controller selection. Current and next-turn fleet fronts, captured stores/exits, patrols, wait/rescue advance and denied normal rewards are implemented. Legacy saves retain 12/6 policy through format 16. 1,024 seed graph checks and three earned eight-sector campaigns passed, with 195 jumps/cold loads. Full 67-check suite, final fleet UI check, previews and ROM build passed. See V23_CHECKPOINT.md.

Sector identity/types, branching sector choices, original event interpretation, active hazards and authentic final-sector/base-defense/flagship remain open. Prior six-sector tactical regression fixtures now explicitly initialize legacy policy; new eight-sector evidence is recorded separately.



## V28 player retreat integration

- [x] Player charge, pure departure checks and atomic campaign arrival; fuel/fleet costs once with no abandoned-battle salvage.
- [x] Carried hostile identity/death state, explicit named away-crew confirmation and home defense during unresolved arrival narrative.
- [x] Touch and button retreat selection, frozen destination map, charging feedback and terminal-screen priority.
- [x] Save22 stores current authored finale retry phase; older versions default to unstarted and preserve backups.
- [ ] Earned boarding-retreat campaign coverage, observed emulator input/audio/storage and physical DS validation.
- [ ] Surrender, full equipment/content coverage, authentic moving flagship, other playable ships, progression and remaining full-goal gates.

See V28_CHECKPOINT.md for validation and checkpoint identity. Current authored boss retries restart that phase at full hull; this does not satisfy the authentic finale requirement.
