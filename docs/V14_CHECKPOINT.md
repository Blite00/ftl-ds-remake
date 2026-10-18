# V14 checkpoint: separate damage and breaches

October 3, 2026. The full base-game goal remains active. The current software integration passes its checks. Emulator and physical release acceptance remain pending.

## Implemented subset

- Separate hull, system and crew damage descriptors captured at launch and retained by queued volleys/projectiles.
- Original raw fire/breach fields imported for Artemis, Burst II, Basic Laser and Heavy Laser I, with explicit absent-override flags and source audit.
- Heavy Laser I corrected to original nine-second cooldown and one power bar; prices remain authored.
- Breach generation, oxygen loss, connected-room exposure and friendly sealing with fire/combat priorities.
- Breach indicators and activity labels on the DS framebuffer UI.
- Save format 6 persists player breach counts/repair progress; formats 2–5 migrate with empty breach state. Map-only policy, valid backups and separate V9 filename remain.

## Validation evidence

- 30 strict host checks pass; output `build/v14-checks.txt`, disposable directory `build/check-decf678e5b764d7c907b37710345d9b0`.
- Independent damage channels, shield filtering, queued properties, imported catalog values, bounded projectile rejection, breach leaks/sealing and fire/combat/medical priorities are covered by focused tests.
- Medbay healing applies once during sealing, firefighting and melee. Captured-ship oxygen and fires affect away survivors awaiting recall without restarting enemy orders or attacks.
- Save tests cover all-room breach/progress persistence, invalid counts/timers, format 5 zero defaults, formats 2-4 migration, exact prior-file backups and corrupt-primary recovery.
- 256 generated graphs, three complete six-sector campaigns and three resumed campaigns with 108 disk cold loads pass. A further full campaign buys a teleporter with earned scrap, deploys and recalls two survivors, and finishes the finale.
- Inspected breach HOME, targeting, sealing/details and boarding framebuffer previews. Guarded rendering tests cover maximum hazards across all current enemy layouts.
- Final ROM build passes: 447488 bytes; SHA-256 `b70ce919de60157d43529bdb8e655ce0f7e2fe24d8c40957a82d9fe7dc1d9888`. Application compilation is clean; existing SDK GNU-stack warning remains.
- ARM9 static main-RAM section union 644156 bytes; image high-water offset 644184. ARM ABI Game 10584 bytes, Campaign 11496 bytes, already included in static allocations. This does not measure runtime free memory or stack/heap high-water.
- Release: `releases/v14/ftl-ds-v14.nds`. Recoverable source/assets/ROM archive: `checkpoints/v14-damage-breaches.zip`, verified against `v14-damage-breaches.sha256`. V13 baseline hash verified before editing. Installed FTL files and existing saves were preserved.

Importer-only evidence: generated weapon properties regenerated deterministically, field values/presence matched the installed original blueprint XML, and archive size/modification time were unchanged. This proves the extracted data, not original combat semantics.

Emulator input/audio/DLDI and physical DS checks remain pending unless separately observed. Host framebuffer images are not emulator screenshots.

## Explicit behavioral limits

Raw chance units are converted to ten percentage points and fire/breach are independently rolled. Missing system overrides inherit hull damage; missing personnel overrides use five crew HP per hull point. These are documented demake policies. XML does not prove engine units, ordering or damage inheritance. Advanced Edition stun is excluded.

Rooms support up to four breaches, bounded by tile area. Friendly stationary crew seal at five crew-seconds per breach when not fighting or firefighting. Current oxygen-loss and repair rates are authored. Base-game weapon families, species, door resistance and sensor visibility remain incomplete.

The campaign is still six sectors/twelve beacons with six humans. No full-game or complete equipment/boarding coverage claim follows from this checkpoint.

## Exact next steps

1. Use the verified V14 checkpoint as the baseline for subsequent changes and retain the documented approximation boundaries.
2. Implement functional doors and sensor visibility with boarding, atmosphere and UI tests.
3. Add base-game species and eight-crew capacity, including saved identities, layout occupancy and roster UI.
4. Continue weapon/system/catalog expansion; larger generated sectors and visible Rebel fleet can proceed independently under FULL_GAME_CHECKLIST.md.
