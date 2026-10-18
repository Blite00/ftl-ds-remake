# Equipment management policy

V20 software integration verified: all 53 registered host checks, ROM build and equipment framebuffer inspection passed. Historical releases retain their own recorded behavior.

## Current API contract

- Installation, swapping, unequipping and reordering are allowed on the map, at stores and after resolved events. Unresolved choices and active combat reject management changes.
- Up to four installed weapons and four cargo entries. Unequipping requires a free cargo entry; a full hold rejects the action without changing ownership. Swapping an installed weapon with occupied cargo exchanges the items without needing a second free entry.
- Zero installed weapons is a valid management/save state. Reinstall from cargo before entering combat if the ship has no other supported way to win.
- Power priority follows installed order from left to right. Reordering is meaningful when weapon power is insufficient. Charges, queued attacks, autofire and target state must not silently attach to a different weapon during equipment changes.
- Exact-item sales are store-only. Choose installed versus cargo and an explicit item index. The policy is half the existing demake catalog price, rounded down; shop buying prices may differ. It is not a promise of original economy fidelity.
- Successful removal compacts installed slots. Other items, cargo ownership and stock must remain unchanged except for the requested transaction. Failed actions must not spend resources or lose items.

## Current reward/overflow audit

Current supplies, distress, courier, hazard, sector-exit and ordinary battle rewards give fuel, missiles, scrap or hull repair, not weapons/drones/augments. Consequently no equipment-reward overflow path exists yet. Existing weapon purchases check for free cargo before spending scrap or decrementing stock.

This is not completion of the full game's excess-loot interface. When original events and salvage can grant equipment, they need a bounded pending-loot state and explicit keep/swap/discard choices. They must not overwrite inventory, silently lose rewards or allow repeated collection.

Current resources also have save validation limits; prolonged future event chains should validate bounded reward arithmetic rather than relying on normal six-sector totals. Availability of room in cargo is distinct from numeric resource overflow.

## Validation coverage

Passing focused checks cover full-cargo rejection, exact cargo/installed sale, zero-weapon save/resume/reinstallation, power priority, state reset/preservation rules, invalid modes/indices and transaction conservation. An ordinary earned-economy campaign should exercise real purchases and exact sales, with cold loads preserving stock and equipment order. These checks do not prove future drone/augment inventory behavior.

## Focused earned-campaign evidence

`tests/equipment_run_smoke.c` passed a strict standalone host build/run. It moves the original starting weapons into cargo, saves/cold-loads with zero installed weapons, reinstalls and reorders them, then completes an ordinary 36-jump six-sector campaign. Earned store purchases are sold by explicit cargo and installed index, with exact sale proceeds and unaffected item identities checked. Thirty-eight disk cold loads preserve weapon order, stock and cargo. Final fixture route: hull 30, scrap 209. No resources, damage or victories are injected.

The combined 53-check suite also passed UI/full-cargo and save checks. Emulator and physical hardware acceptance remain pending. Test files are isolated in a fresh disposable build directory.

Format 12 accepts zero installed weapons and stores their explicit order. Loading formats 2–11 moves the historical second slot first, preserving its former power priority under the new displayed-order rule. Existing reorder helpers move the complete weapon descriptor and autofire state together; replacement rebuilds the newly equipped descriptor. Future active-encounter equipment edits remain prohibited by campaign mode rules.

## V21 drone equipment

Combat I has two installed Kestrel slots and four separate cargo entries. Store purchases enter drone cargo; the control package installs Combat I directly. Fitting swaps ownership atomically, removal requires free cargo, priority swapping preserves identity, and selected sales earn25 scrap. Management actions are blocked during combat. Separate drone/weapon cargo is an explicit current demake approximation; the shared original inventory limit remains to reconcile. Save14 persists both cargo families and appended store stock. No equipment salvage/overflow reward interface exists yet.
