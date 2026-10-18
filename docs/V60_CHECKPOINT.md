# V60 beacon hazards (event batch 2)

The first of the remaining event batches: the original environment hazards. Events that put a beacon in an asteroid field, near a sun, in a nebula or in an ion storm now compile, and fights there take place in the hazard.

## Completed
- **Event compiler.** The `<environment type>` tag is supported for asteroid, sun, nebula and storm. The anti-ship battery (PDS) is Advanced Edition (its `PDS_SHOT` is only in the AE blueprints), so those events stay excluded. Enabled original events rose from 59 to 79: for example the nebula, Slug-nebula and ion-storm events, and the pirate, auto-ship and Rock fights near suns and in asteroid fields.
- **Hazard beacons.** The map's hazard beacons now draw from each sector's original hazard events (13 for nebula sectors, 6 for civilian, 3 each for Engi and pirate, 1 for Rebel), instead of the single authored solar-flare event. That authored event remains the fallback.
- **Hazards in combat.** A fight at a hazard beacon takes place in that hazard. The top screen shows a banner between the enemy panel and the ships.
  - **Asteroid field:** an asteroid hits a random ship every 2–7 s: 1 damage, stopped by shields, and Defense Drone II can shoot it down.
  - **Sun:** a solar flare every 25–35 s (first after 20 s, countdown shown, red in the last 5 s) starts 1–2 fires on each ship.
  - **Nebula:** sensors cannot see into the enemy ship, even manned at level 3 (crew aboard and Slug telepathy still work). Your own ship stays visible.
  - **Ion storm:** a nebula that also halves both reactors. Power comes off the least vital systems first, and power limits use the halved reactor until you jump. Shown as `P4/4`.
- **Reset on jump.** A beacon's hazard clears on every jump. It is not saved; saving at a beacon after its fight has no lasting effect.
- **Test bot.** Spare healthy crew now put out fires, which a flare can start in several rooms.

## Not included
- Pulsar and PDS are Advanced Edition.
- The slower rebel fleet in nebulas is not modelled.

## Evidence
- **Results:** 141 host checks (new: `v60_environment_smoke`) and 45 Python tests passed. The new test covers:
  - every hazard type in the compiled catalog and the hazard pools;
  - asteroids;
  - the first flare and its fires;
  - nebula sensor blindness;
  - the storm halving both reactors and the power limits;
  - an event setting the beacon's hazard, which clears on a jump;
  - hazard beacons drawing hazard events.

  The two older tests that assumed at most 4 event choices now allow the 6 the event screen supports.
- **ROM:** 43,557,888 bytes. SHA-256 `a41da8942e6190f99696dd7b65611c69fde57baeac086c915cb3cd8664353e9f`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,045,124 bytes.
- **Previews inspected:** `v60-asteroid`, `v60-sun`, `v60-nebula` and `v60-storm`.

Recovery: releases/v59/ftl-ds.nds. Current ROM: releases/v60/ftl-ds.nds. Snapshot: checkpoints/v60-hazards.zip and build/v60-manifest.json.
