## V33 enemy hulls, main menu and responsive input

Every active enemy profile now uses original FTL hull artwork with matching source room geometry and doors. The three prior authored rectangles reuse Auto-Scout, Auto-Assault and Rebel Rigger layouts while retaining authored strengths. Boot now offers Continue/New Run/Controls/Sound; menu and result navigation preserve saves and hold gameplay.

VBlank input capture retains distinct quick presses during rendering, preserves coordinates, prevents held repeats and rejects old-view actions across screen/confirmation changes. Automatic diagnostic SD writes are deferred during play. Host checks establish queue semantics, not real hardware latency. See [V33_CHECKPOINT.md](V33_CHECKPOINT.md), [V33_SHIP_ART.md](V33_SHIP_ART.md), [V33_MAIN_MENU.md](V33_MAIN_MENU.md) and [INPUT_RESPONSE.md](INPUT_RESPONSE.md).

Next: real-DS rapid tapping/menu/save validation and runtime profiling, broader balance and remaining controller management, supported original content and the authentic Flagship finale.

## V32 surrender, escape and sector map

New policy5 campaigns include the complete original generic REBEL closure: weighted surrender choices, original hull/crew reward branches and chance-based escape. Acceptance pays once and ceases fire; refusal resumes the existing ship, queues and escape charge. Event choices hold simulation. Source reward amounts and missing escape timing use explicit DS policies. Save24 retains old encounter-policy bounds and clears encounter-only state on cold load.

The map now uses irregular seeded positions and a zoomed current-sector view with full overview. Selection, routes and touch share coordinates; capture rings use actual campaign pursuit predicates. This changes presentation without changing connections, RNG or pursuit rules. See [V32_CHECKPOINT.md](V32_CHECKPOINT.md), [V32_MAP.md](V32_MAP.md) and [V32_EVENT_SOURCE.md](V32_EVENT_SOURCE.md).

Next: broader campaign economy/difficulty measurements, controller-only management, original hazards/equipment/enemy coverage, then the authentic moving Flagship/base-defense finale. Emulator and physical DS validation remain open.

## V31 enemy loadouts

Supported policy4 generated encounters from sector 2 now use independent laser pairs, laser/missile pairs or laser with drone support. Enemy Artemis data is imported separately from player Artemis; missile ammunition is finite. Weapon damage and ionization can suppress individual slots. The display shows threat families, with individual charge and state behind upgraded sensors. Pair selection and reactor allocation are documented DS choices, not verified original generation logic. See [V31_CHECKPOINT.md](V31_CHECKPOINT.md) and [ENEMY_LOADOUTS.md](ENEMY_LOADOUTS.md).

Next: finish surrender reward families and chance-based escape, then enable complete original encounter branches; expand earned-run balance evidence and real-DS acceptance. Unsupported surrender content remains excluded.

## V30 DS playtest fixes

Higher software volume with four settings; D-pad crew/room navigation and Actions menus; larger selected menu text and preserved hazard feedback; later first-sector policy4 encounters now use supported burst profiles. See [V30_CHECKPOINT.md](V30_CHECKPOINT.md), [CONTROLS.md](CONTROLS.md) and [BALANCE.md](BALANCE.md). The user's real-DS report establishes that the prior mix was too quiet; revised hardware acceptance remains pending. Full-game work remains open.

## V29 combat and audio polish

User playtest fixes: softer synthesized music/effects with volume envelopes, less repeated aiming text, a selected-system card and distinct system-hit/shield-absorption feedback. Room targeting already caused system damage; new actual-weapon tests verify its consequences. Save23 and combat balance are unchanged. Current ROM: `ftl-ds.nds`; preserved copy: `releases/v29-polish/ftl-ds.nds`. Revised audio listening and physical DS checks remain pending. See [V29_COMBAT_POLISH.md](V29_COMBAT_POLISH.md).

## V29: original loot and civilian rescue

Original FIND_WEAPON and AUTO_CIVILIAN events are now playable, with all supported source alternatives retained. Weapon loot and named recruits are held until accepted or declined; replacing cargo or crew requires confirmation. Pending rewards hold simulation and block travel, event choices and saving. New runs use event policy 4; safe map checkpoints write save23 and preserve older save policies.

The current ROM is `ftl-ds.nds`; the packaged copy is `releases/v29/ftl-ds.nds`. The map header reads V29. See [V29_CHECKPOINT.md](V29_CHECKPOINT.md) for validation and limitations. Surrender foundations remain unexposed pending complete reward and chance-based escape support. The full base-game goal and hardware acceptance remain incomplete.

# V28 player retreat integration

Game departure preflights and promotes home intruders before detaching the remote encounter. Campaign travel stages one bounded static Campaign, resolves arrival privately and commits only on success. There is no transient public map-mode bypass. Current encounter outcomes count current hostile ownership separately from carried intruders.

Ui retreat fields are transient. ui_campaign_step owns the shared freeze/service policy used by the DS main loop and host integration tests. Explicit home defense can service unresolved arrival events without enabling stores/upgrades or advancing narrative. Modal result rendering and main-loop cleanup prioritize defeat.

Save22 appends a signed boss_resume_phase after save21's variable carried roster. Older versions default to -1; phases 0..2 are accepted only in the final sector. This persists the interim authored retry policy, not the future authentic flagship state.

See V28_CHECKPOINT.md. The records below are historical and do not redefine current completion status.

# V27 current integration: warning scouts and enemy escape

AUTO_WARNING joins the supported original pool with all nine introductions, a frozen escape warning, engine/pilot-dependent charging and distinct destruction/departure callbacks. Escaping scouts advance the fleet once and yield no salvage. Crew left aboard are lost once; surviving hostile boarders remain a combat threat. The countdown, stopped charge and departed ship are visible without a hull explosion.

Six earned eight-sector campaigns completed 389 jumps/cold loads, 70 original decisions and 11 original battles, including one actual escape and its pursuit consequence. Actual V26 save migration preserves the payload and backup exactly. The final audit also preserves home-target bombs during post-escape boarding combat. All 80 C checks and 28 compiler tests, the final UI check and ROM build pass. See [V27_CHECKPOINT.md](V27_CHECKPOINT.md).

Full scope remains open. Next: player FTL charging/retreat and surrender, then original quests/conditions and active hazards. Physical DS and emulator input/audio/storage acceptance remain pending. Historical records below retain their original scope.

# V26 current integration: original station-defense encounters

Two complete original combat-event closures join the six economy roots. New runs can intervene or avoid a station attack, read the original precombat response, then choose Engage. Original hull/crew outcomes and a separate station reward resolve once. Exact source pools include Auto-Scout, Auto-Assault, Rebel Fighter and the newly imported Rebel Rigger. Fixed loadouts, reward distributions and placement remain documented adaptations.

All 76 C checks and 24 compiler tests passed. Six earned eight-sector campaigns completed 392 jumps/cold loads, 68 original choices and nine station battles covering both roots and all four enemies. Those nine battles were hull victories; crew-only callbacks have focused evidence only. Save19 preserves actual V25 payload and backup bytes, retaining old content policy. Affected renderer previews were inspected, clipping corrected and the ROM built. See [V26_CHECKPOINT.md](V26_CHECKPOINT.md).

Full scope remains open. Next: enemy FTL charging/escape and AUTO_WARNING's pursuit consequence, then original quests/conditions and active hazards. Physical DS and emulator input/audio/storage acceptance remain pending. Earlier version records below retain their historical scope.

# V25 current integration: original event interpreter

Six complete original roots (41 nodes/30 choices), resource/repair effects, random lists/text, Engi negotiation and four-choice UI are playable in new runs. Optional supply/repair beacons use supported roots; out-of-fuel WAIT provides the refugee encounter outside captured territory. Save18 stores event policy and uniqueness while preserving older rules.

All 73 C checks and 17 compiler tests passed. Six earned eight-sector campaigns completed 393 disk cold loads, 41 original decisions and 2 actual trades. Coverage includes five ordinary roots; the fuel-only root has focused evidence. Actual V24 migration, inspected framebuffers and ROM build passed. See [V25_CHECKPOINT.md](V25_CHECKPOINT.md).

Full scope remains open. Next: original combat-event closures, system/equipment conditions, quests and persistent consequences, then active hazards and appropriate sector pools. Historical entries below keep their original evidence limits.

# V24 current integration: sector routes

Persistent Civilian/Engi/Rebel/Pirate identities, two-way exit choices, weighted supported pools and Up/Down/A event navigation are implemented. Save format 17 retains new route metadata and old campaign rules on migration. All 70 checks passed; six earned eight-sector campaigns completed 388 disk cold loads across both branch positions. Actual V23 writer migration, final UI checks, inspected previews and ROM build passed. See [V24_CHECKPOINT.md](V24_CHECKPOINT.md).

Full milestones remain open: original events and wider sector coverage, active hazards, remaining systems/ships, authentic finale, progression and DS release validation. Next: supported original event conditions/effects and quests, then active hazards. Older entries below retain their original evidence limits.

# V23 current checkpoint: eight sectors and the pursuing fleet

New campaigns use eight sectors with 23 beacons each. Fleet shading, next-turn danger, captured services/exits and WAIT are integrated. Existing saves retain their six-sector/12-beacon policy. All 67 registered checks passed; three earned new campaigns completed 195 jumps and 195 disk cold loads. Final fleet UI, inspected previews and ROM build passed. See V23_CHECKPOINT.md for evidence and limits.

Next: sector identities/types and branching sector choice, supported original event interpretation and active hazards. Authentic Federation-base/flagship finale, remaining systems/drone types/ships, progression and hardware acceptance remain required. The full-game goal stays active.

# V22 current checkpoint: Defense I

Defense I is available on store page four for 50 scrap. It uses two control power and a part per encounter; each ready drone intercepts one incoming missile with an authored one-second recharge. Other attack types remain unaffected. Save format 15 preserves Defense schematics and stock. All 63 host checks passed, including an earned campaign with a real interception and 36 cold loads. See V22_CHECKPOINT.md.

Next: remaining drone types and movement/vulnerability, shared inventory/loot handling, cloaking/augments and ship-specific systems. Eight sectors, fleet, events, hangar, authentic finale and hardware acceptance remain required. Full-game goal remains active.

# V21 current checkpoint: player Combat I drones

Combat I acquisition, control upgrades, parts, separate drone cargo, touch/controller management and format14 saves are integrated. All 59 host checks passed, including an earned drone campaign with five deployments and 36 cold loads. Final UI checks, inspected previews and ROM build passed. See V21_CHECKPOINT.md for evidence and limitations.

Next: defense drones and projectile interception, remaining drone lifecycle types, cloaking and augments. Shared inventory limits, orbital vulnerability and remaining original behavior need reconciliation. Eight sectors, fleet, original events, hangar and flagship remain required; emulator/physical DS acceptance remains pending.

# V21 development: drone foundation

Player drone control/simulation and format-13 persistence pass 55 host checks, strengthened integration tests and a ROM build. Acquisition, inventory, UI and an earned drone campaign remain unfinished; no new player drone content is in campaign pools. V20 remains the latest playable release. See V21_DRONE_PROGRESS.md for evidence and exact next steps. The full-game goal remains active.

# V20 current checkpoint

Weapon removal, displayed-order power priority, exact-item sales with confirmation, and safe zero-weapon loadouts are integrated. All 53 registered host checks passed, including an earned equipment campaign with 38 disk cold loads. Save format 12 migrates formats 2â€“11, preserving legacy weapon priority. ROM build and equipment framebuffer inspection passed. See V20_CHECKPOINT.md for evidence and limitations.

Next: drone control, drone inventory and lifecycle, then cloaking/augments and corresponding ships. Larger eight-sector maps, visible fleet, original events, hangar and authentic finale remain required. Excess equipment rewards still need a pending-loot interface. Emulator and physical DS acceptance remain pending; FULL_GAME_CHECKLIST.md governs the full objective.

# V19 current checkpoint

Small Bomb, HOME/ENEMY targeting, distinct teleport effects, ammunition accounting, store acquisition and save format 11 are integrated. All 49 host checks passed, including an earned-bomb six-sector run with 36 cold loads. Actual V18 writer migration preserves its backup exactly. ROM build and affected framebuffer inspection passed. See V19_CHECKPOINT.md for evidence and explicit approximations.

A persistent weapon coverage register accounts for 49 named source definitions without enabling unsupported entries. Next: complete equipment removal/order/sale controls, broaden reviewed weapon and enemy coverage, then remaining systems/campaign/hangar/finale work. Hardware/emulator acceptance remains pending; FULL_GAME_CHECKLIST.md is still the full-goal definition. Historical entries retain their original scope.

# V18 current checkpoint

Halberd Beam, real point targeting, shield attenuation, sweep feedback, store integration and save format 10 are verified in software. 44 combined checks plus an earned-beam full campaign passed (45 distinct checks); that run includes 36 cold loads. Actual archived V17 saves migrate with byte-identical backup preservation. ROM build and affected framebuffers passed. See V18_CHECKPOINT.md for approximations and evidence.

Next: bombs, complete equipment controls, broader systems/content and the remaining campaign/hangar/finale requirements. Hardware and emulator acceptance remain pending. FULL_GAME_CHECKLIST.md remains the full-goal definition; historical entries below retain their original scope.

# V17 current checkpoint

Ion Blast I, power lockouts, source weapon catalog, sensor-aware countdowns, ninth store offer and save format 9 are integrated. 40 combined host checks plus the earned-ion campaign check passed (41 total), including 36 ion-run cold loads. Two earned-recruitment mixed-species campaigns passed. ROM build and affected framebuffers verified; hardware/emulator acceptance remains pending. See V17_CHECKPOINT.md for evidence and limitations.

Next: beams and DS line targeting, bombs, broader equipment management, then remaining campaign/ship coverage. The full goal is still governed by FULL_GAME_CHECKLIST.md. Historical entries below retain their original scope.

# Current direction: full base-game DS goal

## V15 doors and sensors â€” software checks passed

Original Kestrel door control and sensors now have damageable, repairable levels and source-derived upgrade prices. Blast doors delay hostile movement; sensor levels gate visible room occupants/hazards and enemy equipment information. Save format 7 preserves subsystem and door-force state, with migration from formats 2â€“6. 33 host checks, complete campaigns, inspected framebuffers and the ROM checkpoint are recorded in V15_CHECKPOINT.md; the full goal remains incomplete.

Next foundation: species and eight-crew capacity. Broader weapons/systems and larger-sector/fleet work remain active, with full-game acceptance governed by FULL_GAME_CHECKLIST.md.

## V14 damage and breaches â€” software checks passed

Projectiles now carry separate hull, system and crew damage plus imported raw fire/breach properties. Breaches vent rooms and can be sealed by friendly crew. Heavy Laser I now uses its original nine-second cooldown and one-power requirement. Save format 6 persists breaches and migrates formats 2â€“5. See [V14_CHECKPOINT.md](V14_CHECKPOINT.md) for 30 host checks, complete campaigns, inspected previews and the verified ROM checkpoint.

The full base-game goal remains incomplete. Next foundations are functional doors/sensors, then species and eight-crew support; larger sectors/fleet work can proceed independently. Six-sector campaigns and six-human rosters remain the current subset. Historical checkpoint evidence below is retained separately.

## V13 boarding integration â€” software checks passed

Current implementation adds store teleporter installation/upgrades, party selection, shared faction melee/sabotage, deployment/recall and hostile boarders. Hull-only aftermath presentation delays rewards once; away survivors must be recalled or explicitly abandoned. Save format 5 migrates formats 2â€“4. See [V13_CHECKPOINT.md](V13_CHECKPOINT.md).

This remains a six-sector, twelve-beacon campaign with six humans and two crew per teleport. Escape/surrender, species, resistant doors, sensor gating, broader equipment, eight-sector generation, hangar and original finale coverage remain open. The full base-game goal remains active. Historical V10â€“V12 evidence applies to those checkpoints; V13 software evidence includes 27 host checks, a ROM build and affected framebuffer inspection; emulator and physical acceptance remain pending.

The accepted goal is the complete base-game demake, with Advanced Edition excluded. [FULL_GAME_CHECKLIST.md](FULL_GAME_CHECKLIST.md) is the active dependency-ordered plan and supersedes older statements below that defer base-game features indefinitely. Historical V10 evidence covers only that six-sector subset.

V12 adds persistent named crew/stations, details and rename controls, shared ship topology/atmosphere, a living original Rebel Fighter crew and explicit crew/hull outcome reasons. See [V12_CHECKPOINT.md](V12_CHECKPOINT.md). V13 extends those foundations with boarding and outcome presentation. Next: separate damage properties and breaches/sealing, then doors/sensors and species/eight-crew support. The full checklist governs subsequent equipment/campaign/hangar work.

---

# Complete campaign implementation checklist

## Current result

V10 is a playable six-sector Kestrel campaign candidate. The selected software subset is implemented and host checked. It is ready for human/emulator playtesting; original DS release acceptance remains pending.

## Phase 1 â€” Baseline

- [x] Read source, README, roadmap and hardware guide; no applicable AGENTS.md found.

- [x] Preserve source/assets/ROM in checkpoints/v9-baseline.zip with SHA-256.

- [x] Reproduce V9 DS build.

- [x] Inspect available emulator and storage/input configuration.

- [ ] Observe emulator screens/input/save baseline. Process startup succeeded but no targetable window was obtained.

- [ ] Physical DS timing/input/storage baseline.

## Phase 2 â€” Combat foundation

- [x] Runtime counts with maximum six crew and four installed weapons.

- [x] Piloting, station bonuses and bounded basic skill growth.

- [x] Explicit activation, per-weapon target locks and autofire.

- [x] Player-only repair/heal/oxygen/crew movement between battles.

- [x] Preserve player state across encounters; reset enemy effects and charges.

- [x] New explicit save format, version/layout validation and valid-backup recovery.

## Phase 3 â€” Progression

- [x] Stock-limited stores, purchasing, sale of cargo and human recruitment.

- [x] Bounded cargo and equipment swaps/additions.

- [x] Reactor/system upgrades with limits and explicit allocation.

- [x] Crew, upgrade, cargo and help screens with paging.

- [x] Four supported weapons and four enemy kinds with scaling.

## Phase 4 â€” Sectors and events

- [x] Six seeded connected sectors, twelve beacons each.

- [x] Bidirectional routes, persistent visited flags and no repeat loot.

- [x] Pursuit and emergency fuel rescue.

- [x] Finite typed conditional event choices and one-time outcomes.

- [x] Courier quest and solar-flare event consequences.

- [x] Restrict campaign to supported authored events; no unsupported XML selected.

## Phase 5 â€” Campaign completion

- [x] Simplified three-stage Rebel Command finale.

- [x] Victory/defeat/new seeded run flows.

- [x] Defined economy/scaling and completable test routes.

- [x] Three actual complete campaign simulations across different seeds.

- [x] Three complete resumed runs with 108 disk save/cold-load checkpoints.

- [ ] Human balancing across alternate play styles/routes.

## Phase 6 â€” Release preparation

- [x] Original bounded synthesized music/effects, sound toggle.

- [x] DS build; application compilation clean apart from SDK linker warning.

- [x] Twelve strict host checks in disposable folders.

- [x] Exact framebuffer previews and visual inspection.

- [x] Static ARM9 accounting and documented timer/audio ownership.

- [x] Controls, limitations, reproducible checks, preview tool and local checkpoint.

- [ ] Observe emulator input, audio and writable DLDI saves.

- [ ] Physical DS frame pacing, stack/heap high-water and stylus comfort.

- [ ] Long physical sessions, SD interruption and flashcart reliability.

## Scope and evidence

Kestrel only; humans with basic station skills; Artemis, Burst II, Basic/Heavy Laser; scout/interceptor/armored variants; authored six-sector events; simplified command finale. Added timings, prices, maps, enemy geometry and extra laser values are demake choices. Imported local art/blueprint values remain separately identified.

Full PC event XML interpretation, fires/breaches, ion/beams, boarding, drones, cloaking, augments, nonhuman species, other playable ships and Advanced Edition are deferred beyond this first campaign subset. No full-fidelity claim is made.

Tests prove the recorded deterministic routes and invariants. They do not prove balance for all seeds or physical hardware reliability. Final acceptance steps and exact next work are in ROADMAP.md and EMULATOR.md.

## Next expansion backlog â€” user priorities, October 3, 2026

These items are planned and do not change the V10 completion evidence above. Detailed dependencies and acceptance criteria are in ROADMAP.md.

- [ ] Persistent player/enemy crew names and rename/details interface.

- [ ] Animated crew: walking, idle, repair, combat, healing and death.

- [ ] Ship destruction animations for player defeat and enemy hull destruction; one-time result/reward transitions.

- [ ] Larger sectors, DS map navigation, varied sector identities and balanced pursuit/resources.

- [ ] Expanded rebel fleet pursuit: visible advancing front, threatened/captured beacons, interception battles, exit pressure and saved fleet state. User clarified that this was the intended "storm" request.

- [ ] Enemy crews and AI, teleporter, boarding both ways, melee/sabotage, recall and crew-only victory.

- [ ] More enemy ships and additional playable ships with a hangar and correct per-layout controls/saves.

- [ ] Supporting fires/breaches, functional doors/sensors, crew species and return-to-station orders.

- [ ] Further weapons, drones, cloaking, augments, escape/surrender and richer events.

- [ ] Difficulty, unlocks, run history, campaign/finale fidelity and DS suspend behavior.

Suggested implementation sequence: named/animated crew and destruction feedback; larger sectors/visible fleet pursuit; boarding with enemy crew AI; new playable ships based on supported systems. Physical DS acceptance continues alongside software work when hardware is available.

