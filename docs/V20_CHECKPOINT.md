# V20 checkpoint: equipment management

October 3, 2026. Software integration verified; full base-game goal remains incomplete.

## Implemented

- EQUIP offers CARGO and INSTALLED lists, removal, reorder and exact-item store sales with name/slot/price confirmation.
- Up to four weapons and four cargo entries. Full cargo rejects removal without loss; zero weapons is supported in UI, simulation and saves.
- Power priority follows displayed order. Reorder carries complete weapon state and autofire with its owner; removal compacts and clears vacated state. Campaign management cannot change equipment during combat or unresolved events.
- Sale proceeds are half the current demake catalog price, rounded down. This is an authored economy policy, not verified original pricing.
- Map-only save format 12 supports empty loadouts. Formats 2–11 move their former second weapon first to retain historical power priority and enabled identity. Transient combat state resets under the existing map-save policy.

## Evidence

All 53 registered strict host checks passed (build/v20-checks.txt). Focused equipment, UI and save tests cover bounds, full cargo, mode guards, conservation, exact sales, reorder state, zero weapons and legacy migration. Existing combat and complete/resumed campaign regressions passed.

The earned equipment campaign uses ordinary purchases and sales, completes six sectors and performs 38 disk cold loads, ending with hull 30 and scrap 209. It saves with zero weapons, reinstalls and reorders, and preserves stock and ownership. This verifies the current subset, not the required eight-sector game.

Actual archived V19 source wrote a format-11 fixture; current source loaded and resaved format 12 while retaining bomb ownership, stock, resources and enabled identity. Backup SHA-256 is bedbc1598ff0a8c039969721128f7b192ffa3e8b109b6fce215afb7894c53cc6. See build/v20-release-migration.json. Fixture funds were used only for compatibility setup.

Inspected actual renderer previews: equipment-installed-v20.png, equipment-cargo-v20.png, equipment-sale-v20.png and equipment-zero-v20.png. ROM build passed with the existing toolchain executable-stack metadata warning. ARM9 static sections 684484 bytes; high-water offset 684512; Game 13360; Campaign 14416. Runtime memory and frame time remain unmeasured.

Release: releases/v20/ftl-ds-v20.nds, 479232 bytes; SHA-256 b7cc26348bb0b9d3e9ae3a8281266e8be4af5b5ec0b51ead5e06915ec351ef24.
Recoverable archive: checkpoints/v20-equipment.zip with per-file manifest and SHA-256 sidecar. Previous release archive verified and preserved.

## Limitations and next steps

1. Current rewards do not grant equipment. Add pending keep/swap/discard loot when event/salvage equipment rewards are introduced; this gate remains open.
2. Implement drone control, parts, inventory and drone lifecycle/targeting, then cloaking, augments and ship-specific systems. Enable defining playable ships only after their mechanics pass.
3. Complete eight-sector maps, visible fleet, original events, branching sectors, hazards and final base defense/flagship.
4. Finish hangar/unlocks, animation coverage, tutorial, audio/lid and full content coverage.
5. Emulator input/audio/DLDI and physical DS usability, performance and SD durability remain pending. Continue independent work.
