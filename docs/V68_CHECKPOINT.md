# V68 shot paths and hit/alarm cues

## Completed
- **Shot paths.** Shots now start at each weapon's original hull mount (`<weaponMounts>` from the layout files, all 18 player layouts, 39 enemy hulls and the flagship). Weapon slot i uses mount i.
  - **Direction:** shots travel toward the other ship, on both screens.
  - **Endings:**
    - a hit ends in the exact targeted room;
    - a miss flies past the ship;
    - a shield block stops at the shield bubble;
    - an interception stays in space.
  - **Overview rooms:** they now use each layout's own hull placement, so Type B ships line up with their art.
  - **Deviations:**
    - Enemy hulls are drawn nose-up, but their shots travel left toward the player.
    - A slot without a mount fires from the weapons room's forward edge.
    - Beams and bombs are unchanged.
- **Hit and damage cues.** All use original sounds from ftl.dat:
  - hull hit, with a brief shake and red flash on your ship;
  - shield hit;
  - system damage;
  - crew death.
- **Alarms.** These loop, one per type, and stop when the cause clears, when you leave combat or when you mute:
  - fire, with the room pulsing and "ON FIRE";
  - hull breach (air leak);
  - low oxygen ("O2 LOW!");
  - hull at 25% or below ("HULL CRITICAL");
  - intruders ("INTRUDERS DETECTED").
  The on-screen warnings use the original text.
  - **Deviations:** the warning chime repeats every 3–4 s while the cause lasts, and the fire and air-leak loops are shortened to 1 s at 11 kHz to fit the effect budget.
- **Saves:** the format is unchanged (32).

## Evidence
- **New tests:** `v68_shot_path_smoke` (6,842 room checks: origin on the hull, heading, hit room, miss, shield ellipse) and `v68_alarm_smoke` (start, no stacking, stop, one-shots once per event).
- **Audio:** effects use 769,200 of 786,432 bytes. All 22 audio files are byte-exact in the ROM.
- **ROM:** 43,925,504 bytes. SHA-256 `66e05c6152ed78555a79907cae6403211c61b64bf528b0559d96a87bb85fc46d`.
- **Memory:** the static ARM9 union is 2,324,428 bytes.
- **Previews:** `v68-shots-kestrel`, `v68-shots-enemy`, `v68-shots-typeb`, `v68-alarms` and `v68-hull-hit`.

Recovery: releases/v67/ftl-ds.nds. Current ROM: releases/v68/ftl-ds.nds.
