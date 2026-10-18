# V21 checkpoint: player Combat I drones

October 3, 2026. Software integration verified. The full base-game goal remains active; emulator and physical DS acceptance remain pending.

## Player behavior

- Stores append control package for 85 scrap (Combat I plus two parts), schematic for 50 scrap and three parts for 24 scrap. Stock persists across revisits and resets in new sectors.
- Drone control starts at two capacity, upgrades to eight, shares reactor allocation and participates in damage, ion, fire, sabotage, crew repairs and Zoltan power.
- Two installed Kestrel slots within a three-slot bound. Four separate drone cargo slots support fitting/swapping, removal, order and exact sales with confirmation. Full holds and invalid actions leave ownership/resources unchanged.
- Deployment spends one part. Suspension, ion or power interruption retain the deployment and charge; resumption is free. Successful travel/sector transition resets launch state, including safe beacons; refused travel preserves it. New encounters require new parts.
- START or touch DRONES opens the panel. D-pad/A navigates; L/R changes power, Y switches cargo, X pauses, B returns. Combat continues unless paused. Inventory actions are blocked/dimmed during battle. Store Up/Down+A purchases are now supported.
- Control room label/damage border, deployed marker, parts/power/health/ion status, charge, sale details and actionable failure feedback fit both DS framebuffers.
- Save format 14 appends drone cargo and three offers after the format 13 control fields. Formats 2–13 migrate; backup recovery remains. Saves remain map-only; deployment/charge reset on load.

## Evidence

All 59 registered strict host checks passed in build/v21-checks.txt. The final drone UI test was rerun after cosmetic changes (build/v21-ui-final-check.txt). Existing weapon, species, boarding, full campaign and 108-cold-load regressions passed. Store blank-row tests were updated to the actual final offer page after adding drone offers.

Earned drone campaign: ordinary package, parts and spare purchases; spare fitting/removal; five paid deployments; isolated actual Combat I shield impact with guns held until resolution; six-sector, three-phase victory and 36 disk cold loads. Final hull 23 / scrap 47. Parts, stock, schematics and reset launch state are checked. No free resources, damage or victories are injected. This is current-subset evidence, not eight-sector completion.

Actual archived format 13 writer migrated installed Combat I, seven parts and campaign state into format 14; new cargo defaults empty and new stock initializes. Original/backup SHA-256: 7cdb72727c27b083cce57da726d05359804657568ceb6a5897997f6b3bfe70bc. See build/v21-format14-migration.json. Earlier actual V20 format 12→13 evidence remains in build/v21-release-migration.json.

Inspected final actual renderer previews: drone-active-v21.png, drone-management-v21.png, drone-sale-v21.png and drone-no-parts-v21.png. Strict preview compilation and ROM build passed. Static ARM9 union 697412 / high-water 697440 bytes; Game 13460 / Campaign 14676. Runtime memory/frame time remain unmeasured. Existing toolchain executable-stack metadata warning persists.

Release: releases/v21/ftl-ds-v21.nds, 491520 bytes, SHA-256 96e71710d143074ea367b17d6a71f2817ba469ee164488af57cfdd6de91feb24.
Recoverable checkpoint: checkpoints/v21-player-drones.zip with manifest and SHA-256 sidecar. V20 and V21 foundation archives preserved; foundation hash verified before integration.

## Source and approximation boundaries

Imported Combat I power and attack data are reused. Fixed ten-second fire, random room selection, five crew HP per damage unit, package/parts prices and upgrade cost 20 + 10 × capacity are authored policies. Control raw source cost 60 / startPower 2 / maxPower 8 and the raw upgrade sequence are registered without claiming recovered original package/level semantics.

Drone movement, vulnerability and destruction are not yet simulated. Current marker/projectile presentation is procedural. Enemy drones retain their older separate controller. Separate four-slot drone cargo adds capacity beyond a shared original cargo hold; that limitation remains explicit. Only Combat I is selected for player and enemy; 13 other raw definitions remain disabled and historical eligibility is not inferred from file placement. DRONE_COVERAGE.md records provenance.

## Exact next work

1. Add defense drone interception and proper drone attack/movement/lifecycle interactions with focused missile tests; then repair/interior/boarding drone roles and shared player/enemy control.
2. Reconcile shared inventory capacity and add pending equipment reward keep/swap/discard handling before salvage/event equipment rewards.
3. Implement cloaking, augments, artillery and special shields; enable matching base-game playable ships only after their mechanics pass.
4. Complete eight-sector larger maps, visible advancing fleet, sector choices, original supported events/hazards, hangar/unlocks and final base defense/authentic flagship.
5. Finish animations/tutorial/audio/lid and emulator/physical DS acceptance. Continue independent implementation while hardware is unavailable.
