# V54 original-style UI

Blite asked for a UI closer to the original FTL that stays readable and easy to use on the DS. Screens from now on (cloaking, augments, drones, hangar, tutorial) are built in this style.

## Completed
- **Frames everywhere.** Every panel and button on both screens now draws as the original FTL window (`img/window_base.png`), scaled to the DS:
  - **Panels:** a dark interior using the original hex texture, an original mint border (2 px on large panels, 1 px on small ones) and chamfered corners.
  - **Buttons:** solid mint with dark lettering; disabled ones stay dark with grey lettering.
  - **State colours:** yellow for selected, green for ready, red for danger, grey for disabled, blue for blue options, mint for normal.
  - **Unchanged:** panel and button positions and sizes, so every touch target and all D-pad/button navigation still works as before.
  - This covers the store, ship management, upgrades, rewards, events, map panels, combat panels and crew list.
- **Resource bar.** The FUEL, MISSILES and SCRAP labels are replaced by the original top-bar boxes: the mint icon square (fuel, missiles, drone parts, the scrap gear) next to a dark number box. Drone parts are now shown too.
- **Events.** The narrative sits in an original window, and each reply is its own framed box:
  - **selected:** yellow;
  - **normal:** mint;
  - **blue options:** blue;
  - **unavailable:** grey.
- **Power bars.** The combat power row (formerly "SHD 2/2" text) is now the original style: each system's state-tinted icon plus one segment per bar (green powered, dark empty, red damaged, blue ionized). The −/+ buttons are unchanged.
- **Main menu.** The top screen shows the original title art (`img/main_menus/main_base2.png`, letterboxed so both the fleet and the FTL logo show). Menu entries use the new frames, yellow when selected.
- **Font.** The existing pixel font is kept: the original's large fonts don't stay legible at 256×192.
- **New importer:** `tools/import_ui_frames.py` produces `generated/ftl_ui_art.h` (hex tile, four resource icons, title art; about 99 KB). `build.ps1 -FtlArchive` reruns it.

## Evidence
- **Results:** 135 host checks and 45 Python tests passed. The UI pixel tests compare states, not fixed images, and all pass.
- **ROM:** 41,817,088 bytes. SHA-256 `bc7aa45e78dd72c18faa4b070a4330023d1d2485e1081a8fd6d71df2a4c4b554`.
- **Memory:** the static ARM9 union is 1,812,100 bytes. Audio is byte-exact.
- **Previews inspected:** `main-menu-continue`, `v53-event-blue`, `store`, `combat`, `map`, `sector-routes` and `v51-combat-weapons` (all `-v54`).

## Not yet verified
- **Physical DS:** readability of the 1 px small-button borders and the 15 px resource icons on hardware.
- **Restyle coverage:** some screen headers still use the plain V53 strip, and the weapon cards keep the DS layout inside the new frames.

Recovery: releases/v53/ftl-ds.nds. Current ROM: releases/v54/ftl-ds.nds. Snapshot: checkpoints/v54-ui.zip and build/v54-manifest.json.
