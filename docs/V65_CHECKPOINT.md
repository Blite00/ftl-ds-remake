# V65 Crystal secret sector

The last original event feature: `<secretSector/>` and the hidden Crystal homeworld. This was delivered as a reviewed patch script (`tools/patches/secret_sector_patch.py`, drafted by a helper agent, which made 34 exact-anchor edits) and applied here.

## Completed
- **Compiler.** `<secretSector/>` is a new effect (`IE_SECRET_SECTOR = 18`), allowed only on choice-free, ship-free events. The new sector type `SECTOR_SECRET = 6` (existing values unchanged) uses the original CRYSTAL_HOME lists:
  - fights: HOSTILE_CRYSTAL, BOARDERS_CRYSTAL;
  - other beacons: NEUTRAL_CRYSTAL, ITEMS_CRYSTAL, NOTHING_CRYSTAL;
  - start beacon (new pool category 8): START_BEACON_CRYSTAL.

  ROCK_CRYSTAL_BEACON is now a Rock/Engi homeworld special.
- **Runtime.** Taking the secret route jumps straight into the Crystal sector as the next sector, with no fuel cost.
  - **Arrival:** the run earns Ancestry (Rock cruiser) and plays the original start beacon, which places the CRYSTAL_UNLOCK quest (unlocking the Crystal cruiser).
  - **Enemies:** all enemy ships there are Crystal ships.
  - **Afterwards:** the run continues to an ordinary sector.
  - **Limit:** the jump is refused when only the final sector is left.
- **Map.** The sector is labelled "CRYSTAL".
- **Saves.** The format is unchanged (32). Validation accepts the secret sector only where it can legally occur.
- **Events.** Enabled original events: 160. The count now also includes 14 events reached only as quest targets.

## Evidence
- **Results:** 131 quick host checks passed (new: `v63_secret_sector_smoke`, which covers the Rock route, Ancestry, the Crystal unlock bit, Crystal fights and pools, the save round trip, the refusal at sector 8, and format 30/31 loads), plus 46 Python tests. The helper also ran all 144 checks on a scratch copy.
- **ROM:** 43,759,616 bytes. SHA-256 `caf52d26db49c9e8e56389d904d2c54340d9f4cc3fc7e017680562e53ab2883d`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,268,468 bytes.
- **Preview inspected:** `v65-crystal-sector`.

Recovery: releases/v64/ftl-ds.nds. Current ROM: releases/v65/ftl-ds.nds.
