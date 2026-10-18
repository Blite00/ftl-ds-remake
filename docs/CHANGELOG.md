# Changelog and development notes

This is the original development log, newest first. Paths such as `ftl-ds.nds`, `releases/` or `.tools/` refer to the developer's local builds, which are never published. Checkpoint documents are in this folder.

## V70: damage audit and hit flash

V70 checks damage against the original rules, fixes direct-hit crew damage to the original 15 HP per point, and flashes the hit room red with its damage on both screens. See [V70_CHECKPOINT.md](V70_CHECKPOINT.md).

V69 shows each weapon's own target with a numbered marker, and B clears one weapon's target so it holds fire, as in the original. See [V69_CHECKPOINT.md](V69_CHECKPOINT.md).

V68 fires shots from each ship's original weapon mounts, lands hits in the aimed room, and adds the original hit sounds, damage cues and alarms. See [V68_CHECKPOINT.md](V68_CHECKPOINT.md).

V67 makes the half-built flagship's two weak artillery guns fire, using their original stats. See [V67_CHECKPOINT.md](V67_CHECKPOINT.md).

V66 adds the original "flagship under construction" event in Rebel sectors. Beating it unlocks the Federation Cruiser, so every base-game ship is now unlockable the original way. See [V66_CHECKPOINT.md](V66_CHECKPOINT.md).

V65 adds the hidden Crystal homeworld. The Rock homeworld's crystal beacon can send you there, earning the Ancestry achievement, and its quest unlocks the Crystal cruiser. Every base-game ship can now be unlocked the original way. See [V65_CHECKPOINT.md](V65_CHECKPOINT.md).

V64 adds the tutorial to the main menu: 26 guided Kestrel steps using the original tutorial text, covering power, crew, repairs, fires and doors, the helm and jumping, the star map, shields, weapons and targeting. It works with buttons or touch, can be skipped at any time, and never touches your save. See [V64_CHECKPOINT.md](V64_CHECKPOINT.md).

V63 shows event replies as numbered lines in one window, as in the original, and fixes saving on real DS hardware: after start-up, NitroFS had redirected file writes away from the SD card. This was found and verified in melonDS with DLDI. See [V63_CHECKPOINT.md](V63_CHECKPOINT.md).

V62 adds the remaining common event features: event stores, crew leaving, boarders beaming aboard, system status limits and named story crew. Choice boxes now fit long original labels. Each people's homeworld can now host the original story event that starts its ship-unlock quest (Stealth, Rock, Mantis, Slug and Zoltan cruisers). 130 original events are enabled, and saves use format 32 (older saves still migrate). See [V62_CHECKPOINT.md](V62_CHECKPOINT.md).

V61 adds original quests. Some events now mark a QUEST beacon on the map (or in the next sector), and visiting it plays the quest. Unique quest ships, enemy crew species mixes and fleet delays are supported too. 98 original events are now enabled, and saves use format 31 (older saves still migrate). See [V61_CHECKPOINT.md](V61_CHECKPOINT.md).

V60 adds the original beacon hazards. Asteroid fields send asteroids at both ships. Suns throw solar flares that start fires. Nebulas blind your sensors to the enemy, and ion storms also halve both reactors. Hazard events now compile (79 original events are enabled, up from 59), and hazard beacons on the map draw from them. See [V60_CHECKPOINT.md](V60_CHECKPOINT.md).

V59 adds the original ship achievements: three per cruiser, 27 in all, for example the Kestrel's Tough Little Ship or the Engi's Robotic Warfare. Earning two of a cruiser's three unlocks its Type B layout in the hangar, as in the original. Achievements are kept in the SD profile file, the hangar shows your progress, and saves now use format 30 (older saves still migrate). See [V59_CHECKPOINT.md](V59_CHECKPOINT.md).

V58 makes every base-game ship playable. The Federation Cruiser's Artillery Beam pierces shields and fires on its own, with a power row and upgrades. Stores now sell the systems a ship is missing (shields, doors, medbay, sensors or cloaking), and Rock B starts with its own Heavy Pierce Laser. See [V58_CHECKPOINT.md](V58_CHECKPOINT.md).

V57 adds the hangar. New Run now opens a ship selection with the 9 base-game cruisers, each in Type A and Type B. The original unlock rules apply: the Kestrel is always available, reaching sector 5 with the Kestrel unlocks the Engi Cruiser, the other ships come from their original quests, and Type B layouts come from achievements. Locked ships show as silhouettes with their unlock condition. Unlocks are kept in a separate SD profile file (`/ftl-ds-profile.dat`) with a backup. All 18 layouts are imported with their original crew, systems, weapons, drones and augments. Every ship except the Federation Cruiser can already be played once unlocked; the Federation Cruiser needs its Artillery Beam, which comes next. Hull and System Repair drones now also work between fights. See [V57_CHECKPOINT.md](V57_CHECKPOINT.md).

V56 completes the base-game drones. The new ones are Combat Drone Mark II, Defense Drone Mark II (which also shoots down lasers), the Anti-Ship Beam Drone and the Hull Repair Drone. There are also three drones that act on board: the System Repair Drone, the Anti-Personnel Drone, which hunts intruders, and the Boarding Drone, which breaches an enemy room and sabotages its systems. These three are drawn in their room. Stores sell a drone schematic in their second drone offer, and original event drone rewards now give drones. See [V56_CHECKPOINT.md](V56_CHECKPOINT.md).

V55 adds the player cloaking system and augments. Stores sell cloaking (150 scrap), which can then be upgraded to levels 2 and 3 on the ship page. In combat it is powered and activated from the Actions menu: while cloaked you get +60% evasion and enemy weapons stop charging. Firing costs 2 s of cloak time, and the system recharges afterwards. 20 base-game augments work in three slots, sold in stores and given by events (for example Scrap Recovery Arm, Zoltan Shield, Weapon Pre-igniter and Adv. FTL Navigation). Saves use format 29, and older saves still migrate. See [V55_CHECKPOINT.md](V55_CHECKPOINT.md).

V54 redraws the interface in the original FTL style: original window frames and textures on every panel and button, the original resource icons with numbers, event replies as separate highlighted boxes, original-style power bars and the original title art on the main menu. Touch targets and controls are unchanged. See [V54_CHECKPOINT.md](V54_CHECKPOINT.md).


V53 draws beacon events from the original sector event lists. 57 original events are now enabled, up from 12, with blue options for systems and weapons, weapon rewards, system damage and up to six choices. Saves use format 28, and older saves still migrate. See [V53_CHECKPOINT.md](V53_CHECKPOINT.md).


V52 adds 32 original enemy ships, including the base pirate variants: Slug, Rock, Mantis, Federation, Engi, Zoltan, Crystal and the Rebel Elite. Each keeps its original layout, systems, species crew and weapon list, and some now carry cloaking, teleporters and drones. Ships are drawn from their sector's peoples. See [V52_CHECKPOINT.md](V52_CHECKPOINT.md).


V51 adds the original base-game weapon list: 36 player weapons covering lasers, ion, missiles, beams and bombs, including Hull Smasher, Pegasus, Glaive, Fire and Anti-Bio beams, Healing and Lockdown bombs. Stores stock them, and enemies carry them from sector 3, including beams and bombs. Flak is Advanced Edition-only and is not included. See [V51_CHECKPOINT.md](V51_CHECKPOINT.md).


V50 redraws the beacon map, the sector overview and the next-sector chooser in the original FTL star-map style, using the original map art: galaxy backgrounds, beacon, ship, boss, hazard and distress icons, the red rebel fleet zone, labelled STORE, EXIT and BASE beacons, and the sector map. Controls, routes and saves are unchanged. See [V50_CHECKPOINT.md](V50_CHECKPOINT.md).


V49 removes the V48 replacement transfer: a dead flagship gunner silences that sealed artillery room for good. See [V49_CHECKPOINT.md](V49_CHECKPOINT.md).


V48: when a flagship gunner dies, another surviving flagship crewmember takes over the artillery room. If there is a door path they walk in. Otherwise, because the original artillery rooms are sealed, they transfer in after a visible 10 s delay (shown as `LAS MAN8` and a blinking room). [V48_CHECKPOINT.md](V48_CHECKPOINT.md) also lists which rooms currently need crew. See that file for details.


V47 shows enemy system damage colours at any sensor level, as in the original, and reduces clutter on narrow aim screens such as the flagship. See [V47_CHECKPOINT.md](V47_CHECKPOINT.md).


V46 makes combat easier to read on the bottom screen.
- **Room icons:** each system room on both ships shows its original FTL icon, tinted by state: green when working, orange when damaged, red when destroyed, grey when unpowered and blue when ionized.
- **Crew movement:** crew walk through doors instead of cutting through walls.
- **Shots:** shots in flight are drawn with markers on their target rooms. On the aim screen, each of your shots lands with a HIT, MISS or SHIELD tag and the damaged room is tinted.
- **Fire:** fire now spreads within its room. As before, it needs air, uses it up, and goes out in a vacuum.
- **Sensors:** manning the sensors room adds one sensor level.
- **Manned weapons (house rule, by request):** weapons only fire while someone is in the weapons room, and an UNMANNED warning shows when nobody is. Saves are unchanged.

See [V46_CHECKPOINT.md](V46_CHECKPOINT.md).


V45 adds a FINALE option to the main menu. It chooses the flagship difficulty for a new run: TUNED (the default DS balance from V44) or ORIGINAL (FTL 1.6.14 normal numbers). Each run keeps its choice in its save, and older saves load as TUNED. V45 is the final build of this work. Host memory, stack and timing measurements are in [HARDWARE.md](HARDWARE.md), and the short physical test pass is [DS_TEST_CHECKLIST.md](DS_TEST_CHECKLIST.md). See [V45_CHECKPOINT.md](V45_CHECKPOINT.md).

V44 completes the flagship with its stage-3 Zoltan super shield and the 7-laser surge, where every 4th surge restores the shield, which also blocks your teleporter. The authentic numbers proved unwinnable for DS builds, so a DS tuning is the default (V45 makes it a menu choice). See [V44_CHECKPOINT.md](V44_CHECKPOINT.md).

V43 gives the flagship's second stage its original drone system: defense, combat, beam and boss boarding drones with parts and power limits, plus the 20-30 s drone-swarm power surge with a warning. See [V43_CHECKPOINT.md](V43_CHECKPOINT.md).

V42 replaced the interim finale platforms with the original Rebel flagship: imported stage hulls, artillery rooms, stage-1 cloaking, the original 11-crew placement with isolated gunners and per-room survivors, AI takeover, and an invasion teleporter in stage 3. Drones, surges and the super shield come in V43–V44. V44 changes the default difficulty to a DS tuning; see [V42_CHECKPOINT.md](V42_CHECKPOINT.md) and [V42_FLAGSHIP.md](V42_FLAGSHIP.md).

V41 brought the original Last Stand sector: the rebel flagship advances on the Federation base every second jump, rebel control spreads, and the base falls three jumps after the flagship arrives unless you engage it. Phase battles still use the interim platforms. See [V41_CHECKPOINT.md](V41_CHECKPOINT.md) and [V41_FINAL_SECTOR.md](V41_FINAL_SECTOR.md).

V40 added original calm-ship healing (crew heal below 99% when nothing aboard is wrong) and priority-based manning, so small or depleted crews staff pilot, shields and weapons first. See [V40_CHECKPOINT.md](V40_CHECKPOINT.md).

V39 brought enemy boarding in line with FTL 1.6.14: half the crew boards, at most two sends, random landing rooms, fighter-first selection, and original retreat and forced-evacuation rules. See [V39_CHECKPOINT.md](V39_CHECKPOINT.md) and [V39_BOARDING_AI.md](V39_BOARDING_AI.md).

V38 replaced authored enemy aiming with original normal-difficulty targeting recovered from FTL 1.6.14: a 33% system priority, otherwise random rooms; distinct rooms per volley; independent weapon timing; and two-room beam lines. See [V38_CHECKPOINT.md](V38_CHECKPOINT.md) and [V38_WEAPON_AI.md](V38_WEAPON_AI.md).

V37 brought enemy healing, repair priorities, defense and boarding closer to documented original behavior. See [V37_CHECKPOINT.md](V37_CHECKPOINT.md) and [AI scope](V37_COMBAT_AI.md). Full game parity remains unfinished.

V36 adds five original music tracks and twelve effects, converted for bounded DS playback. See [V36_CHECKPOINT.md](V36_CHECKPOINT.md) for evidence and pending physical DS audio checks. Original assets remain local.

V35 adds crew naming before launch, crew deselection with B or empty-space taps, elapsed-display-frame combat timing and cheaper bitmap drawing. See [V35_CHECKPOINT.md](V35_CHECKPOINT.md) for tests and physical DS checks.

V34 clears combat panels during ship destruction and moves enemy weapon information above the ships. See [V34_CHECKPOINT.md](V34_CHECKPOINT.md) for validation and remaining DS checks.

V33 replaces rectangular enemy placeholders with original FTL hull artwork and matching rooms, adds a main menu, and improves input capture for quick taps. Boot now offers Continue, New Run, Controls and Sound. Start from the sector map opens Resume Run; combat Start still opens Actions. New Run never overwrites your checkpoint until you explicitly save.

The Interceptor uses the Auto-Scout hull, Armored Drone uses Auto-Assault, and the interim Rebel Command uses the Rebel Rigger hull. Their authored combat strengths remain; Command is still the interim finale. See [V33_SHIP_ART.md](V33_SHIP_ART.md).

Input is sampled every VBlank, so distinct taps can be retained while rendering. Holds do not repeat, old-screen taps cannot activate a new confirmation, and routine diagnostic SD writes are deferred during gameplay. Actual hardware responsiveness still needs a DS playtest. See [INPUT_RESPONSE.md](INPUT_RESPONSE.md).

Current ROM: `ftl-ds.nds`. Preserved release with launcher icon: `releases/v54/ftl-ds.nds`. See [V33_LAUNCHER_ICON.md](V33_LAUNCHER_ICON.md) for icon validation. See [V33_CHECKPOINT.md](V33_CHECKPOINT.md), [V33_MAIN_MENU.md](V33_MAIN_MENU.md) and [CONTROLS.md](CONTROLS.md). The V32 bottom-screen zoom/irregular map and surrender encounters remain included. Existing saves keep their encounter policy; new runs use policy 5. The complete base-game goal and physical DS acceptance remain unfinished.

Player FTL charging and retreat now connect combat to normal campaign arrivals. Open the jump map with **B** from crew controls, or tap the FTL button. Select a linked beacon with **L/R**, then **A / JUMP**. The map holds simulation; close it to keep charging. Retreat costs one fuel, advances the Rebel fleet and grants no salvage from the battle left behind.

Away crew require explicit abandonment confirmation. Hostile boarders travel with the ship. At an unresolved arrival event, **Y / DEFEND SHIP** opens home defense and repairs without choosing an event outcome. **B / RETURN** returns to the same narrative.

Save22 preserves carried intruders and the unfinished phase of the current authored finale. Returning to that encounter restarts the same phase at full enemy hull without another entry cache. This is an interim rule; the authentic moving flagship and base-defense finale remain unfinished.

The full base-game goal remains active: wider original content, surrender, hazards, equipment, playable ships, authentic finale, progression and DS release validation. See [FULL_GAME_CHECKLIST.md](FULL_GAME_CHECKLIST.md) and [V28_CHECKPOINT.md](V28_CHECKPOINT.md). Earlier checkpoint documents retain their historical evidence and limitations.

Current validation and limitations are recorded in V54_CHECKPOINT.md. Historical checkpoint documents describe their own builds.

## Play

Open `ftl-ds.nds` in a DS emulator or on a compatible homebrew flashcart. The menu and map display **V54**. A new run uses a seed derived from the clock; that seed is shown on the map. Campaign save formats 2 through 26 migrate on load. Older formats receive missing identity/station or fire defaults. The next save writes format 28 (V41-V42 add final-sector flagship route and crew state; V45 adds the finale choice), including original event policy/uniqueness, route history/offers, teleporter, breach, subsystem and door-force state, and retains the prior valid file as backup.

New runs explore eight sectors with 23 connected beacons each. Saves from before V23 retain six sectors and twelve beacons; V23 saves retain eight sectors with their original unbranched rules. Routes are bidirectional; revisits cost fuel without repeating rewards. Stores retain stock. After eight turns in a sector, fleet pursuit begins catching earlier columns. Travel and waiting advance it; rescue advances it by two turns. An escaped warning scout advances it once more. Red captured beacons trigger patrols, including stores and exits. Choose event outcomes, carry courier codes, buy supplies and equipment, upgrade the ship, and defeat the current three-stage Rebel Command encounter at the final exit; the authentic flagship/base-defense finale remains unfinished.

If fuel runs out, **RESCUE** replaces JUMP: it supplies two fuel for ten scrap, or a small hull cost if scrap is unavailable. The fleet advances two jumps. Departure, store, courier, repair and exit locations provide a predictable framework; other events and connections vary by seed.

## Controls

### Map and events

- Touch a visible beacon, or use **L/R** to select. The zoomed view follows selection. **Up / ALL** toggles overview; **B / CENTER** returns to your ship. **A / JUMP** travels to a linked beacon.

- **X / SAVE** writes the campaign at a map checkpoint.

- **Down / WAIT** advances the fleet by one turn without spending fuel. Waiting in captured territory triggers a patrol. In new runs, waiting with no fuel outside captured territory opens the refugee contact.

- **Y / SHIP** opens ship management. **Start** opens drone controls; HELP is available in ship management.

- Tap an event choice, or use **Up/Down** to select and **A** to confirm. Disabled choices show unmet requirements. Sector exits display destinations and encounter hints.

- **L/R**, or PREV/NEXT TEXT, reads additional narrative pages when shown.

- **A / CONTINUE** leaves resolved events. Unresolved events and sector routes require confirming a choice.

- After a resolved event, **Y / SHIP MANAGEMENT** lets you prepare before continuing.

- At a store, tap an offer or use Up/Down and A; **L/R / MORE OFFERS** changes offer pages. **B / LEAVE** returns to the map.

- Victory/defeat: **Start / NEW RUN** begins a fresh seeded run. Existing manual saves remain until replaced by another save.

### Rewards

Use Up/Down or touch to select free space or a specific cargo/crew slot, then A / ACCEPT. Occupied slots require confirmation showing what will be replaced. B / DECLINE also requires confirmation; B cancels either confirmation. Resolve rewards before continuing the event or saving at the next map checkpoint.

### Combat: buttons or touch

- **D-pad:** choose a room. Selection alone does not issue an order.
- **L/R:** choose crew on the crew screen, or a weapon while aiming.
- **A:** move the selected crew to the highlighted room, or fire while aiming.
- **Y:** switch between crew and aiming; **X:** pause/resume.
- **B:** back; from the crew screen, open the retreat map.
- **Start:** open Actions. Time is held while this menu is open.
- **Actions:** Up/Down selects; A confirms; Left/Right adjusts system power. Use it for weapon power, autofire, doors, boarding, drones, retreat and volume. B closes it.
- **Beam:** use D-pad and A to mark two endpoints, then A fires; Actions can reset the line. **Bomb:** Actions switches between enemy and home targeting.
- **Touch:** existing crew, room, weapon, power and fire buttons remain usable alongside controller input.
- **Select:** diagnostics, holding simulation.

Full system names and impact feedback explain target effects. Enemy interior/system details still require the appropriate sensor level. Upgrade/equipment and name-entry screens retain some touch-only operations; this release does not claim complete controller coverage of every screen.

### Ship management

Tabs provide **CREW**, **UPGRADE**, **EQUIP** and **HELP**.

- Crew page uses the same ship, crew, doors and reactor controls as combat. Repairs, oxygen and healing advance only while this page is open and unpaused. Events, shops and other management pages freeze simulation. The explicit DEFEND SHIP view also runs home repairs and defense during unresolved arrival events.

- Upgrades increase installed system capacity or reactor output. Allocate power separately afterward. Damaged systems still need repair.

- Cargo shows installed weapon names, power needs and volley damage on the top screen. Select a cargo item and tap an installed slot to swap, or the next empty slot to add it.

- Stores sell fuel, ammunition, hull repairs, species recruits, weapons and drone equipment. Purchases go into cargo. On EQUIP, select an installed or cargo item, choose SELL SELECTED and confirm its name and price. Sales require a store.

- **MAP / STORE / RETURN** or **B** leaves management. **VOL** cycles OFF, LOW, MED and HIGH. HIGH is the default.

## Implemented subset

- Kestrel with three starting humans, up to eight crew and four installed weapons plus four cargo slots.

- Artemis, Burst Laser II, Basic Laser, Heavy Laser, Ion Blast I, Halberd Beam and Small Bomb. Selected catalog values are imported; timing/probability adaptations are documented in the versioned checkpoints.

- Piloting, station bonuses, simple skill growth, repairs, healing, doors, room oxygen, suffocation, shields and projectile combat.

- Original Auto-Scout, Auto-Assault, crewed Rebel Fighter and Rebel Rigger, plus procedural Interceptor, Armored Drone and Rebel Command variants. Assault uses its original layout and hull art, starting shields and one supported Combat I drone.

- Seeded sectors, pursuit, typed conditional events, a courier quest, stores, upgrades and a three-stage finale.

- Versioned campaign saves, validated backup recovery, bounded logs and synthesized audio.

See [CONTENT.md](CONTENT.md) and [COMBAT.md](COMBAT.md) for exact rules and approximations. Fire/breaches, species, boarding, ion/beam/bomb weapons, player Combat I and Defense I drones are implemented in the supported subset. Cloaking, augments, other drone roles and additional playable ships remain required. Advanced Edition remains outside scope. The finale is simplified; it does not reproduce the original flagship's special mechanics.

## Saves and recovery

The current build writes **`/ftl-ds-campaign.sav`** on homebrew SD storage. V9's **`/ftl-ds.sav`** remains untouched and is not imported. This is an intentional separate format for the expanded campaign.

Saving is map-only; active battles and unresolved choices are not saved. Saves contain explicit bounded fields, checksums and a layout/version check. Writes stage through `.tmp` and retain the prior valid primary as `.bak`. If the primary is missing or invalid, load tries the validated backup. When replacing an owned corrupt primary, `.bad` retains that corrupt file and the valid backup remains available. Foreign files are not overwritten.

The filesystem must be writable and support homebrew DLDI. A save is stored inside the emulator's configured SD storage, not automatically beside the Windows ROM. Physical SD power-loss durability remains unverified.

## Build and checks

The local portable BlocksDS/Wonderful/MSYS2 toolchain lives in `.tools/`.

```powershell

.\build.ps1

.\tools\check.ps1

.\tools\preview.ps1

python tools/memory_report.py

```

`check.ps1` runs the host checks in a fresh disposable directory under `build/`, preventing test files from replacing user saves. `preview.ps1` renders the actual framebuffer code and exports PNGs. These images are host previews, not emulator screenshots.

To regenerate the local asset subset:

```powershell

.\build.ps1 -FtlArchive 'C:\Program Files (x86)\Steam\steamapps\common\FTL Faster Than Light\ftl.dat'

```

Import/conversion uses Python and Pillow. The PC archive is read only. Extracted original content and ROMs remain local under ignored paths.

## Historical V12 evidence and remaining validation

Twenty-one host checks cover combat, doors, medbay, bounded rosters, activation/targeting/autofire, campaign rules, audio scheduling, diagnostics, saves and UI. Tests exercise 256 generated maps; three complete campaign routes finish with ordinary combat and economy actions. A second set of three complete runs includes 108 disk save/cold-load checkpoints. These establish the tested routes, not balance for every player or seed.

The DS ROM builds. Application compilation is clean; the existing SDK startup GNU-stack linker warning remains. Static ARM9 memory is recorded in [HARDWARE.md](HARDWARE.md); it is not free-memory or performance evidence. Runtime timing instrumentation remains available through diagnostics.

Framebuffer screens have been visually inspected. The bundled melonDS executable starts, but automation did not obtain a targetable window. Its inspected configuration has unbound buttons and disabled DLDI. Current emulator screen/input/audio checks and physical DS/flashcart validation remain pending. See [EMULATOR.md](EMULATOR.md).

Original synthesized audio is implemented and host scheduling checks pass; actual listening is unverified. This build is a playable campaign candidate pending human playtesting and hardware acceptance.

## Checkpoints and next work

- `checkpoints/v9-baseline.zip` and its SHA-256 preserve the pre-campaign source, assets and ROM.

- [IMPLEMENTATION.md](IMPLEMENTATION.md) tracks the executed plan and remaining acceptance work.

- [ROADMAP.md](ROADMAP.md) separates the current campaign subset from future coverage.

- [HISTORY.md](HISTORY.md) preserves the earlier milestone notes.

## V11 additions and full-game goal

Touch flashes and 2.5-second action messages acknowledge input and explain rejected actions. Both ships show current/effective shield layers and recharge progress. A visible FIRE alert and room flames identify burning rooms; order crew into a burning room to fight the fire, or vent it. Firefighting takes priority over repair. Fires persist between encounters and through saves; simulation still advances only in combat or the unpaused Crew management page.

Auto-Assault can appear from sector two. Its Combat I drone fires one laser every ten seconds while drone control has two healthy powered bars. Target its DRN room to disable it. Automatic repair can restore it. Timing, hull scaling and a ten-percent interpretation of source fireChance 1 are documented demake choices, not recovered engine behavior. Unsupported beam/advanced drones remain excluded.

The accepted full base-game scope and dependencies are in [FULL_GAME_CHECKLIST.md](FULL_GAME_CHECKLIST.md). New runs use eight sectors; legacy saves retain six. Later milestones expand original content, ship coverage and the finale. See [ROADMAP.md](ROADMAP.md) for current evidence and exact next steps.

## V12 crew controls

Crew cards display names. Select a crew member, then tap their card again to open details. This panel holds simulation and shows health, current action, room, saved station and station skill progress. Rename uses a bounded touch keyboard; CANCEL/B discards edits and DONE commits a valid name. SAVE STNS/X saves all living crew positions; RETURN/Y orders them back. CLOSE/B resumes the prior screen. Names, identities and stations persist through encounters and saves; replacement recruits receive new identities.

The original Rebel Fighter can appear from sector two. Its three humans man stations, repair damage, fight fires and seek its medbay. Crew presence and health affect its behavior, and defeating the crew preserves the enemy hull. The current reward is the same ordinary salvage amount, awarded once. Activity frames are procedural overlays on the existing local crew sprite; full directional/combat/death animations remain on the checklist. Enemy crew information is gated by sensors and local crew sight, as recorded in V15.

## V13 boarding controls and current validation

Buy a teleporter for 120 scrap through **SHIP / UPGRADE** at a store. Additional capacity bars cost 40 scrap each, up to three. Upgrades are also available on the map or after a resolved event. Allocate power separately.

Open **BOARD** from crew controls. Select up to two crew, use **GATHER** to order them to the teleporter, and wait until both are stationary there. **P-/P+** adjusts power. Use **ENEMY** to choose a destination room and **SEND / A** to deploy. Selecting rooms orders your away party around the enemy ship. **RECALL / Y** returns selected survivors from the same room after recharge. **HOME** shows your ship, **X** pauses and **B** closes the panel. Recharge takes 20/15/10 seconds at one/two/three powered bars and stops without functioning power.

Hostile boarders fight and sabotage aboard your ship. Friendly crew sharing their room fight automatically; use the intruder alert and HOME view to direct defenders. Destroying a hull kills everyone aboard it. Destroying the enemy hull does not remove surviving hostile boarders from your ship.

After a crew-only victory, recall away survivors before continuing. **ABANDON** requires a second confirmation and permanently loses all away survivors. Losing your last crew ends the run. Hull-destruction outcomes show a two-second blast/debris sequence before rewards or defeat; crew-only outcomes skip explosions.

Saving remains map-only. Escape, surrender, species, door resistance and sensor visibility remain unfinished. V13 passed 27 host checks and the ROM build; emulator and physical acceptance remain pending; [V13_CHECKPOINT.md](V13_CHECKPOINT.md) tracks the gate. Earlier full runs are not V13 evidence.

## V14 breaches and weapon properties

A **BREACH** alert and room markers identify hull leaks. Order friendly crew into an affected room to seal it; firefighting and combat take priority. Breaches vent oxygen even with the airlocks closed. Seal progress and remaining breaches persist through map saves. Heavy Laser I now charges in nine seconds and needs one power bar; its two-damage shot can cause fires and breaches.

Separate hull/system/crew damage is now carried by each launched shot, so queued shots retain their launch properties. Raw original fire/breach values are imported locally; probability interpretation and crew-damage rates remain documented demake choices. Advanced Edition stun remains excluded. V14 passes 30 host checks, including complete campaigns and 108 cold loads; the ROM builds and affected previews were inspected. See [V14_CHECKPOINT.md](V14_CHECKPOINT.md). Emulator and physical DS acceptance remain pending. Powered medbay healing continues during sealing, firefighting and melee. Captured-ship oxygen and fires continue while your away crew await recall.

## V15 doors and sensors

On **SHIP / UPGRADE**, open **DOORS / SENSORS UPGRADES**. Doors cost 35 then 50 scrap; sensors cost 25 then 40. Each caps at level three and uses no reactor power. Damaged levels must be repaired by crew at their original rooms: door control room 2 and sensors room 3 in the internal layout (displayed room numbers add one).

Level-two/three closed doors delay intruders until forced. Forced doors remain visibly stuck open for five seconds before closing; **SEAL ALL** skips them. If door control is destroyed, remote door buttons stop working until repaired. Friendly crew can still move through doors.

Sensors level one reveals your interior, level two reveals enemy rooms, and level three reveals detailed enemy equipment information. When sensors cannot reveal a room, your living crew present there still provide local sight. Hidden interiors remain targetable; they do not reveal occupants or hazards. Upgrading visibility does not alter the underlying simulation.

These resistance/visibility policies are documented demake behavior. V15 passes 33 host checks, complete campaign/save runs and the ROM build; affected framebuffer previews were inspected. See [V15_CHECKPOINT.md](V15_CHECKPOINT.md). The full game remains unfinished; six humans and the six-sector campaign are still the current limits.

Static room functions remain visible for targeting even when interiors are hidden. Level three shows actual enemy weapon allocation, required power and charge. Door upgrades also reduce fire spread through closed doors; open doors continue to allow spread. Repeated remote close commands do not erase active forcing progress.

