# V24 sector routes and identities

## Implemented policy

New runs begin in a Civilian sector. Sectors 1–6 offer two distinct destinations selected from Civilian, Engi, Rebel and Pirate. Sector 7 offers Last Stand; sector 8 has no later route. Route offers are generated once with the campaign RNG after generating each map. Reading names, descriptions, choices or previews never consumes randomness. Taking a route records its identity in the eight-entry history, then generates its map and subsequent offers. Unvisited history entries remain zero.

The existing 23-beacon graph, two store anchors, initial supplies, courier and pre-exit repair remain available in every sector. Changing identity changes weighted non-anchor beacon kinds and ordinary hostile-contact pools. Sector 1 retains scout-only contacts. Captured beacons always use the existing Rebel interceptor patrol independently of sector identity. Distress escort fights retain their existing scout encounter.

An unresolved exit with two destinations requires an explicit choice. Continue cannot choose for the player. The penultimate exit and final confrontation keep their single-choice behavior. Invalid/repeated actions and departures with living crew away do not advance or award resources. Captured exits offer the same destination choices but no Federation cache.

## Authored content boundaries

These are authored demake sector pools over existing supported encounters, not imported original event lists or verified original generation probabilities. The sector names describe route identity; an Engi sector does not add an Engi enemy hull. Scout, interceptor, armored, assault and Rebel contacts remain the existing supported encounter implementations. No additional enemy/content ID is enabled.

| Identity | Main beacon emphasis | Ordinary enemy emphasis |
| --- | --- | --- |
| Civilian | Supplies and distress | Scouts and interceptors |
| Engi | Repair and supplies | Scouts and armored contacts |
| Rebel | Hostile contacts | Rebel and Rebel boarding contacts |
| Pirate | Hostiles and salvage/distress | Assault contacts with mixed hostiles |
| Last Stand | Hostiles with repair access | Supported hostile mix |

Exact relative beacon weights before fixed anchors overwrite their nodes:

| Identity | Fight | Supply | Distress | Store | Repair | Quest | Hazard |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Civilian | 1 | 4 | 3 | 1 | 2 | 1 | 1 |
| Engi | 1 | 3 | 1 | 1 | 4 | 2 | 1 |
| Rebel | 6 | 1 | 2 | 1 | 1 | 1 | 1 |
| Pirate | 4 | 2 | 3 | 1 | 1 | 1 | 2 |
| Last Stand | 5 | 1 | 1 | 1 | 2 | 1 | 1 |

Each ordinary fight after sector 1 chooses uniformly from six entries; repeated entries encode their authored weight:

- Civilian: Scout, Scout, Interceptor, Interceptor, Assault, Rebel.
- Engi: Scout, Scout, Armored, Armored, Interceptor, Assault.
- Rebel: Rebel, Rebel, Rebel Boarder, Rebel Boarder, Interceptor, Assault.
- Pirate: Assault, Assault, Interceptor, Armored, Scout, Rebel Boarder.
- Last Stand: Rebel, Rebel Boarder, Interceptor, Armored, Assault, Rebel.

Remaining original sector types, sector-specific original event lists, source-derived population rules and broader faction hull coverage are still incomplete. These four branch types are this milestone's supported roster, not the full-game target.

Hazard nodes still use the earlier authored solar-salvage choice. They are not active combat environments. Last Stand is a sector identity; moving flagship, Federation base defense and an authentic finale remain unfinished.

## Compatibility and validation

The route policy is explicit. Policy 0 retains the pre-V24 encounter selection and random-number sequence, with all route metadata zero. Existing save formats migrate to policy 0, including V23 eight-sector saves. New campaigns use policy 1. Loading an old map must never regenerate it. Persistence uses the new save appendix documented in the checkpoint.

`sector_routes_smoke` checks 256 complete synthetic eight-sector routing histories, pure query behavior, distinct offers, forced final destination, explicit selection, repeat/invalid/away-crew guards and cache suppression. It also samples 2,048 generated sector maps and fights for pool membership, weighted differences and preserved economy anchors. These are targeted synthetic tests, not earned campaign or hardware evidence. Integration runs, framebuffer inspection, save migration and physical acceptance are tracked separately in the checkpoint.
