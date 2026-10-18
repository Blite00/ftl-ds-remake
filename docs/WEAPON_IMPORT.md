# Selected original weapon import

`tools/import_enemy_catalog.py` now generates `weapon_catalog.h` and `weapon_catalog.json` alongside `weapon_properties.h`. The bounded selection has stable indices: 0 Artemis (`MISSILES_2_PLAYER`), 1 Burst II (`LASER_BURST_3`), 2 Basic Laser (`LASER_BURST_1`), 3 Heavy I (`LASER_HEAVY_1`), 4 Ion Blast (`ION_1`). Existing indices do not move.

## Ion Blast source facts

Original blueprint `ION_1` has localized title Ion Blast, projectile type LASER, hull damage 0, ion 1, one shot, power 1, cooldown 8 seconds, shield piercing 0, speed 30, cost 30, fire/breach chance 0. System/personnel overrides are absent. Installed-source stun chance 1 is retained in the JSON audit as excluded post-AE data; it does not enable stun.

Original prose describes system disabling without hull damage and ion affecting shields. These descriptions do not establish numeric lockout duration, stacking/recovery order, power allocation rules or exact shield interception. Runtime implementation must identify its selected policies separately; imported blueprint values alone do not verify engine behavior.

## Data contract and boundaries

`ImportedWeapon` exposes source ID/name/type, cooldown milliseconds, power, hull damage, shots, ammunition, shield piercing, ion, cost, speed and raw fire/breach chance. `ImportedWeaponProperties` separately carries optional override presence flags. Missing source speed is encoded as zero in the C table, meaning unspecified engine default, not a zero-speed projectile; JSON records `has_speed` explicitly.

Costs are original blueprint values, not an instruction to change the existing campaign economy. In particular original Burst II/Basic/Heavy cost 80/20/50 while earlier demake catalog values were 65/28/48. A runtime price change needs a deliberate documented policy. The four earlier combat loadouts retain their stable identities.

The archive remains read-only; extracted content and generated headers stay local. Deterministic regeneration and stable ordering checks passed. `weapon_catalog.json` records the source blueprint SHA-256. This is source-data evidence, not a completed ion gameplay or ROM validation claim.

## V18 Halberd Beam selection

Catalog index 5 adds original BEAM_2, localized Halberd Beam, preserving indices 0–4. Exact source fields: type BEAM, damage 2, cooldown 17 seconds, power 3, cost 65, beam length 80, shield piercing 0 and fire/breach chances 0. Speed, shots, missile use and system/personnel overrides are absent. The header's zero for absent shots/speed must not be treated as a projectile with no shots or speed: the beam-specific runtime owns sweep behavior. `ImportedWeapon.beam_length` is the final appended field; non-beam entries have zero.

Source data supplies beam length and damage but does not by itself establish room-intersection geometry, sweep duration, shield attenuation, hit ordering or whether two touched cells in one room count once. Runtime geometry conversion and interaction choices must be documented/tested separately. This selection emphasizes shield attenuation with a two-damage beam; unsupported beam families remain outside the pool.

Deterministic regeneration and index/length checks passed. No existing source weapon IDs moved; original source files remained read-only. Gameplay/full-campaign/ROM evidence is separate from this data audit.

## V19 Small Bomb selection

Index 6 adds original BOMB_1, localized Small Bomb, without changing indices 0–5. Source type is BOMB; hull damage 0, explicit sysDamage 2, explicit persDamage 2, one shot, one missile per activation, power 1, cooldown 13 seconds, cost 45, shield piercing 0, raw fire chance 1 and breach chance 0. Both override-presence flags are true. No stun field is present.

Original descriptions identify a self-teleporting weapon that damages systems and crew without hull damage and permits targeting the player's own ship. This supports a distinct bomb path rather than pretending shield-piercing projectiles are equivalent. Evasion, teleport/impact timing and personnel-unit conversion still require documented runtime policies. The source value persDamage 2 is not itself a statement of two crew HP; neither the XML nor supplied descriptions establish its HP unit. Do not silently conflate it with the existing authored five-HP-per-hull-point rule.

The existing ImportedWeapon.type string identifies bombs; no extra header flag is added. Regeneration, stable index and explicit override-presence checks passed. Actual bomb effects, own-ship targeting, ammunition accounting and campaign persistence require separate gameplay validation.
