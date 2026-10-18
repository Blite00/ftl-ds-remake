# Historical roadmap through V23

Superseded by ROADMAP.md. These entries preserve the scope and evidence recorded at each checkpoint; they are not current completion claims.

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

Weapon removal, displayed-order power priority, exact-item sales with confirmation, and safe zero-weapon loadouts are integrated. All 53 registered host checks passed, including an earned equipment campaign with 38 disk cold loads. Save format 12 migrates formats 2–11, preserving legacy weapon priority. ROM build and equipment framebuffer inspection passed. See V20_CHECKPOINT.md for evidence and limitations.

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

## V16 species and eight-crew roster — software checks passed

Working source expands both bounded rosters to eight and implements the seven base-game species, group orders, dismissal, original species frames and save format 8. Six species are available through deterministic store offers; Crystal has no supported acquisition route yet and remains outside that pool. 36 host checks, inspected renderer previews and the ROM build passed. See V16_CHECKPOINT.md for source provenance, approximation boundaries and pending runtime validation.

After validating species/power/visibility/lockdown interactions and migration, continue full weapon/system/equipment coverage, appropriate Crystal acquisition and broader original enemy/playable-ship content. Larger sectors and visible Rebel fleet progression may proceed independently. The full base-game goal and hardware acceptance remain unfinished; historical V15 evidence below does not prove V16.


## V15 doors and sensors — software checks passed

Original Kestrel door control and sensors now have damageable, repairable levels and source-derived upgrade prices. Blast doors delay hostile movement; sensor levels gate visible room occupants/hazards and enemy equipment information. Save format 7 preserves subsystem and door-force state, with migration from formats 2–6. 33 host checks, complete campaigns, inspected framebuffers and the ROM checkpoint are recorded in V15_CHECKPOINT.md; the full goal remains incomplete.

Next foundation: species and eight-crew capacity. Broader weapons/systems and larger-sector/fleet work remain active, with full-game acceptance governed by FULL_GAME_CHECKLIST.md.


## V14 damage and breaches — software checks passed

Projectiles now carry separate hull, system and crew damage plus imported raw fire/breach properties. Breaches vent rooms and can be sealed by friendly crew. Heavy Laser I now uses its original nine-second cooldown and one-power requirement. Save format 6 persists breaches and migrates formats 2–5. See [V14_CHECKPOINT.md](V14_CHECKPOINT.md) for 30 host checks, complete campaigns, inspected previews and the verified ROM checkpoint.

The full base-game goal remains incomplete. Next foundations are functional doors/sensors, then species and eight-crew support; larger sectors/fleet work can proceed independently. Six-sector campaigns and six-human rosters remain the current subset. Historical checkpoint evidence below is retained separately.


## V13 boarding integration — software checks passed

Current implementation adds store teleporter installation/upgrades, party selection, shared faction melee/sabotage, deployment/recall and hostile boarders. Hull-only aftermath presentation delays rewards once; away survivors must be recalled or explicitly abandoned. Save format 5 migrates formats 2–4. See [V13_CHECKPOINT.md](V13_CHECKPOINT.md).

This remains a six-sector, twelve-beacon campaign with six humans and two crew per teleport. Escape/surrender, species, resistant doors, sensor gating, broader equipment, eight-sector generation, hangar and original finale coverage remain open. The full base-game goal remains active. Historical V10–V12 evidence applies to those checkpoints; V13 software evidence includes 27 host checks, a ROM build and affected framebuffer inspection; emulator and physical acceptance remain pending.

The accepted goal is the complete base-game demake, with Advanced Edition excluded. [FULL_GAME_CHECKLIST.md](FULL_GAME_CHECKLIST.md) is the active dependency-ordered plan and supersedes older statements below that defer base-game features indefinitely. Historical V10 evidence covers only that six-sector subset.

V12 adds persistent named crew/stations, details and rename controls, shared ship topology/atmosphere, a living original Rebel Fighter crew and explicit crew/hull outcome reasons. See [V12_CHECKPOINT.md](V12_CHECKPOINT.md). V13 extends those foundations with boarding and outcome presentation. Next: separate damage properties and breaches/sealing, then doors/sensors and species/eight-crew support. The full checklist governs subsequent equipment/campaign/hangar work.

---

# FTL DS roadmap and checkpoint

## Current milestone: V10 complete campaign subset

The first release target is a replayable Kestrel demake campaign on original DS. The engine implements a six-sector run with stores, upgrades, authored event choices, scaling enemies and a simplified three-stage finale. Host simulations can complete it. Runtime acceptance on DS is still pending.

The project is a native reconstruction. Imported assets and blueprint values do not establish fidelity of engine behavior. Generated sectors, economy, new enemies, skill timing and finale mechanics are authored approximations.

| Feature | Current status | Remaining work |

|---|---|---|

| Kestrel/scout imported geometry and art | Data verified | No blanket gameplay fidelity claim |

| Combat/oxygen/repair/healing | Implemented approximation; host checked | Calibrate and playtest |

| Piloting/stations/skills | Implemented approximation | Human balance assessment |

| Variable crew/equipment | Bounded six humans, four weapons | Other species/content deferred |

| Activation/targets/autofire | Implemented; host/UI checked | Stylus verification |

| Management/economy/cargo/upgrades | Implemented; host/UI checked | Playtest usability and pricing |

| Six generated sectors/pursuit | Implemented; 256 maps checked | Wider playtesting |

| Conditional events/quest | Finite authored event dispatcher | Original XML interpreter deferred |

| Finale | Three-stage demake boss; full runs checked | Original flagship mechanics deferred |

| Save/resume/backup recovery | Explicit V10 format; cold-load runs checked | DLDI and power-loss testing |

| Music/effects | Original synthesized implementation | Listening on emulator/DS |

| Real hardware | Pending | Timing, stack/heap, input, audio, SD reliability |

## Completed integration evidence

- Pre-change recoverable V9 checkpoint plus SHA-256.

- DS build and twelve focused host checks.

- Three complete campaign runs using public combat/economy/service APIs.

- Three additional complete runs with 108 disk save/cold-load checkpoints.

- 256 seeded map connectivity/symmetry tests.

- Visual inspection of combat, targeting, procedural boss, six-crew page, map, events, stores, management and end screens.

- Audio scheduling checks and static memory accounting; neither proves physical output/performance.

See IMPLEMENTATION.md, CONTENT.md, COMBAT.md, AUDIO.md and EMULATOR.md for details.

## Next work in order

Hardware acceptance remains open and should proceed alongside independent feature development. The next expansion priorities below reflect the user's requested additions on October 3, 2026. These are planned features, not implemented in V10.

### Priority 1: named, animated crew and battle aftermath

- Persistent names for player and enemy crew; generated defaults and optional rename UI. Keep names bounded, legible and preserved in saves.

- Directional walking, idle, repairing, fighting, healing and death animations. Use original local animation definitions where applicable and bounded sprite storage.

- Distinct enemy/player ship destruction sequences: initial blast, secondary explosions, debris, fade and transition to salvage or defeat.

- Separate combat result from presentation: freeze combat at a terminal result, grant rewards once, prevent repeated input, and never show ship destruction for surrender, retreat or crew-only victory.

- Crew detail page: name, health, station, skill progress and current order; save/return-to-stations controls.

Completion: crew can be recognized by name and behavior, and both battle outcomes play an appropriate animation before advancing. Pause, reload and repeated taps cannot duplicate rewards.

### Priority 2: larger sectors and the pursuing rebel fleet

- Expand beyond the current twelve-beacon/two-lane sector structure. Prototype roughly 20-24 beacons, with the final limit determined by DS readability and performance.

- Add map pan/overview controls, readable connections, sector identities, branch choices between sectors, clearer pursuit boundaries and hazard markers.

- Rebalance fuel, event density, stores and pursuit against the larger graph. Validate connectivity, viable routes and persistence across many seeds.

- Expand V10's existing simplified pursuit counter/patrol encounters into a clearly visible rebel fleet front that advances with travel and applicable events.

- Mark threatened and captured beacons, show the effect of the next jump, and communicate remaining escape routes to the exit.

- Define captured-beacon behavior: fleet interception battles, disabled ordinary services/events, limited rewards and exit-beacon consequences. Avoid repeat-reward exploits.

- Integrate event-driven pursuit delays/advances and sector-specific pursuit behavior as the supported event roster expands.

- Preserve fleet position and captured-beacon state in saves; verify movement on revisits, rescue, sector transitions and near-exit situations.

- The user clarified that "storm" meant the chasing fleet. Environmental storms are optional future content, not this requested feature.

Completion: the player can see and plan around the approaching fleet on a larger DS map, with consistent capture, interception and escape rules.

### Priority 3: boarding and living enemy crews

- Add enemy crew rosters, movement, repair, healing, station operation and combat AI; current opponents are automated.

- Add teleporter installation/power/cooldown, party selection, deployment and recall.

- Support player boarding enemy ships and hostile boarders entering the Kestrel.

- Implement melee combat, sabotage, room occupancy, faction-aware targeting and clear crew selection across ships.

- Add door health/resistance, sensor visibility and species abilities incrementally.

- Distinguish hull destruction, enemy crew elimination, surrender and retreat rewards.

- Handle crew stranded on a departing/destroyed ship and prevent returning dead crew or duplicating crew across ships. Expand the save schema when required.

Completion: both sides can board, fight, repair and retreat; crew-only victories and stranded-crew cases resolve consistently.

### Priority 4: more ships and distinct play styles

- Expand enemy silhouettes, imported layouts where available, equipment, crew compositions and tactics.

- Add a hangar and further playable ships with different room layouts and starting equipment. Choose ships whose defining systems are implemented first.

- Generalize the current Kestrel-specific rendering, furnishings, cockpit placement and save-layout identity before enabling another hull.

- Add ship variants, unlock quests and run records after the underlying ships work.

- Improve the final boss's appearance and phase-specific behavior as supported mechanics grow.

Completion: a new ship changes tactical decisions as well as appearance, and its controls, routing, saves and equipment work throughout a full campaign.

## Additional missing aspects worth adding

These are accepted full-game requirements scheduled after the foundations, not features of the historical V10 subset:

- Fires, fire spread, firefighting, hull breaches, sealing breaches and local crew damage.

- Optional environmental hazards: nebula visibility, ion storms, timed solar flares and asteroids. V10 only has a solar-flare event choice; active hazards would need their own simulation and pause/reset rules.

- Beam targeting, ion effects, bombs and a wider weapon catalog; distinct projectile/impact effects.

- Drone control, defense/offense/repair drones, drone parts, cloaking and ship augments.

- More crew species and abilities, combat/repair skill progression, portraits and better crew selection.

- FTL charge and escaping combat; enemy escape timers and surrender decisions.

- Functional sensors and doors, information visibility, subsystem upgrades and room occupancy rules.

- Richer event chains and equipment/species-dependent options; later, a supported original-event interpreter with an explicit coverage register.

- Eight-sector campaign option, sector-type selection and a closer recreation of the original flagship once the required systems exist.

- Difficulty settings, hangar loadouts, achievements/unlocks, scores and run history.

- Saveable crew stations, return-all-to-stations, tooltips, clearer alerts, controller-only menu navigation and DS lid/suspend handling.

- Broader original audiovisual coverage within a bounded asset-loading/audio budget.

Advanced Edition additions (such as hacking, mind control and clone bays) remain a separate later scope.

## Acceptance work still open

- Visible emulator and physical DS verification of input, sound, timing and writable DLDI storage.

- Human playtesting of complete runs, including alternate routes and strategies.

- Stack/heap high-water measurements, long sessions and interrupted SD-save testing.

## Integration map

- game.c/game.h: bounded simulation, catalog, crew, service and enemy setup.

- campaign.c/campaign.h: generated maps, event choices, economy, pursuit and finale.

- render.c/campaign_view.inc: DS screens and touch dispatch.

- main.c: hardware controls, seeded startup, save path, simulation gating and audio.

- save.c: pointer-free format2, static staging, validation and backup handling.

- audio.c: bounded PSG patterns and effects, no ARM9 profiler timer conflict.

- diagnostics.c/performance.c: bounded records and update-work timing.

Source/art/ROM baseline history is retained in checkpoints/v9-baseline.zip; older prose is in HISTORY.md. User V9 saves are preserved by the new campaign filename. No live PC files are modified.
