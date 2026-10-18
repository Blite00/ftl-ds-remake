# V16 checkpoint: species and eight-crew roster

October 3, 2026. Software integration verified; physical DS and emulator acceptance remain pending. The full base-game goal remains active.

## Implemented

- Bounded eight-crew friendly/enemy rosters, group selection/orders and extra crew/boarding pages. Teleport parties remain limited to two.
- Human, Engi, Mantis, Rock, Slug, Zoltan and Crystal mechanics, with per-species health/movement/work/combat and selected abilities.
- Deterministic six-species store recruitment using source prices: Human 45, Engi 50, Mantis 55, Rock 55, Slug 45, Zoltan 60 scrap. Crystal is intentionally excluded until quest/hangar acquisition is supported. No Crystal availability claim follows from test fixtures.
- Management dismissal for living crew aboard the player's ship; last survivor and away crew are protected. Removal compacts roster slots and preserves surviving identities. Dismissal does not trigger a Zoltan death burst.
- Original 14x14 species shapes, one imported downward-walk frame each, with existing procedural activity overlays.
- Map-only format 8: species, cooldown/death state and room lockdown, migration from formats 2–7 and existing backup preservation policy.

## Source facts versus authored behavior

Local original descriptions explicitly give Engi doubled repair/halved melee, Mantis 1.5x melee/1.2x movement/halved repair, Rock 150 health/fire immunity/half movement, Zoltan 70 health and occupied-system power, Crystal 125 health/20-percent slower movement. Human faster learning, Slug telepathy, Crystal reduced suffocation and active lockdown are described without all engine details.

Current authored policies include roughly ten-percent faster Human station training; 30-Hz room movement rounded to 25 ticks for Mantis, 60 for Rock and 38 for Crystal; Crystal alternating two/three HP suffocation damage against ordinary five; 12-second lockdown with a 40-second cooldown. Lockdown closes bordering doors and blocks crossing; teleportation remains possible. These rates and exceptions are not recovered engine proof.

Slug sight currently reveals its own/adjacent rooms plus rooms occupied by living enemy crew, bypassing normal sensor room visibility. Zoltan contributes local friendly-system power without reactor cost, bounded by health; its death causes 15 damage to opposing crew in the same room, once, including chains. The installed post-AE archive mentions that death behavior; it does not establish historical pre-AE applicability. This compatibility choice must remain explicit. Slug mind-control immunity and Lanius are outside scope.

Source extraction audit: `generated/species_audit.json` records blueprint prices/rarities/prose, animation rectangles and hashes. Seven frames total 2744 RGB555 bytes. Regeneration/order/cost checks passed and the enlarged contact sheet was visually inspected. This narrow asset evidence does not validate simulation or ROM integration.

## Validation evidence

The strict 36-check host suite passed on current source (`build/v16-checks.txt`), including focused species, UI, save migration and corrupted-state rejection. Existing complete six-sector campaigns and 108 disk cold-load checkpoints passed; those ordinary campaign drivers do not prove a complete mixed-species campaign or the future eight-sector game.

Inspected actual renderer previews: eight-crew final page, Crystal details/lockdown controls and eight-crew boarding selection. UI tests cover group orders, dismissal eligibility/confirmation, card bounds and away-crew order rejection. These checks do not establish physical input comfort or every possible roster arrangement.

ROM build passed (`build/v16-build.txt`). Static ARM9 sections total 665932 bytes; image high-water 665960 bytes; Game 12584 bytes and Campaign 13496 bytes. Runtime stack/heap and frame time remain unmeasured. The toolchain emits its existing executable-stack metadata warning.

Release: `releases/v16/ftl-ds-v16.nds`, 462848 bytes, SHA-256 `f0a86a5ba09268ede1e9d8045034fb60ab2390fb5be7b13a0ef332b86bce3690`.
Recoverable source/assets/ROM archive: `checkpoints/v16-crew-species.zip`, with SHA-256 sidecar. Evidence: `build/v16-evidence.json`.

Emulator input/audio/DLDI and physical DS timing, stack/heap, touch and save durability remain pending. Host images do not establish hardware acceptance.

## Remaining scope and next steps

1. Exercise mixed-species recruitment and eight-crew management through ordinary complete campaigns; broaden interactions beyond focused fixtures.
2. Add appropriate Crystal acquisition and validate species interactions in ordinary campaigns; expand full crew animation coverage.
3. Continue full weapon/system/equipment coverage and original enemy/playable-ship configurations supported by those mechanics.
4. Develop larger eight-sector maps, sector choice and visible Rebel fleet progression independently where feasible.

The campaign remains six sectors/twelve beacons. Full base-game events, equipment, hangar, flagship behavior and release acceptance remain incomplete. FULL_GAME_CHECKLIST.md remains the acceptance definition.
