# V50 original-style star map

Blite asked for the beacon map and the next-sector chooser to look like FTL's star map. Only the presentation changed: map layout, links, pursuit, RNG, touch and button controls, and saves all work as before.

## Completed
- **Art:** `tools/import_map_art.py` converts the original `img/map` art into `generated/map_art.h` (185 KB, local only). `build.ps1 -FtlArchive` reruns it. The converted art is:
  - the three galaxy zone backgrounds, dimmed to 45%;
  - the yellow (unvisited) and blue (visited) beacon diamonds;
  - the red rebel triangle, hazard, distress warning, ship and flagship (boss) icons;
  - the nebula clouds;
  - the civilian, hostile and nebula sector legend dots.
  - Icons keep a 4-bit alpha channel so their glow blends over the background.
- **Beacon map (bottom screen):**
  - **Layout:** a framed BEACON MAP panel over the sector's zone background (chosen from the seed and sector).
  - **Sector:** the header shows the sector number and its type in the original colours: green civilian, red hostile, purple nebula. Uncharted (nebula) sectors also show the purple nebula clouds.
  - **Routes:** dotted white routes, with routes out of the current beacon in green.
  - **Beacons:** your ship icon marks the current beacon. Beacons show the visited/unvisited diamond, or the hazard, distress or rebel icon. Jump targets get green brackets and the selection gold brackets.
  - **Labels:** STORE, EXIT, BASE, QUEST and DISTRESS labels in the original box style. The overview shows them as single letters.
  - **Rebel fleet:** the original's red striped overlay is drawn as hatched red discs over rebel-held beacons, with dashed red rings for the beacons it takes next turn. The flagship shows its boss icon.
- **Sector overview (top screen):** the same art at overview scale, plus the existing zoom window and status lines.
- **Choose next sector:** the top screen now shows the original-style SECTOR MAP: a dark red panel with the route taken so far as coloured sector dots, your ship on the current sector, the offered sectors (the selected one bracketed and named), the remaining columns, and a legend. The choice buttons, their touch areas and controls are unchanged.
- **Font:** the existing compact pixel font is kept. The original `.font` files were not converted.

## Evidence
- **Tests:** 132 host checks and 45 Python tests passed (build/v50-check.log).
  - `map_zoom_smoke` now looks for the V50 rebel-zone rim instead of the old threat ring.
  - Touch-to-beacon selection, routes and saves are unchanged and still covered.
- **ROM:** 41,191,424 bytes. SHA-256 `5869c79c0e4f8bad4a9002d94865693a3447bd12769f4817aba09e4734afdba2`. The only build warning is the existing SDK one.
- **Memory:** the static ARM9 union is 1,185,300 bytes (+188 KB for the map art) of the 4 MiB main RAM. All 17 audio files are byte-exact.
- **Previews inspected:** `map`, `map-overview`, `map-pursuit-zoom`, `final-sector-map` and `sector-routes` (all `-v50.png`).

## Not yet verified
- Physical DS readability of the smaller map icons.
- **Label clutter:** label boxes can overlap rebel zones and each other when many distress beacons are close together.

Recovery: releases/v49/ftl-ds.nds. Current ROM: releases/v50/ftl-ds.nds. Snapshot: checkpoints/v50-star-map.zip and build/v50-manifest.json.
