# FTL DS first-sector prototype

A native Nintendo DS proof of concept using Kestrel data extracted from this PC's copy of FTL. This is an early reconstruction, not a decompiled/recompiled FTL executable or the full game. No PC engine code was recovered in this milestone.

## Current milestone: V9 interface and autofire

Start on an eight-beacon map. Select a linked beacon by touch or L/R, then press **A / JUMP**. Every jump costs one fuel. Combat, supply, repair and quiet beacons lead toward the exit. This graph and its rewards are prototype rules; only the imported quiet-event text is original event content. The full FTL event interpreter and sector generation remain unfinished.

Ship hull, crew injuries, systems, reactor allocation, room oxygen, door states and ammunition carry between encounters. New fights reset weapon charge and replace the enemy. Map/event screens currently freeze simulation, so repair and healing are available during encounters only. Reach the exit to complete this first-sector milestone; further sectors are not implemented.

On the map, **X / SAVE** writes `/ftl-ds.sav` to writable homebrew SD storage. A valid save is loaded on launch. Saving is deliberately restricted to the map; no mid-combat saves yet. Missing/unwritable storage is reported and the current run remains playable. Start a new run from the terminal victory/defeat screen. The previous manual save remains until you save again.

## Combat controls

- Touch a crew button, then a room to move. Stationary crew repair damaged systems; powered medbay heals them.
- Tap system minus/plus buttons to allocate the eight reactor bars.
- **Start / X**: pause/resume; orders can be issued while paused.
- **A / LASER**: fire charged Burst Laser II at the selected target.
- **B / MISS**: fire charged Artemis; consumes a missile.
- **Y / AIM**: show enemy rooms. Tap a numbered room; gold marks the target. Y/BACK returns to crew controls.
- **L / R** in combat: cycle enemy rooms.
- **Down / DOORS**: door controls; tap doors, including orange exterior airlocks.
- **Up / SEAL ALL** in door mode: close all doors.
- **Select**: diagnostic logs; simulation is held until closed.

Weapons share a selected target; Burst Laser II receives priority when weapon power drops. In Y / AIM, tap LASER AUTO or MISS AUTO to toggle repeat fire independently. Both start off each encounter and after loading. Autofire waits for charge, power and ammunition, stops while paused, and uses the selected room for the next volley. A volley already queued retains its target. Explicit weapon power selection remains planned.

## Roadmap and hardware evidence

See [docs/ROADMAP.md](docs/ROADMAP.md) for milestone criteria and the feature register. [docs/HARDWARE.md](docs/HARDWARE.md) records the static memory baseline and provisional budgets. Runtime work-time records appear in diagnostics as `work_us` after the DS build actually runs. Static memory estimates and host tests are not measurements of DS frame rate or SD reliability.

## Doors and atmosphere

The original 26 Kestrel doors are now interactive, including six exterior airlocks. All start closed. Crew automatically open an interior door while crossing it, then close it after a short delay. Manually opened doors stay open.

An open airlock exposes its room to space; open interior doors carry that exposure through the ship. Closed doors isolate rooms. Oxygen is tracked separately in each room, and the HUD shows an average weighted by room area. Rooms with low oxygen turn red. Crew take suffocation damage based on the room they occupy. Seal the airlocks and restore oxygen-system power to refill rooms. You can order door changes while paused; the exposure display updates immediately, while oxygen quantities remain frozen.

Drain, refill, diffusion and crew-door timing are provisional approximations, not recovered FTL constants. Fires, hull breaches and door-system damage/locking are not implemented yet.

## Diagnostic logs

Press **Select** to view recent records and the logging status on the DS screens. File logging appends to **`/ftl-ds.log`**, at the root of the mounted SD filesystem. In melonDS this means a configured writable homebrew/DLDI SD image; it is not automatically a Windows file beside the ROM. A physical flashcart needs working DLDI storage support.

If storage is missing, unwritable, or the log reaches **256 KiB**, the game continues with the latest **64 records in memory**. These disappear when the game closes. The diagnostic screen reports the current status. Logs are also sent to compatible emulator debug consoles through `nocashMessage` (console capture depends on emulator configuration).

Records include session/build identifier, button/touch inputs, actions, hull/shield changes, power/system damage, crew movement/health, manual doors, room exposure, oxygen threshold changes, messages, and snapshots of crew/weapon/room state. `f=` is the display-loop frame counter; snapshots also contain simulation ticks. Writes are buffered and flushed at 10-second display-frame intervals, on outcomes/restarts, and when Select opens/closes logs. The most recent buffered entries may be lost if the emulator is closed abruptly. No per-frame disk writes are made. The cap never truncates existing logs; archive/remove the log yourself before a new session to resume file logging after the cap is reached.

## What is authentic

Imported from the local archive: Kestrel's 17 rooms, 26 doors, installed system capacities, 30 hull, reactor capacity 8, crew count 3, starting missiles 8, and original hull art. Combat also uses an original auto-scout image and a human crew sprite extracted from the installed archive. The Auto-Scout now uses its original five-room layout, system placement, base hull 6 and reactor 6. Its starting blueprint has no shields, crew or oxygen. This fixed encounter selects LASER_BURST_1 from its original randomized weapon pool: one shot, one damage, one power, 10-second base charge. Sector scaling and weapon randomization are not implemented. Artemis has an 11-second base cooldown, 2 damage, 1 shot and 1 power; Burst Laser II has a 12-second base cooldown, 3 shots of 1 damage and 2 power.

`generated/manifest.json` records the initial source data extraction and hashes. Additional original combat art is retained under `generated/original/` and converted by `tools/convert_ship_art.py`. The installed FTL files are read only. `generated/` contains original local game content and is excluded from source control, as is the compiled ROM containing that content.

## Limits of this milestone

- One fixed, unscaled Auto-Scout encounter. Automated repair remains provisional at one installed system bar every 12 seconds.
- Provisional movement (one room/second), repair (5 seconds/bar), shield recharge (2 seconds), projectile timing and evasion (5% per powered engine bar).
- Crew placement is chosen for the prototype. No pilot/manning bonuses, skills, fires, breaches, boarding, drones or audio. Events, map and saving use the limited first-sector prototype described above. Pilot, sensors and door consoles currently display their original furnishings only. The medbay is functional as of v7.
- Combat displays room cutaways on both ships, original ship artwork, the Kestrel hull under the tactical rooms, eight original room furnishings, interactive doors, venting effects, crew sprites, shield outlines, projectile travel and impact flashes. The enemy room overlay is aligned using its original hull bounds. Crew sprites interpolate between connected room positions. Projectiles travel for 12 simulation ticks, then apply damage once and display an 8-tick effect; these visual timings are provisional.
- Fixed 30 Hz simulation, deterministic RNG and allocation-free game state. This is designed for original DS; real hardware performance and input remain unverified.

## Build

A portable MSYS2/Wonderful/BlocksDS installation is under `.tools/` for this workspace. Rebuild from PowerShell:

```powershell
.\build.ps1
```

To regenerate local assets:

```powershell
.\build.ps1 -FtlArchive 'C:\Program Files (x86)\Steam\steamapps\common\FTL Faster Than Light\ftl.dat'
```

The archive importer uses Python's standard library. Art conversion also needs Pillow. On a fresh machine, follow [BlocksDS Windows setup](https://blocksds.skylyrac.net/docs/setup/windows/) and run `make` in the Wonderful shell after generating the assets. The Makefile uses the SDK's ARM9 template and default ARM7 core.

## Validation performed

- DS ROM compilation completed with BlocksDS 1.24.0 and Wonderful ARM GCC. The SDK startup object emits a GNU-stack linker warning; application compilation is clean.
- One focused host smoke test in `tests/smoke.c` checks imported starting stats, pause, crew routing, repairs, power caps, missile use and an encounter ending in victory. With the original base Auto-Scout, its scripted encounter finished in 733 ticks with 30 hull. The prior focused logging check passed for append behavior, state records, snapshots, file cap and memory fallback. The v3 door check passed for airlock isolation, open-door propagation, local suffocation, resealing/refill, pause, weighted oxygen and crew crossings. The render preview also exercised touchscreen door selection; new room/door layout images were visually inspected.
- Layout inspected using `tools/render_preview.c`, which renders the same framebuffer code on the PC. Images under `build/` are framebuffer previews, not emulator screenshots.
- The user confirmed the first ROM runs in the emulator. Automated window capture failed, so no independent emulator run was verified. The art/scout revision was compiled and checked on the host without another emulator session; SD writes and revised frame pacing still need runtime confirmation.

## Next milestone

Use diagnostics from normal play to identify problems. Next verify combat timings and expand crew/system behavior, starting with piloting and crew station bonuses. Reserve emulator checks for startup, storage, rendering or input changes that host checks cannot cover.

## Research references

- [FTL Hyperspace](https://github.com/FTL-Hyperspace/FTL-Hyperspace): reverse-engineering reference, a PC binary mod rather than a portable engine.
- [Project Wormhole](https://gitlab.com/znixian/xftl): existing engine reconstruction; no code from it is included here.
- [Mike Hopley's weapon timing research](https://github.com/MikeHopley/ftl-weapon-timing/blob/master/weapons.js): reference for timing research; the two base weapon cooldowns in this build come directly from local FTL data.

## Original-art comparison (v4)

The installed archive, its ship XML, blueprints and animation definitions were used as independent references. Furniture previously scaled to 20 pixels across a 28-pixel room; it now matches the ship grid at 0.4 times original size. The original floor is positioned at its XML offset. Normal crew view no longer covers furnishings with text labels; door mode keeps system labels for navigation. Tile lines are drawn before furnishings.

The previous human image was the frame explicitly named `human_portrait`. Crew now use the first `human_walk_down` frame at the same scale as the rooms. Animation cycles and crew color overlays remain unfinished.

`generated/kestrel_original_asset_reference.png` is a full-size reconstruction from original assets for comparison, **not a screenshot of gameplay**. `generated/art_audit.json` records source coordinates. `generated/enemy_data.json` records original blueprint values, chosen encounter simplifications and extraction hashes. The final combat and venting framebuffers were inspected at native dimensions enlarged by nearest-neighbor scaling. No live original-game or new emulator session was verified in this revision.

## Visible room floors (v5)

Fixed the remaining floor rendering issue: opaque gray room rectangles hid the underlying original floor and made rooms look empty. Room cells now use light panels with subtle inset edges and visible seams, with the original furnishings drawn above them. Low oxygen progressively tints the panels red without losing the tile detail. Both combat miniatures and enemy targeting also show light room floors.

An archive audit did not locate a standalone light room-tile texture. These panels are a procedural visual approximation; the imported `kestral_floor.png` remains the original gray base silhouette. Earlier asset reconstruction images are not evidence of the original game's dynamic room shading.

The screen header now reads **FTL DS / V5**, and the log identifies `encounter-v5`, so the loaded ROM is identifiable. Reopen the rebuilt ROM in the emulator to load this revision. Validation was limited to the DS build and the host framebuffer preview (normal rooms, venting, enemy targeting), including the existing door touch assertions. No simulation changes or additional emulator session were needed.

## Enemy targeting (v6)

Press **Y / AIM** to open the enemy cutaway on the touch screen. Tap a numbered room; its outline turns gold and its name appears below the ship and on the top screen. The view stays open so you can inspect the target and fire. **L/R** cycle rooms. Room numbers shown to players start at 1.

Press **A / LASER** or **B / MISS** once the weapon is charged. The buttons are green when ready. Both weapons use the selected room; a volley already queued keeps its original target. **Y / BACK** returns to crew controls without losing the selection. **Start/X / PAUSE** lets you plan while paused; opening targeting alone does not pause combat. Empty rooms still take hull damage. The pilot room is labeled, but piloting functionality remains unfinished.

The targeting view now includes the enemy hull, larger room cells, installed system health, persistent target confirmation and touch fire buttons. Header: **FTL DS / V6**. Focused host assertions passed for selecting engines, ignoring blank taps, firing both weapons at the selected room and returning without changing target or pause. DS build passed and the targeting framebuffer was visually checked; no new emulator session was run.

## Powered medbay (v7)

The Kestrel medbay now heals injured living crew stationed in its room. Select a crew member and tap the medbay (upper room immediately above shields). Free one bar using another system's minus control, then tap **MED +**. All five powered systems share the original eight-bar reactor. The medbay starts unpowered to preserve the existing combat allocation; this initial allocation is a prototype choice.

Healing stops when the medbay loses power, is destroyed, the crew member leaves or combat pauses. Crew can repair a damaged medbay using the existing repair behavior. Healing is capped at 100 HP and cannot revive dead crew. The crew button reads **HEAL** while recovery is active, and a green cross appears beside that crew member. Existing health and system log records include recovery and medbay power changes.

Original local blueprints confirm room 4, one installed power level, and descriptions confirm healing in the powered room. Healing speed is provisionally **6 HP/second** at this level; this value and the current 100-HP crew maximum were not recovered from those blueprints. Fires, skills and the original medbay occupancy rules remain incomplete.

V7 validation: medbay checks passed for power budget, room-based healing, pause, depower, departure, system repair, maximum health and no resurrection. The existing combat smoke check passed. The framebuffer preview exercised the revised engine/medbay touch controls and was visually inspected. The DS ROM built successfully; no additional emulator session was run.

## V8 validation and save limitations

DS build and focused campaign/save checks passed. The campaign check completed three real battles en route to the exit; save checks covered roundtrip, checksums/corruption, valid routes, map-only restriction and protection of unrelated files. Framebuffer previews checked map/event touch transitions and were visually inspected. Static ARM9 main-RAM sections total464020 bytes (about453KiB); this is not total runtime usage or a frame-rate measurement.

Saves use explicit fields, version/layout validation, a temporary file and `.bak` replacement. Load currently reads the primary save only; the backup is retained for manual recovery. Power-loss durability and homebrew SD behavior still require runtime confirmation. Save format changes may invalidate old saves. Save/load staging is single-threaded and uses static memory. File logging retains its256KiB cap.

Current checkpoint and exact next steps are in docs/ROADMAP.md. Full campaign content, faithful sector generation, piloting, expanded combat systems and audio remain unfinished.

## V9 interface and autofire

The rebuilt `ftl-ds.nds` includes original nebula art and resource icons, framed status panels, crew health bars, weapon charge/readiness cards, and revised map and event screens. The map header identifies V9; diagnostics identify `sector-v9`. Reload the ROM in your emulator to run this version.

Targeting has separate laser and missile autofire buttons. Toggles apply only to the current encounter and can be changed while paused. Disabling autofire prevents the next volley; it does not cancel shots already queued. Each missile volley consumes ammunition normally. Existing map saves keep their version-1 format; autofire is not saved.

Validation: eight host checks passed (combat, doors, medbay, targeting, autofire, campaign, save and logging). The exact framebuffer preview passed its touch assertions and six screens were visually inspected. BlocksDS compiled the ROM successfully with the existing SDK GNU-stack warning. Static ARM9 main-RAM sections total 568156 bytes (about 555 KiB). These previews are host renders; no new emulator or physical DS run was performed. Runtime performance, touch comfort and SD save reliability remain unverified.
