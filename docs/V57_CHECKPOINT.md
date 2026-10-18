# V57 hangar, ship selection and SD profile

Step 5 of the plan, part 1: a hangar to choose the ship, the original unlock rules, a persistent profile file on the SD card, and a player ship built from data instead of hard-coded to the Kestrel. This build also includes Blite's fix: repair drones now work between fights.

## Completed
- **Hangar.** New Run opens the hangar instead of going straight to crew naming.
  - **Layout:** the 9 base-game cruisers in a 3×3 grid in the original order (Kestrel, Stealth, Mantis, Engi, Federation, Slug, Rock, Zoltan, Crystal), each with Type A and Type B.
  - **Top screen:** the ship's name, class and hangar art, plus its starting hull, reactor, missiles, drone parts, crew species, weapons, drones and augments.
  - **Locked layouts** show as a dark silhouette. The original unlock condition appears in full on the bottom screen.
  - **Controls:** D-pad picks the ship, Y/L/R switches Type A/B, A starts, B goes back. Every button can also be touched.
  - **Crew naming** follows the chosen ship: the art, the name and 2–4 crew rows.
- **Original unlock rules (base game).**
  - **Kestrel A:** always available.
  - **Engi Cruiser (The Torus):** reach sector 5 with the Kestrel. This works now.
  - **Stealth, Mantis, Federation, Slug, Rock, Zoltan and Crystal:** unlocked by their original quest events (`unlockShip`). The compiler and runtime handle these now. The quest events themselves arrive with the planned quest batch.
  - **Type B layouts:** need 2 of the 3 achievements for that cruiser. Achievement tracking is the next step. The text comes from the installed game data.
- **SD profile.** Unlocks are stored in `/ftl-ds-profile.dat`, separate from the campaign save.
  - **Format:** magic, version and checksum.
  - **Backup:** each write goes to a temp file, is read back to verify it, and then replaces the file. The previous good version is kept as `/ftl-ds-profile.dat.bak`.
  - **Recovery:** a damaged file falls back to the backup, then to the defaults (Kestrel A only). The Kestrel can never be locked.
  - **When it updates:** unlocks earned during a run are written at once, with an on-screen message.
- **All 18 base layouts imported.** `tools/import_player_ships.py` reads each layout from the original files:
  - rooms, doors and systems;
  - starting crew species, weapons, drones, drone parts and augments;
  - weapon and drone slots.

  Each layout also gets an art pack in NitroFS (`nitrofiles/ships/*.bin`, 1.65 MB in total): the tactical hull with floor, the combat image, the hangar image and the room furnishings. Only the chosen ship's art is in RAM. The Kestrel A pack is pixel-identical to the previous built-in art, which remains as a fallback.
- **Data-driven player ship.**
  - **Setup:** `game_init_ship()` builds any layout, including teleporter, drone control and cloaking when the ship starts with them.
  - **Missing systems:** ships can lack shields (Stealth), doors (Rock B), sensors (Mantis A, Slug A/B, Engi B) or a medbay (Slug B), as in the original. Without sensors you only see your own rooms that have crew in them.
  - **Labels:** room labels, furnishings and the "KESTREL" labels now follow the current ship.
  - **Two-crew ships:** the second crew member starts in the weapons room (house rule: weapons need a gunner).
- **Saves.** The format is unchanged (29).
  - **Layout id:** a save names its layout by the id of the layout's starting rooms and doors. Kestrel saves keep their old id, so V56 saves still load.
  - **Validation:** checks now allow each layout's missing systems, lower reactors (the Zoltan starts with 5) and systems that start installed.
- **Repair drones between fights (Blite's fix).** The Hull Repair and System Repair drones can be deployed and work outside combat while the crew page is open and unpaused, like repairs, oxygen and healing. Combat drones still need a fight.
- **Kestrel drone parts.** The Kestrel now starts with its original 2 drone parts (from the blueprint). Older saves that never stored parts still load with 0.

## Not included yet
- **Federation Cruiser A and B:** shown and unlockable, but they can't be started until the Artillery Beam system exists (next build).
- **Missing weapon:** Rock B's unique Heavy Laser (+1 shield piercing) is temporarily the standard Heavy Laser I.
- **Buying systems:** stores don't sell the systems a ship lacks yet (shields, medbay, sensors, doors). Their rooms are already reserved, and saves accept them.
- **Achievements:** not tracked yet, so Type B layouts can't be unlocked yet.
- Type C layouts and the Lanius cruiser are Advanced Edition and excluded.

## Evidence
- **Results:** 138 host checks (new: `v57_hangar_smoke`) and 45 Python tests passed. The new test covers:
  - the profile's round trip, backup restore and rejection of bad files;
  - all 18 starting loadouts;
  - 40 s of combat on every playable layout;
  - a save/load round trip of every playable layout;
  - the Engi unlock at sector 5;
  - hangar keys and touch;
  - the 4-crew setup;
  - repair drones between fights.
- **ROM:** 43,518,976 bytes. SHA-256 `7ea806bec2c54addb7415a6914f642ca45f80b8ee191c9534964910cb9c649de`. All 18 art packs are in NitroFS. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,004,388 bytes (was 1,828,700). This covers the per-layout art buffers and a scratch layout used to identify saves.
- **Previews inspected:**
  - the hangar: Kestrel, a locked silhouette, an unlocked Engi and the Federation;
  - Crystal crew naming;
  - 14 layouts in combat;
  - the Engi on the star map.

## Not yet verified
- **Physical DS:** the NitroFS loading time of a ship's art pack (about 90 KB, once per run start or load) and the readability of the hangar on hardware.

Recovery: releases/v56/ftl-ds.nds. Current ROM: releases/v57/ftl-ds.nds. Snapshot: checkpoints/v57-hangar.zip and build/v57-manifest.json.
