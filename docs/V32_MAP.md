# V32 sector map presentation

The touch screen defaults to a horizontally enlarged (2x) view of the current sector. Selecting a beacon moves the camera to its region. The top screen keeps the whole sector visible and outlines the area shown below. Fuel, missiles, scrap, hull, crew count and fleet turns remain visible.

## Controls

- L/R or Left/Right: select a beacon; the camera follows it.
- Up or ALL/ZOOM: switch between the enlarged map and the entire sector.
- B or CENTER: return selection and camera to the ship.
- Touch a visible beacon: select the nearest beacon within a 13-pixel radius; A/JUMP confirms travel.
- Down/WAIT, X/SAVE and Y/SHIP retain their existing meanings.

Every beacon remains selectable through buttons or the full-map touch view. Selecting a beacon never travels automatically. Green routes show actual linked jumps, cyan identifies the ship, gold outlines selection, and red marks Rebel control. The beacon layout is irregular: seeded relocation changes both axes while enforcing at least 26 pixels between beacon centers in overview mode. Enlarged mode increases horizontal separation further. The selected-beacon card states jump eligibility and imminent fleet capture. Store and exit beacons retain S/E labels.

## Source and scope

The installed FTL archive was opened read-only to inspect `img/map/map_overlay.png`, `map_icon_diamond_blue.png` and `map_icon_ship.png`. The diamond and ship-marker references informed the DS presentation. Dotted connections and per-beacon pursuit rings are authored presentation choices. The renderer uses locally authored primitives; no new extracted bitmap is compiled. This is an original-inspired DS adaptation, not a pixel-exact reproduction or verified reconstruction of the original map generation.

The displayed positions are deterministic for the seed, sector and saved original coordinates. A bounded cached layout pass keeps broad left-to-right progression while avoiding repeated rows and aligned columns. It never consumes campaign RNG.

Only UI coordinates and presentation changed. Sector generation, graph connections, pursuit timing, reachable destinations, RNG and save format are unchanged. Camera state is temporary UI state. Zoom enlarges horizontal spacing while keeping the scattered beacon field within the usable touch-screen height. It is not free dragging or a new sector-route graph.

## Focused validation

`map_zoom_smoke` covers 64 generated seeds in both camera modes, every visible beacon hit-test, controller access, recentering, reproducible irregular layouts, 26-pixel minimum separation, exact threat-ring colors, render purity and framebuffer guard words. The spacing and touch checks also cover 64 legacy 12-beacon fixtures. `fleet_ui_smoke` continues to cover 23-beacon and legacy 12-beacon maps, fleet rendering and waiting. All touch fixtures use the shared display transform. Exact captured/next-turn campaign predicates drive red/gold dotted rings, including around the player ship. There is deliberately no straight fleet boundary: irregular display x is not the pursuit coordinate, so such a boundary would imply incorrect capture rules. Framebuffer fixtures cover the starting area, middle, exit, overview and pursuit. Physical DS readability and control acceptance remain pending.
