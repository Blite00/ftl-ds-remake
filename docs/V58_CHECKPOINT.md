# V58 Federation artillery, store systems, Rock B laser

Step 5 of the plan, part 2: every base-game ship can now be started, and ships missing original systems can buy them.

## Completed
- **Federation Artillery Beam.**
  - **Setup:** the Federation Cruiser (The Osprey, level 1) and Nisos (level 2) carry the original `ARTILLERY_FED` beam in their artillery room.
  - **Damage:** 1 per room, pierces 5 shield layers, 10% fire chance, very long sweep.
  - **Charge time:** 40 s at 1 power, 32 s at 2, 24 s at 3 and 16 s at 4 (the flagship artillery curve, scaled so level 1 is the original 40 s).
  - **Targeting:** it fires by itself across the enemy, starting from your selected target room (otherwise a random room). Aiming the line by hand is not modelled.
  - **Power:** the Actions menu gets an ARTILLERY power row (left/right). The top screen shows `ART nS`.
  - **Damage and repair:** the artillery room takes system damage and ion, and crew repair it.
  - **Upgrades:** on the Upgrade page next to cloaking, at the original prices (30/50/80 for levels 2–4).
  - **Saves:** Federation saves store its level, health and power. All other ships' saves are byte-identical to before, so the format stays at 29.
- **Shield-piercing beams.** Beams now support shield piercing. Before, only lasers and missiles had it.
- **Store systems (offer 15).** Each store sells one system the ship is missing, at the original price, installed into the room the layout reserves for it:
  - cloaking 150;
  - shields 125 (level 2, one layer);
  - doors 60;
  - medbay 50;
  - sensors 40.

  On the Kestrel only cloaking is ever missing, so its stores sell cloaking as before. Systems bought this way load back correctly.
- **Rock B (Shivan)** now starts with its own Heavy Pierce Laser Mark I: 2 damage, 1 shield piercing, not sold in stores.
- **All 18 layouts can be started** once unlocked.

## Not included yet
- **Achievements** for Type B unlocks (next build).
- **Artillery as a target:** enemy drones don't target the artillery specifically.

## Evidence
- **Results:** 139 host checks (new: `v58_artillery_systems_smoke`) and 45 Python tests passed. The new test covers:
  - artillery charge, piercing damage, power row, room damage, upgrades and the save round trip;
  - shields bought on the Stealth, doors on Rock B and a medbay on Slug B, with saves;
  - the Kestrel still being offered cloaking;
  - Rock B's laser.
- **ROM:** 43,523,072 bytes. SHA-256 `b92bc8cca2cee7c822afaaec35bf408bd8b4b634dd63d2c826cadc9d6b272992`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,010,044 bytes.
- **Previews inspected:** `v58-federation-artillery`, `v58-artillery-power-row`, `v58-upgrade-artillery` and `v58-store-shields`.

Recovery: releases/v57/ftl-ds.nds. Current ROM: releases/v58/ftl-ds.nds. Snapshot: checkpoints/v58-artillery.zip and build/v58-manifest.json.
