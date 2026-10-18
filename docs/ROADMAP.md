# V70 damage audit and hit flash

Completed: crew damage 15 HP per point (was 5), damage audit test, red flash plus damage tag on the hit room on both screens. Release folder updated. See [V70_CHECKPOINT.md](V70_CHECKPOINT.md).

# V69 per-weapon targets

Completed: numbered target marker per weapon, B clears a weapon's target (holds fire). Release folder updated. See [V69_CHECKPOINT.md](V69_CHECKPOINT.md).

# V68 shot paths and alarms

Completed: shots from original weapon mounts, exact hit rooms, misses and shield stops; original hit and damage sounds; fire, breach, O2, hull and intruder alarms. Simple release in release/. See [V68_CHECKPOINT.md](V68_CHECKPOINT.md).

# V67 construction artillery

Completed: BOSS_SPECIAL's ARTILLERY_BOSS_1/2 mounts fire (crewless). Final package updated. See [V67_CHECKPOINT.md](V67_CHECKPOINT.md).

# V66 Federation unlock

Completed: FLAGSHIP_CONSTRUCTION special and BOSS_SPECIAL hull; all cruisers unlockable. Final package updated. See [V66_CHECKPOINT.md](V66_CHECKPOINT.md).

# V65 Crystal secret sector

Completed: secretSector, Crystal homeworld sector, Ancestry, Crystal unlock quest; 160 enabled events. Next: full check, final melonDS pass, final package. See [V65_CHECKPOINT.md](V65_CHECKPOINT.md).

# V64 tutorial

Completed: main-menu tutorial (26 steps, original text, skippable, save untouched), verified in melonDS. Next: Crystal secret sector (V65), full check, final package. See [V64_CHECKPOINT.md](V64_CHECKPOINT.md).

# V63 replies and DS save fix

Completed: unboxed numbered replies, NitroFS working-directory save fix verified in melonDS, quick check mode. Next: tutorial (V64), Crystal secret sector (V65), final full check and package. See [V63_CHECKPOINT.md](V63_CHECKPOINT.md).

# V62 event features and ship-unlock quests

Completed: store, removeCrew, boarders, status limits, named crew, taller choice boxes, homeworld specials (5 unlock chains), save32, weapon-catalog importer fix; 130 enabled events. Next: Crystal secret sector (patch in progress), tutorial (draft in progress), DS hardware readiness, final package. See [V62_CHECKPOINT.md](V62_CHECKPOINT.md).

# V61 quests

Completed: quest beacons (placement, arrival, sector carry-over/loss, cap 4, save31 + catalog hash), named quest ships (Rock Assault Elite, Zoltan Peace), crew mixes, fleet delay; 98 enabled events. Next: remaining event features (stores, crew removal, status, boarders, named crew, secret sector, long labels) to finish the ship-unlock quests; then tutorial, DS hardware readiness, final package. See [V61_CHECKPOINT.md](V61_CHECKPOINT.md).

# V60 beacon hazards

Completed: environment events (asteroid, sun, nebula, ion storm; PDS is AE), hazard beacon pools, combat hazards with banner, ion-storm reactor limit, 79 enabled events. Next: quests (incl. ship-unlock quests and the secret sector), boarders, stores and other remaining event tags; then tutorial, DS hardware readiness, final package. See [V60_CHECKPOINT.md](V60_CHECKPOINT.md).

# V59 ship achievements

Completed: 27 base-game ship achievements (26 live; Ancestry waits for the secret sector), shot origins and fight tracking, run progress in save30, profile storage with Type B unlocks, hangar progress list. Next: event batches (environment hazards, quests incl. ship-unlock quests and the secret sector, boarders, stores), then tutorial, DS hardware readiness and the final package. See [V59_CHECKPOINT.md](V59_CHECKPOINT.md).

# V58 Federation artillery and store systems

Completed: player Artillery Beam (shield-piercing beams, power row, upgrades, Federation-only save fields), store installs of missing systems at original prices, Rock B Heavy Pierce Laser; all 18 layouts startable. Next (V59): the 27 base-game ship achievements and Type B unlocks; then quest/hazard event batches, tutorial, hardware readiness, final package. See [V58_CHECKPOINT.md](V58_CHECKPOINT.md).

# V57 hangar and ship selection

Completed: menu hangar (3x3 cruisers, Type A/B, silhouettes with original unlock text), SD profile with backup/validation, Engi unlock at sector 5 and unlockShip event support, 18 imported layouts with NitroFS art packs, data-driven player ship and multi-ship saves, 2-4 crew setup, repair drones between fights. Next (V58): Federation Artillery Beam, store system purchases, Rock B laser, ship achievements for Type B unlocks; then quest/hazard event batches, tutorial, hardware readiness, final package. See [V57_CHECKPOINT.md](V57_CHECKPOINT.md).

# V56 remaining drones

Completed: all base-game player drones (Combat II, Defense II, Beam, Hull Repair, System Repair, Anti-Personnel, Boarding) with interior/boarding units, store drone offer, event drone rewards and requirements, importer order fix. Anti-Combat is Advanced Edition and excluded. Next: hangar and ship selection (V57+), then more event batches, tutorial, hardware readiness, final package. See [V56_CHECKPOINT.md](V56_CHECKPOINT.md).

# V55 cloaking and augments

Completed: player cloaking (store install, levels 1-3, power, activation, enemy charge pause, +60% evasion, firing cost, recharge lockout, damage/ion/repair), 20 base-game augments in three slots (store offer, event rewards and requirements, effects), save29. Next: remaining drones (V56), then hangar and ship selection, more event batches, tutorial, hardware readiness, final package. See [V55_CHECKPOINT.md](V55_CHECKPOINT.md).

# V54 original-style UI

Completed: original window frames/hex texture for all panels and buttons, resource icon bar, framed event replies, original power bars, title art menu. Next: cloaking, augments, drones (V55+). See [V54_CHECKPOINT.md](V54_CHECKPOINT.md).

# V53 original events (batch 1)

Completed: original sector beacon lists (per-member fail-closed compile), 57 enabled events, system/weapon/list blue options, weapon rewards, all non-augment autoRewards, system damage/fire/breach, six-choice screen, save28. Next: UI redesign (V54), then cloaking/augments/drones, hangar, more event batches (hazards, quests), tutorial, hardware readiness, final package. See [V53_CHECKPOINT.md](V53_CHECKPOINT.md).

# V52 original enemy ships

Completed: 36 imported base-game hulls (32 new incl. pirates), generic ENEMY_SHIP setup with species crews, cloaking/teleporter/drone fitting and original weapon lists, sector-people pools. Next: original events (V53). Hangar plan updated: menu hangar, original unlock rules, separate profile file. See [V52_CHECKPOINT.md](V52_CHECKPOINT.md).

# V51 base-game weapon list

Completed: 37-entry original weapon catalog (36 player weapons), hull-bust, lockdown/heal/ion bombs, shield-blocked zero-damage beams, enemy beams and bombs, sector-scaled enemy loadouts, store weapon stock. Next: enemy ship types (V52). See [V51_CHECKPOINT.md](V51_CHECKPOINT.md).

# V50 original-style star map

Completed: beacon map, sector overview and next-sector chooser redrawn with imported original map art (zone backgrounds, beacon/ship/boss/hazard/distress icons, rebel zone, labelled beacons, sector map). Layout, controls and saves unchanged. See [V50_CHECKPOINT.md](V50_CHECKPOINT.md).

# V49 silent flagship gunners

Completed: dropped the V48 transfer; a dead flagship gunner silences its sealed artillery room for good (V47 behaviour). TUNED balance 10/9/10. See [V49_CHECKPOINT.md](V49_CHECKPOINT.md).

# V48 replacement flagship gunners

Completed: a surviving flagship crewmember takes over a dead gunner's artillery room (walks if reachable, otherwise a visible 10 s transfer into the sealed room); station-manning audit for both ships. TUNED balance unchanged (10/9/10). See [V48_CHECKPOINT.md](V48_CHECKPOINT.md).

# V47 public enemy damage

Completed: enemy damage icon colours and target-card health at any sensor level (power/ion stay sensor-gated); narrow aim screens drop small room numbers. Flagship gunner question still open. See [V47_CHECKPOINT.md](V47_CHECKPOINT.md).

# V46 combat readability

Completed: original room system icons tinted by state on both ships; door-following crew walk animation with waiting at blocked doors; bottom-screen shot markers, HIT/MISS/SHIELD landing tags and damage tints; per-tile fire and breach markers plus in-room fire growth; manned sensors bonus; house rule that volleys need a manned weapons room (UNMANNED indicator, enemy gunners staffed before shields). No save change. Next: physical DS acceptance. See [V46_CHECKPOINT.md](V46_CHECKPOINT.md).

# V45 finale difficulty choice

Completed: main-menu FINALE option (TUNED default / ORIGINAL normal) applied at New Run, stored per run in save27 with save26 and older migrating to TUNED. Host memory/stack/cost measurements are in HARDWARE.md and the physical test pass is DS_TEST_CHECKLIST.md. Next: physical DS acceptance. See [V45_CHECKPOINT.md](V45_CHECKPOINT.md).

# V44 flagship final stage

Completed: stage-3 super shield (absorbs all hits, blocks teleporting), 7-laser surge with 4th-surge shield restore, stage 1-2 artillery level cap, DS-tuned flagship default (authentic numbers retained behind a flag). Next: host pacing/audio/memory measurements and the physical-DS test checklist (V45). Difficulty decided in V45 (menu choice). See [V44_CHECKPOINT.md](V44_CHECKPOINT.md).

# V43 flagship drones

Completed: stage-2 drone system (defense/combat/beam/boss boarder, parts, power order, slot-tied boarders) and drone power surge. Next: stage-3 super shield and laser surge (V44). See [V43_CHECKPOINT.md](V43_CHECKPOINT.md).

# V42 authentic flagship

Completed: imported BOSS_1-3 stages with artillery, cloaking, original crew seeding/persistence, AI takeover, stage-3 invasion teleporter and save26. Next: stage-2 drones and drone surge (V43), stage-3 super shield and barrage (V44). Difficulty decision pending playtest. See [V42_CHECKPOINT.md](V42_CHECKPOINT.md).

# V41 Federation base defense

Completed: original final-sector flow (moving flagship route, alternate-jump movement, spreading rebel control, three-jump base countdown/loss, post-phase jumps and reversal, repair stations, base supplies) with save25. Next: imported flagship hull and phase 1 artillery/cloaking (V42), then drone/boarding phases and power surges. See [V41_CHECKPOINT.md](V41_CHECKPOINT.md).

# V40 calm-ship crew AI

Completed: bCalmShip detection with 99%/25% healing thresholds and original manning-task priorities with reassignment. Next: authentic Federation base defense and three-phase flagship (V41+). See [V40_CHECKPOINT.md](V40_CHECKPOINT.md).

# V39 boarding AI

Completed: original sabotage party size and stay-behind, two-send limit, random landing room, fighter-first selection, health/forced/stay-behind evacuation and grouped intruder targeting. Next: calm-ship behavior (V40), authentic finale. See [V39_CHECKPOINT.md](V39_CHECKPOINT.md).

# V38 enemy weapon AI

Completed: original normal-difficulty shot targeting, distinct volley rooms, independent weapon timing and two-target beams, verified from the 1.6.14 executable. Next: boarding landing/recall/specialization (V39), calm-ship behavior, authentic finale. See [V38_CHECKPOINT.md](V38_CHECKPOINT.md).

# V37 combat AI

Completed: species-relative healing, safer medical routing, prioritized repair/defense, pilot preservation when other workers are available, reactor budgeting and bounded boarding parties. Next: original weapon target/volley behavior, boarding landing/recall validation and DS acceptance. See [V37_CHECKPOINT.md](V37_CHECKPOINT.md) and [V37_COMBAT_AI.md](V37_COMBAT_AI.md).

# V36 original audio

Five imported tracks and twelve effects now play through bounded streaming/mixing with synth fallback. Build, import reproducibility, embedded ROM payloads and host checks pass. Next: physical DS/flashcart playback, speaker volume, sustained combat underruns and runtime memory measurements. Crossfades remain unfinished. See [V36_CHECKPOINT.md](V36_CHECKPOINT.md).

## V35 crew setup and combat timing

Completed: pre-launch naming with cancellation, touch/button deselection, elapsed-VBlank pacing and optimized bitmap drawing. 119 host checks / 45 Python tests pass; ROM and recovery saved. Physical DS speed/input acceptance remains pending. See [V35_CHECKPOINT.md](V35_CHECKPOINT.md).

## V34 combat visibility

Completed: slim enemy weapons line above the hulls and clear destruction presentation, including when targeting was open at the final hit. Four focused host checks and ROM build pass; actual framebuffer previews inspected. Physical DS acceptance remains pending. See [V34_CHECKPOINT.md](V34_CHECKPOINT.md).

## V33 launcher icon update

Completed: Kestrel/FTL game-selection icon, rebuilt ROM, embedded pixels/title/CRC checks and unchanged executable verification. Flashcart display remains pending. See [V33_LAUNCHER_ICON.md](V33_LAUNCHER_ICON.md).

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

# FTL DS roadmap

## Target and acceptance

A complete, replayable base-game FTL demake for original Nintendo DS. Advanced Edition is excluded. The full requirements and acceptance gates are in [FULL_GAME_CHECKLIST.md](FULL_GAME_CHECKLIST.md). In-scope content remains required even while its unsupported entries are disabled during development.

V28 integrates player retreat, carried boarders, explicit abandonment, home defense during arrival events and save22 boss retry state. See [V28_CHECKPOINT.md](V28_CHECKPOINT.md). Runtime and physical DS acceptance remain open.

## Current implementation and remaining work

| Area | Implemented | Required next |

|---|---|---|

| Campaign | Eight sectors, larger connected maps, fleet, capture/patrols, wait/rescue, sector identities and branches | Broader original event pools, quests, additional sector types and active hazards |

| Combat | Shields, lasers/missiles, selected ion/beam/bomb, fire/breaches, doors/sensors, enemy escape, player retreat | Full catalog interactions; broader surrender/escape coverage, cloaking, augments, artillery and special shields |

| Crew | Names/stations, eight slots, species foundations, both-way boarding | Full animation sets, broader acquisition/quests, remaining skill/boarding/outcome coverage |

| Drones | Player Combat I and Defense I acquisition, control, parts and inventory | Other roles, movement/vulnerability and unified lifecycle |

| Equipment | Stores, upgrades, fit/remove/reorder and exact sales | Shared inventory limits, remaining loot families, additional systems and augments |

| Ships | Kestrel; original scout, assault, Rebel Fighter and Rigger enemies plus authored variants | General player layouts, all base-game ships/variants, broader original enemies |

| Finale | Authored three-stage Rebel Command | Federation base defense, moving original flagship and phase mechanics |

| Progression | Persistent run resources/crew/equipment | Hangar, base-game difficulty, unlocks, achievements and history |

| Interface | Action feedback, combat readouts, destruction effects, touch and partial button navigation | Complete controller navigation, animations, tutorial, audio coverage and lid handling |

| Saves | Map-only checkpoints, explicit version migration, backup validation | Extend with each feature; clear unsaved progress and complete failure recovery validation |

| DS validation | ROM builds, host framebuffer inspection and static accounting | Observed emulator input/audio/DLDI, runtime profiling, human playtests and physical DS acceptance |

## Execution order

1. V24 software integration complete: sector identities, supported encounter mixes and branching choices; both branches and save migration validated. Hardware gates remain open.

2. V27 enemy escape and AUTO_WARNING consequences verified. V28 player retreat integrated. Next implement surrender and its original event callbacks; broaden conditions, quests and active hazards.

3. Complete remaining combat/equipment dependencies and broaden original enemy/content coverage. Generalize hulls and enable each playable ship once its defining mechanics work.

4. Complete final-sector base defense and the three-phase flagship.

5. Complete hangar progression, difficulty, unlocks, records, tutorial, animations/audio and lid handling.

6. Audit every objective requirement; run integrated campaigns, usability/balance testing and DS release gates.

Reorder dependency work when evidence requires it. Keep each checkpoint playable. Verify the ROM and recoverable archive after meaningful integration; use real gameplay for campaign acceptance and isolated fixtures only for focused rules/migration tests.

## Evidence rules

Imported data, verified behavior and authored approximations must be distinguished. Local content stays local; installed FTL and user saves are preserved. Unsupported content stays out of random pools/stores/rewards/hangar. Test code, a green build and static memory size do not prove human usability or physical DS performance.

Hardware and emulator input/audio/storage acceptance remain pending until actually observed. Continue independent implementation when hardware is unavailable; keep the full goal active.

