# V31 enemy weapons and threat feedback

## Delivered

- Two independently charging enemy weapon slots with power priority, queued volleys and finite missile ammunition. Existing single-weapon encounters retain their legacy path.
- Enemy Artemis is imported from MISSILES_2 (10 seconds, two power), separately from the player's Artemis (11 seconds, one power). The original installation is read only. Generated source provenance remains local.
- In policy4 generated campaigns, supported encounters from sector 2 use Basic + Heavy, Basic + enemy Artemis, or Basic + existing drone support. The first-sector V30 mix remains. Original source lists permit the components; exact pairs, selection and system/reactor allocation are authored DS choices.
- Weapons damage and ionization can disable the second slot. Already queued shots retain their payload. Retreat, departure and terminal results clean up transient offense.
- The normal combat screen identifies enemy weapon families. Sensors level 3 exposes individual names, charges, FIRING/OFF/EMPTY status. Low sensors do not reveal charge, ammunition or power state. The aiming screen retains the existing room/system targeting card.
- Safe map saves stay format23. Cold load, active-save refusal, retreat and original event engagement have focused regression coverage. Event policies0..3 retain legacy loadouts. Existing policy4 saves remain valid, but future supported battles receive the new equipment; pair selection consumes no additional RNG draw.

## Evidence

The ROM builds with the existing SDK executable-stack metadata warning. Four affected host framebuffer scenes were generated and visually inspected: family identification, independent charges, empty ammunition and disabled secondary weapon. Renderer checks cover sensor privacy, framebuffer guards and unchanged Game state.

Core loadout, save integration and earned-run tests also pass undefined-behavior trap builds. The save checks cover 32 cold-load/profile continuations and byte-preserving refusal of active combat saves. All 107 host checks, 37 event compiler tests and 3 enemy importer tests pass. Evidence is recorded in build/v31-evidence.json and build/v31-check.log.

Three ordinary-action eight-sector campaigns completed with 195 jumps and nine new-profile battles, including two missile pairs. They consumed 88 player missiles and one enemy missile; final hull30 includes purchased repairs. No artificial player resources, victories or damage were used in these pilots. Controlled 32-seed fixtures separately show that Weapons targeting suppresses these threats. See ENEMY_LOADOUTS.md for methods and results. Neither dataset establishes original difficulty parity.

ARM9 static allocated union: 851,908 bytes; image high-water: 851,952. Game: 17,876 bytes; Campaign: 22,016. A static encounter buffer keeps staging off the DS stack. Runtime stack/heap and frame-time measurements remain pending.

## Release and recovery

Current ROM: G:/demake/ftl-ds.nds. Preserved copy: releases/v31/ftl-ds.nds. Size: 570,368 bytes. SHA-256: `6e7e4c7c339f47d9c1fc1aadbcd8679c421ef606b2a40c514fa124bf66864475`.

Recovery: checkpoints/v31-enemy-loadouts.zip with SHA-256 sidecar and build/v31-manifest.json. V30 remains preserved in releases/v30 and checkpoints/v30-ds-controls.zip. Installed FTL files and existing user saves were not changed.

## Exact next steps

1. Complete generic REBEL surrender dependencies: RANDOM stuff, HIGH fuel and LOW scrap_only rewards; chance-based escape defaults and priority; one-time surrender/gotaway/dead-crew callback ownership. Keep all source alternatives and weights. Only then enable complete original event branches and offers.
2. Expand earned-run balance coverage across route types, missile profiles, upgrades and retreat policies. Investigate whether effective Weapons suppression makes later battles too safe before adjusting tuning. Add more supported source equipment and enemy profiles as their full behaviors become available.
3. Finish remaining controller-only management/confirmation paths. Check target, damage and incoming-threat feedback with human playtesting at native screen size.
4. Perform emulator input/audio/storage and physical DS checks: V30 HIGH volume/clipping, controls, readability, frame pacing, suspend and flashcart save/resume. These are pending; host previews and ROM builds do not establish hardware acceptance.

The broader base-game checklist remains incomplete, including surrender, wider content/equipment, authentic flagship/base defense and progression. This milestone starts the requested combat work; it does not claim completion of the full game.
