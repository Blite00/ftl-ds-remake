# V17 checkpoint: ion weapon foundation

October 3, 2026. Software integration verified. Emulator and physical DS acceptance remain pending. The full base-game goal remains incomplete.

## Implemented

- Ion Blast I at stable catalog ID 4, available for 30 scrap as the ninth store offer. Three store pages safely handle empty rows. Existing weapon IDs remain unchanged.
- Imported original ION_1 stats: one ion damage, eight-second charge, one power, one projectile, no ammunition and no physical damage. Source audit is in WEAPON_IMPORT.md and generated/weapon_catalog.json. Installed-source AE stun data is excluded.
- Ion bolts respect evasion and strike shields first. Unshielded hits temporarily reduce the affected system's usable power without damaging hull, crew or system health. Player/enemy weapon queues preserve their ion payload.
- Powered systems, subsystems, piloting, teleporter and enemy drone consumers respect ion state. Zoltan bonus power remains usable. Sensor visibility gates enemy ion details; player countdowns, room notices and cyan effects distinguish ion locks.
- Map-only save format 9 preserves player ion state and the ninth offer's stock. Formats 2–8 migrate; existing valid files remain byte-exact backups. Historical map encoding remains eight offers with new fields appended, preserving old offsets.

## Authored policies and limits

Ion stacks one disabled normal-power bar and five seconds per ion damage, capped at eight bars and forty seconds. All accumulated bars clear when the timer expires. Normal reactor allocations remain reserved for restoration and can still be changed with existing power controls; bonus Zoltan power bypasses these disabled normal bars. Own locks persist across encounter transitions and recover during active service; enemy locks reset with a new enemy. These timing, bounds and allocation policies are authored DS behavior, not verified PC-engine fidelity. Projectile flight uses the existing twelve-tick presentation rather than source speed simulation.

Only Ion Blast I is newly playable. There is no new enemy ion configuration in the campaign pool yet. Beams, bombs, the remaining ion catalog, complete drones/cloaking/augments and full weapon content remain unfinished. Existing authored economy prices are retained for older weapons.

## Validation

40 combined strict host checks passed, plus the separately compiled and executed ion_run_smoke check (41 distinct checks total; all are now registered in tools/check.ps1). Evidence is in build/v17-checks.txt and build/v17-ion-run.txt. Focused coverage includes both-side ion volleys, shield interception, evasion, stacking/recovery, pause, Zoltan power, affected power consumers, malformed-save rejection, old-format migration, stock persistence and sensor-respecting framebuffer output.

The earned-economy Ion run completed six sectors with 30 hull and 127 scrap, purchasing and installing Ion Blast through ordinary store actions, observing actual ion effects/locks and performing 36 disk cold loads. No resources, damage or victories were injected. Existing three complete campaigns and three resumed campaigns with 108 cold loads also passed. Added mixed-species recruited runs completed seed 1 with five crew (Human/Rock) and seed 42 with six crew (Human/Mantis/Zoltan), preserving identities/species/health through disk reloads. These runs do not establish full eight-crew or Crystal campaign acceptance.

Inspected renderer previews: ion-player-v17.png, ion-enemy-v17.png, ion-bolt-v17.png and store-ion-v17.png. Enemy details remain hidden below the required sensor level in exact framebuffer tests. Physical touch comfort and audio output are unverified.

ROM build passed. ARM9 static sections: 670532 bytes; image high-water: 670560 bytes. Game: 12816 bytes; Campaign: 13776 bytes. These exclude runtime stack/heap measurements. Existing toolchain executable-stack metadata warning remains.

Release: releases/v17/ftl-ds-v17.nds, 466944 bytes, SHA-256 `11bd746c4d38668ba7d3af13bbaf28270629a3bcbea59373d4dc0980efe27500`.
Archive: checkpoints/v17-ion-weapons.zip with SHA-256 sidecar and per-file manifest. V16 baseline archive hash verified before work; earlier checkpoints remain preserved.

## Exact next steps

1. Add beams with a usable DS start/end targeting interaction, shield-layer attenuation, bounded room intersection and appropriate original beam entries.
2. Add bombs with shield bypass, evasion and distinct physical/system/crew payloads; broaden equipment controls and catalog coverage.
3. Exercise full eight-crew and additional species combinations through ordinary progression; implement supported Crystal acquisition.
4. Continue larger eight-sector maps, visible fleet progression, original events, hangar and authentic supported finale according to FULL_GAME_CHECKLIST.md.
5. Perform emulator input/audio/save exercises and physical DS frame time, stack/heap, lid and SD durability checks when available. They remain pending.
