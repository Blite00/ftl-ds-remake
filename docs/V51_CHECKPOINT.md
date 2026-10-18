# V51 base-game weapon list

This is step 1 of Blite's content plan.

## Completed
- **The full base-game weapon list.** `tools/import_weapons.py` imports 37 weapons from `data/blueprints.xml` into `generated/weapon_catalog.h`. 36 are player weapons; index 7 is the enemy-only Artemis variant.
  - **Lasers:** Basic, Dual Shot, Burst I/II/III, Heavy I/II, Hull Smasher I/II.
  - **Ion:** Ion Blast I/II, Heavy Ion.
  - **Missiles:** Leto, Artemis, Hermes, Pegasus, Breach, Hull Missile.
  - **Beams:** Mini, Pike, Halberd, Glaive, Fire, Anti-Bio, Hull.
  - **Bombs:** Small, Breach I/II, Fire, Ion, Healing Burst, Lockdown.
  - **Crystal:** Crystal Burst I/II and Heavy Crystal I/II. These are reserved for crystal content: they are not sold or used by enemies yet.
- **Saved weapon ids are unchanged.** Ids 0–6 keep their old meaning, so saves stay valid with no format change. The old price and name table still covers those seven.
- **Not included:** flak, chain guns, charge guns and stun weapons. They exist only in Advanced Edition (`dlcBlueprints.xml`), which is outside the base-game goal.
- **New mechanics:**
  - **Hull-bust** (Hull Smasher lasers, Hull Missile, Hull Beam): double hull damage against rooms with no system.
  - **Lockdown Bomb:** seals the room for 12 s, using the Crystal lockdown.
  - **Healing Burst:** heals crew in the room, up to their maximum health.
  - **Ion Bomb:** ion damage that bypasses shields.
  - **Beams with no damage of their own** (Fire, Anti-Bio): any shield layer stops them completely, as in the original.
  - Weapon fire and breach chances come from the original data.
- **Enemies can now carry beams and bombs.** From sector 3, enemy loadouts are drawn from the list:
  - Rarer weapons become allowed as the sector number rises.
  - Each loadout must fit both the ship's weapon system and its reactor.
  - Support bombs and crystal weapons are never chosen.
  - In 150 sampled sector 3–7 fights, enemies used 24 different weapons.
- **Stores:**
  - The three offers 8–10 now stock distinct original weapons at their original cost. Rarer weapons appear less often, and the picks are fixed per run, sector and beacon.
  - Offers 4–6 stay Basic, Heavy and Burst II, which keeps the existing store-bot tests valid.
  - Legacy runs keep Ion, Halberd and Bomb in offers 8–10.
  - Loot and sales accept any player weapon.
- **Display:** weapon panels use up to 8-character short names (for example GLAIVE, PEGASUS, HULL1), and the enemy weapon line shows the full short name.

## Evidence
- **New test `v51_weapons_smoke`:** checks catalog identity, that every player weapon can be equipped, hull-bust, the lockdown, healing and ion bombs, shield-blocked zero-damage beams, enemy beams and bombs firing, and store stock and purchase.
- **Updated tests:** `enemy_loadout_smoke` (every catalog id can arm an enemy) and `target_card_smoke` (the enemy-only Artemis can't be equipped).
- **Results:** 133 host checks and 45 Python tests passed.
- **ROM:** 41,200,640 bytes. SHA-256 `3d1012815a94ce2e4c5a300f54cb9c6fe30f80ae3f8969df02554d4398cb10f9`.
- **Memory:** the static ARM9 union is 1,196,228 bytes. Game is 19,752 bytes and Campaign 24,044 bytes. Audio is byte-exact.
- **Previews:** `v51-store-weapons` and `v51-combat-weapons`.

## Next
Step 2: more original enemy ship types and crews.

Recovery: releases/v50/ftl-ds.nds. Current ROM: releases/v51/ftl-ds.nds. Snapshot: checkpoints/v51-weapons.zip and build/v51-manifest.json.
