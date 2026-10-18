# V15 checkpoint: doors and sensors

October 3, 2026. The full base-game goal remains active. This software integration is verified as described below; full-game and hardware acceptance remain incomplete.

## Implemented subset

- Original Kestrel door control in room 2 and sensors in room 3, initially level one, capped at three.
- Source-derived upgrade schedules: doors 35/50 scrap, sensors 25/40; no reactor allocation.
- Subsystem projectile/fire/sabotage damage and crew repairs.
- Closed blast-door resistance to intruders, shared forcing progress, idle reset and temporary forced-open jams; disabled remote control when doors are destroyed.
- Sensor visibility tiers and friendly local sight, without leaking hidden room occupants/hazards; geometry remains targetable.
- Save format 7 for subsystem and door-force state; migration from formats 2–6 preserves earlier saves through the existing backup policy.

## Source provenance and choices

Original local system/ship blueprint fields verify rooms, starting levels, maximum levels and cost lists. Localized descriptions support doors impeding intruders/fire spread and sensors revealing interiors/enemy information. They do not establish the exact resistance durations, information tiers, repair/manning or jam behavior used here.

Current AUTO_BASIC, AUTO_ASSAULT and REBEL_SKINNY data contain neither subsystem; runtime preserves their absence. Procedural opponents are not assigned fabricated original subsystem rooms. Future original configurations require their supporting content and mechanics before entering the pool.

Current authored policies: level-two/three hostile door forcing takes two/four crew-seconds; progress resets after one idle second; a forced door jams open for five seconds. Sensor levels one/two/three reveal home rooms/enemy rooms/detailed enemy equipment respectively; friendly living crew provide local sight. Nebula interference, species abilities and manning bonuses are not included.

## Validation evidence

- 33 strict host checks passed. Log: `build/v15-checks.txt`; disposable directory `build/check-4d40ec296dde432dacfa90bac800e05b`.
- Tests cover subsystem damage/repair/sabotage, original costs and mode/cap/reactor guards, invalid side rejection, reinforced traversal, jams/pause/recovery and remote-command spam resistance.
- Fire tests compare 1,000 seeds per condition: basic closed doors spread more than reinforced doors, strongest closed doors block this spread, and opening them permits it. Authored probabilities are 3%/1%/0% for closed effective levels 0-1/2/3, and 10% open.
- Exact framebuffer comparisons verify hidden dynamic room state does not change aim/boarding/diagnostic views. Static system identities stay visible. A live simulation test confirms the level-three enemy charge bar advances; displayed power distinguishes allocation from weapon requirement.
- Save tests cover upgraded/damaged subsystems, door-force/idle/jam timers, malformed bounds and identity rejection, format 6 defaults and byte-preserved backup. Existing migration checks for formats 2-5 also pass.
- 256 generated graphs; three complete six-sector campaigns and three resumed campaigns with 108 disk cold loads. An additional earned-scrap teleporter campaign deploys/recalls survivors and finishes with 29 hull and 116 scrap. These scripts do not prove human balance or full-game coverage.
- Inspected sensor levels 1/2/3, broken sensors, local away-crew sight, subsystem upgrades, reinforced and broken door previews. Host framebuffers are not emulator screenshots.
- ROM build passes: 452608 bytes; SHA-256 `cdbae2983ac43867417263be5023f6f34e994ced0e281ef22a86d6b244043cd6`. Application compilation is clean; existing SDK GNU-stack warning remains.
- ARM9 static main-RAM union 653292 bytes; image high-water offset 653320. ARM ABI Game 11784 bytes; Campaign 12696 bytes, already included in static allocations. Runtime free memory and stack/heap high-water are unmeasured.
- Release `releases/v15/ftl-ds-v15.nds`; recoverable archive `checkpoints/v15-doors-sensors.zip` with verified `v15-doors-sensors.sha256`. V14 archive hash verified before editing. Original installed content and existing saves preserved.

Emulator input/audio/DLDI and physical DS timing, stack/heap, controls and storage durability remain pending.

## Remaining scope and next steps

1. Use the verified V15 source/ROM checkpoint as the next integration baseline.
2. Implement base-game species and eight-crew support, including names, saves, occupancy, combat/oxygen exceptions and roster UI.
3. Continue complete weapon/system/catalog coverage and richer enemy configurations.
4. Develop larger eight-sector maps and visible Rebel fleet progression independently where possible.

The current campaign remains six sectors/twelve beacons with six humans. This does not prove full doors/sensors interactions with all future species, hazards or ships, and does not complete the full-game goal. FULL_GAME_CHECKLIST.md remains the acceptance definition.
