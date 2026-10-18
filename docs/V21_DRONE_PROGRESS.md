# Historical V21 foundation record

This staged record is superseded by V21_CHECKPOINT.md: acquisition, inventory, UI, format14 and earned campaign gates now pass. Its original evidence below remains scoped to the earlier development snapshot.

# V21 drone foundation — work in progress

The latest playable release remains V20. Current development source adds player Combat I simulation and save format 13, but has no player acquisition or drone controls yet. It must not be described as a released drone milestone. The full base-game objective remains active.

## Implemented and verified

- Bounded control and schematics: two Kestrel slots within a three-slot storage bound; imported Kestrel drone-control room.
- Manual reactor allocation, displayed slot priority, one part per deployment, free suspension/resumption within an encounter and reset on successful departure. Refused departure retains deployed state.
- Combat I uses imported power/damage and an authored ten-second firing period, random room targeting and five crew HP per hull damage unit. Projectile-pool saturation retries without losing charge or rerolling targets.
- Control participates in projectile/ion/fire damage, hostile sabotage, crew repairs and Zoltan bonus power. Actual ion/bomb, service repair, fire and Zoltan interactions are covered by the strengthened drone smoke check.
- Format 13 appends 48 bytes: parts, installed, room, capacity, health, power, ion bars/timer, slot count and three schematic IDs. Format 2–12 loads default to no control and zero parts. Active deployment, enabled state, charge and target reset under the map-only save policy.
- Source register enumerates 14 drone definitions with raw dependent weapon/control data. Only the existing enemy Combat I dependency is in campaign pools. No additional definition is enabled merely by appearing in the archive.

## Evidence

All 55 registered strict host checks passed (build/v21-foundation-checks.txt), including previous earned equipment/weapon/boarding/species campaigns and the three-run 108-cold-load regression. The strengthened final drone test separately passed (build/v21-foundation-drone-check.txt). These do not yet prove an earned player-drone campaign.

ROM build and renderer preview generation passed. Existing equipment framebuffer inspected; no drone UI exists yet. Preview filenames still use V20 because release UI branding has not changed. Latest static ARM9 union: 687652 bytes; high-water offset: 687680; Game 13460; Campaign 14516. Runtime stack/heap, frame rate, emulator input/audio/DLDI and physical DS checks remain pending.

V20 archive SHA-256 was verified before work: 5447a0d72d6eb967044ade4a1d2e18f4be5938541d7ec4d64a030fdb7517b12c. Original archive preservation and deterministic drone extraction evidence are in DRONE_COVERAGE.md.

## Exact next steps

1. Add campaign acquisition, control installation/upgrades, parts supply and bounded drone inventory with store stock and conservation checks. Review source costs/package contents; label unverified mapping choices.
2. Add a readable drone management/combat panel with touch and controller navigation, control power, deployment/status/charge and error feedback. Show control room damage and readable drone attack presentation.
3. Extend save format for inventory/stock as needed while preserving format 13. Add exact-item removal/sales and full-inventory handling.
4. Complete an earned drone campaign with cold loads and inspect drone framebuffer previews before enabling the supported player Combat I pool and publishing V21.
5. Continue defense/repair/boarding drones, cloaking, augments and ship-dependent systems; eight sectors/fleet/events/hangar/finale and hardware acceptance remain required.

Known limitations: fixed-period procedural attacks do not reproduce moving drone orbits or vulnerabilities. Enemy drones still use the older separate controller. Generic equip/remove APIs assume an ownership-enforcing campaign caller; they are not exposed as combat inventory edits. Drone rewards and excess loot remain unfinished.

Actual archived V20 writer migration passed: format 12 (2488 bytes) to 13 (2536 bytes), preserving equipment order, disabled identity, cargo, resources and stock; old saves default to absent drones and zero parts. Original/backup SHA-256: 6bd3058888d29b8ca7556cdedc65a03bdd5d324bfe956377765f0c0b59312917. See build/v21-release-migration.json. Compatibility fixture funds are not earned-run evidence.

Development checkpoint: checkpoints/v21-drone-foundation.zip with per-file manifest and SHA-256 sidecar. No V21 release ROM is published yet.
