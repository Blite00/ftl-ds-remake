# V47 public enemy damage colours and a less cluttered aim screen

These are the defaults chosen for the questions left open in V46. Everything else is unchanged from [V46_CHECKPOINT.md](V46_CHECKPOINT.md).

## Completed
- **Enemy damage colours at any sensor level:** this matches the original.
  - Enemy room icons turn orange when damaged and red when destroyed, even without sensors, and the room floor is tinted to match.
  - The target card shows the enemy system's health (for example `SYSTEM 1/2`) at any sensor level.
  - Sensors still gate:
    - interior, crew and room oxygen (level 2);
    - enemy weapon charge, the powered/unpowered (grey) icon state and ion state, both blue and timers (level 3).
- **Flagship gunners:** unchanged. A dead isolated gunner still silences its artillery room, and the decision is still open with Blite.
- **Narrow aim screens:** less clutter on narrow hulls like the flagship.
  - Room numbers are hidden on rooms that show a system icon (as in V46).
  - Room numbers are also hidden on any enemy room narrower than 16 px.
  - Result tags stay short (HIT, ION, MISS, SHIELD) and still avoid each other.

## Tests changed
- `target_card_smoke` and `subsystems_ui_smoke` now treat enemy system damage as public.
- `v46_combat_smoke` checks that enemy damage changes the aim screen at sensors level 1 while ion state does not.

## Evidence
- **Checks:** 132 host checks and 45 Python tests passed (build/v47-check.log).
- **ROM:** 41,003,008 bytes. SHA-256 `93102acdecc74b58359fe7dc693cab059d802269c526dba1aa3f9480cb7c4633`. The only build warning is the existing SDK one.
- **Memory:** the static ARM9 union is 996,892 bytes. All 17 audio files are byte-exact in NitroFS.
- **Saves:** no change. Format 27 is unchanged and installed FTL files are untouched.
- **Previews:** `v46-flagship-icons-v47.png` and `v47-enemy-damage-no-sensors-v47.png` were inspected.

## Not yet verified
Physical DS acceptance (DS_TEST_CHECKLIST.md item 19).

Recovery: releases/v46/ftl-ds.nds. Current ROM: releases/v47/ftl-ds.nds. Snapshot: checkpoints/v47-public-damage.zip and build/v47-manifest.json.
