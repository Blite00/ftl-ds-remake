# V46 combat readability: room icons, door paths, shot markers and manned weapons

Blite played V45 and asked for five additions. All five are in this build.

## Completed

### 1. Room system icons
- Each system room on both ships shows the original FTL room icon (`img/icons/s_*_overlay.png`, or `s_artillery.png` for flagship artillery). There are 12 icons: shields, engines, weapons, oxygen, medbay, pilot, doors, sensors, drones, teleporter, artillery and cloaking.
- `tools/import_system_icons.py` converts them to 4-bit coverage masks at 12×12 (bottom-screen ships) and 7×7 (top-screen miniatures and small rooms). Rooms too small for 7×7 get a 3×3 state dot. The output is `generated/system_icons.h`, about 2.3 KB, and stays local like the other imported art. `build.ps1 -FtlArchive` reruns the importer.
- Icons are tinted the way the original tints them:
  - **green**: working and powered
  - **orange**: damaged
  - **red**: destroyed
  - **grey**: unpowered (for example, the medbay at the start)
  - **blue**: ionized
- Damaged and destroyed rooms also get a light orange or red floor tint.
- **Enemy icon tint and sensors.** Enemy rooms always show which system they hold, because the target card already treats that as public. Below sensors level 3 their tint is neutral: the existing rule hides enemy system health and ion state until level 3, and the `ion_ui_smoke` and `subsystems_ui_smoke` checks enforce it. With level-3 sensors, or level 2 manned (see item 5), enemy icons and damage tints are shown in full.

### 2. Crew pathing through doors
- **Already there:** routing was already door-only. Room adjacency is built from doors, not from rooms that touch, and every step passes through `crew_pass_door`. What was wrong was the drawing: crew slid straight from one room slot to the next, cutting across walls.
- **Now:** each step is animated along a three-leg path:
  1. from the crewmember's slot to the tile beside the connecting door;
  2. through the door;
  3. to the slot in the next room.
- This applies to your crew, enemy crew, boarders and intruders, on both ships. When a room has two doors, the nearer one is used.
- A crewmember who cannot move on waits at the door instead of walking into the wall. That happens when the next room is full, the door is sealed against them, or a Crystal lockdown blocks it. The new `game_crew_blocked` reports this case.
- Enemy doors are now drawn on the aim screen for rooms your sensors can see.

### 3. Bottom-screen combat view
- **Incoming fire, home view:**
  - Each shot in flight is drawn travelling in from the right: a red laser, a gold missile or a cyan ion.
  - The room it is heading for gets pulsing corner brackets.
  - Bombs show a pulsing ring and beams draw their line across the rooms.
- **Your own fire:** shots in flight leave your weapons room toward the enemy.
- **Aim screen:**
  - Your shots come in from the left toward their target room, which is bracketed.
  - Each shot then lands with a tag: **HIT** (or **ION**), **MISS**, or **SHIELD**. A SHIELD absorb also draws the shield bubble around the enemy.
  - A hit flashes the room, and the room keeps its damage tint afterwards.
  - Tags move aside so they don't overlap, and stay above the lower panels.
  - Interceptions stay a top-screen event, as before.
- **Fires:** one flame is drawn per burning tile, up to the room's 4-fire cap. Breaches show one marker each, with venting particles while the room still has air. Room oxygen still tints the floor toward red.
- **Fire rules:**
  - **Already there:** fires need at least 10% room oxygen to burn and use up air every second. They damage the room's system every 3 s while they have air, hurt crew in the room, and spread to adjacent rooms through doors (open doors most easily). They go out one per second below 10% oxygen, so a vacuum smothers them. Crew put them out at 3 crew-seconds per fire.
  - **New:** fire also spreads to other tiles of its own room, with a 10% chance each second while it has air. The original's fires grow like this; before V46 a room only gained fire from hits or from neighbouring rooms.
- **Breaches:** already vented oxygen and still need 5 crew-seconds of repair each.

### 4. Weapons need a manned weapons room (house rule)
- **Rule:** a weapon volley starts only while a crewmember of that ship's own side is standing in its weapons room. A crewmember still walking there does not count.
- **What it covers:**
  - your manual fire and autofire;
  - enemy loadouts and the legacy single enemy weapon;
  - each flagship artillery room, which is manned by its own isolated gunner.
- **What it doesn't stop:**
  - Charging continues, so a weapon fires as soon as someone arrives.
  - A volley that has already started finishes.
  - The original's manning charge bonus is unchanged.
- **Automated ships** (Auto Scout, Auto-Assault, drone ships, and the flagship after its AI takeover) count as always manned, because they have no crew.
- **Shown as:**
  - **UNMANNED** in red on your weapon panels;
  - a blinking red outline on your weapons room;
  - FIRE greyed out;
  - "Weapons room unmanned. Station a crewmember." if you try to fire.
  - With sensors, enemy weapons show **UNMAN** (or **WPN UNMANNED** for the legacy enemy weapon) and artillery shows **<kind> UNMAN**.
- **Enemy manning order:** enemy AI now staffs weapons before shields: pilot 64, weapons 65, shields 66, engines 67. A three-crew Rebel ship therefore staffs pilot, weapons and shields as before, but a depleted crew keeps its gunner instead of its shield operator.
- **Checked:** with an idle player, every enemy kind fires within 45 s. All flagship artillery rooms start manned in all three stages.
- **Deviation from FTL 1.6.14:** in the original, manning weapons only speeds up charging and unmanned weapons still fire. Blite asked for this rule. Its side effect: killing an isolated flagship gunner (for example by boarding) now silences that artillery room. A dead gunner is not replaced while any flagship crew survives, so this makes the finale somewhat easier. With TUNED, the balance check still passes its stage-win thresholds.

### 5. Aim-screen results and sensors (follow-up request)
- **Shot results:** each shot's hit, miss or shield absorb, and the room damage tint, are covered by item 3.
- **What sensors already did:**
  - Level 1 shows your own interior.
  - Level 2 or higher, while working, shows the enemy interior: crew sprites and positions, room oxygen tint, fires and breaches.
  - Level 3 adds enemy weapon charge times, system health and ion timers.
  - Enemy crew were already drawn at level 2, so no crew drawing was missing.
- **New:** a working sensors room manned by your crew reads one level higher (`game_sensor_level`). For example, level 2 manned gives the level-3 view. A destroyed sensors room gains nothing from being manned. A crewmember who is walking there or fighting an intruder does not count.

## Saves
- No save format change. Format 27 is unchanged, saves 2–26 still migrate, and installed FTL files are untouched.

## Build and evidence
- **Tests:** 132 host checks passed (build/v46-check.log), including the new `v46_combat_smoke`. It covers:
  - the player manning gate, autofire and walking crew;
  - the enemy gate, every enemy kind still firing, and automated ships;
  - flagship gunners at start, a dead gunner silencing artillery, and AI takeover;
  - the sensors manning bonus at levels 0–2;
  - door-only paths for every Kestrel door, in both directions and at every move tick;
  - a boarder waiting at a sealed door;
  - in-room fire growth and vacuum extinguishing;
  - icon state changes on the home view.
- **Updated tests:** these encoded the old behaviour:
  - The bots in `campaign_v10_smoke`, `species_run_smoke` and `flagship_final_smoke` now keep a gunner. The eight-sector bot also fights fires.
  - `enemy_calm_ai_smoke` uses the new manning order.
  - `flagship_smoke` measures the 2-bar artillery cooldown with its gunner alive.
  - `foundation_smoke` checks that autofire waits for a gunner.
  - `balance_probe` only fires while manned.
- 45 Python tests passed.
- **ROM:** 41,003,008 bytes. SHA-256 `905691ae568269b497c14093c93123c5dc2049e1f3cac8f80b3eb54f50118100`. The only build warning is the existing SDK one.
- **Memory:** the static ARM9 main-RAM union is 996,764 bytes, up 12 KB from V45. Game is 19,456 bytes and Campaign 23,748 bytes, both unchanged. All 17 audio files were verified byte-exact in NitroFS.
- **Previews:** these host renders were inspected: `v46-incoming`, `v46-aim-results`, `v46-aim-shield`, `v46-unmanned`, `v46-icon-states` and `v46-flagship-icons` (all `-v46.png`).

## Not yet verified
- Physical DS acceptance. Item 19 of the checklist covers the V46 visuals.
- **Small aim views are crowded.** On the flagship and other narrow hulls, room numbers on rooms without a system, the 7×7 icons and the result tags compete for space.

Recovery: releases/v45/ftl-ds.nds. Current ROM: releases/v46/ftl-ds.nds. Snapshot and hashes: checkpoints/v46-combat-readability.zip and build/v46-manifest.json.
