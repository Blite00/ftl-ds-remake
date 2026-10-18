# V22 checkpoint: Defense I

October 3, 2026. Software integration verified. The complete base-game goal remains active; emulator and physical DS acceptance remain pending.

## Behavior

Defense I catalog ID 1 uses imported name, cost 50 and power 2. Store offer 14 appends to page four, places the schematic in drone cargo and preserves existing offer IDs. Installation, power, parts, exact sales and loadout priority use the existing drone interfaces.

A deployed, powered Defense I recharges for one second, then intercepts one pending hostile missile, choosing shortest remaining flight time and pool index as tie-breaker. Interception runs before normal impacts and has a distinct result, so it cannot also deal hull/system/crew damage or consume shields. Multiple ready drones choose different missiles. Interception needs no spare projectile slot. Pause, power/ion interruption, damage, suspension and terminal outcomes gate operation. Lasers, ions, beams, bombs, friendly projectiles and already-resolved impacts are excluded.

Drone UI shows Defense I, readiness/recharge and a cyan interception burst between ships without a hull-hit flash. Existing touch/controller controls remain. Save format 15 appends one offer's stock per beacon and accepts Defense ID 1; older formats remain restricted to their prior catalog and receive new stock defaults.

## Evidence

All 63 registered strict host checks passed (build/v22-checks.txt). The final strengthened defense test passed separately (build/v22-defense-final-check.txt), covering salvos, multiple defenders, saturated pools and suspended defense. Save tests verify exact purchased ownership/stock, invalid stock rejection, rejection of Defense IDs in older formats and byte-exact backups.

Earned Defense campaign: ordinary package, Defense I and parts purchases; three deployments; one actual enemy missile observed changing to INTERCEPT; six-sector/three-phase victory and 36 disk cold loads. Final hull 26, scrap 53. Player fire was withheld briefly against a missile-capable opponent to observe interception; no funds, attacks, damage or victories were injected. Existing Combat I, weapon, boarding/species and 108-cold-load campaign regressions also passed. This remains evidence for the current six-sector subset.

Inspected actual renderer previews: defense-ready-v22.png, defense-intercept-v22.png and defense-store-v22.png. Strict preview compilation and ROM build passed. Static ARM9 union 698612 / high-water 698640 bytes; Game 13460 / Campaign 14724. Existing executable-stack metadata warning persists. Runtime stack/heap/frame time are unmeasured.

Release: releases/v22/ftl-ds-v22.nds, 492544 bytes; SHA-256 1587db7b395f1f7e218a7cac9c50a35d4f890a0ae055485888128b2b62793fe1.
Recoverable archive: checkpoints/v22-defense-drone.zip with manifest and SHA-256 sidecar. V21 archive was verified before work and remains preserved.

## Fidelity and remaining work

The imported raw cooldown is 1000; its unit is not established. One-second recharge, instantaneous interception, earliest-arrival targeting and fixed drone position are explicit demake approximations. No orbital aiming, turning, blind spots, drone vulnerability or enemy defense controller is claimed. Asteroids/boarding pods do not yet exist as interceptable projectiles. Defense II and other unimplemented definitions stay outside all pools.

Next: implement remaining drone roles and lifecycle/movement interactions, then shared inventory and excess rewards, cloaking/augments/artillery/special shields and dependent ships. Larger eight-sector maps, advancing fleet, supported original events/hazards, hangar/unlocks and authentic final-sector/flagship remain required. Continue independent work while emulator/physical DS acceptance is pending.

Actual archived V21 writer migration passed, format14 to15, preserving installed Combat I, cargo, five parts, resources and spent stock. New Defense stock initializes correctly. Original/backup SHA-256: bc1ff08aae99cfa8e37664997f4739d3e593c2452030f53c4fb141438b4ebe67. Evidence: build/v22-release-migration.json. Fixture funds were used solely for compatibility setup.
