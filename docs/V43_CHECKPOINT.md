# V43 flagship stage-2 drones checkpoint

## Completed

Stage 2 of the flagship now runs its original drone system: room 12, 8 bars and 10 drone parts. The drones follow the BOSS_2_NORMAL droneList order: Defense I, Combat I, Anti-Ship Beam I and the boss Boarding Drone.

- **Power and parts:** each drone needs two effective bars and they are powered in blueprint order, so damaging or ionizing the drone system disables drones from the end of the list. Deploying a drone spends a part.
- **Defense drone:** shoots down your missiles, including Artemis, once a second.
- **Combat and beam drones:** fire a 1-damage laser or a short 20 px beam. Both use the port's authored 10 s interval; the original's orbit-driven timing is not modeled.
- **Boss boarding drone:** flies for 3 s, breaches a random room and fights as a 150-HP intruder. Rock stats stand in for the drone; it uses the V39 intruder targeting.
  - As in the original, it stays tied to its drone slot.
  - Losing drone power or destroying the stage shuts it down. Killing it frees the slot for a new drone, at the cost of a part.
  - Your Defense I can shoot it down in flight.
- **Power surge:** every 20–30 s, with a "SURGE nS" warning 5 s ahead, six extra drones (the beam/combat split is fixed per encounter) fire two shots each. They spend no parts and ignore drone-system damage.

Sources: `data/bosses.xml` and `data/blueprints.xml` (drone and DRONE_LASER/DRONE_BEAM blueprints); `BossShip::OnLoop` (stage-2 surge to PrepareSuperDrones); the FTL wiki (6 surge drones on normal, 20–30 s interval, 5 s warning, two shots each). See also [V42_FLAGSHIP.md](V42_FLAGSHIP.md).

Approximations:
- Drone movement and orbits aren't simulated.
- Firing intervals are authored.
- The boarding drone uses Rock stats (fire immune and suffocates, unlike the original's suffocation-immune drone).
- Surge shots are spread over about 2–7 s.

No save change (all of this is combat-transient). Existing saves and installed FTL files are untouched.

## Build and evidence

- Full suite passed: 129 host checks (new `flagship_drones_smoke`) and 45 Python tests (build/v43-check.log). The drone test passes again with undefined-behavior traps (build/v43-focused.log).
- Difficulty is unchanged from V42: a strong build wins stage 1 in 16/20 fights; the 18 scripted fixtures still lose at stage 1.
- ROM: 40,989,696 bytes; SHA-256 `eed9b8fb90463e5c9489ab3f63af683de6bb14d17a1f57509ae9ec1b765786dc`. Only the existing SDK warning.
- All 17 audio files verified. Static ARM9 union 982,700 bytes. Game state is 19,436 bytes.
- The preview `flagship-drones-surge-v43.png` (stage-2 drones, an intruder drone after a breach, the surge countdown) was inspected. It is a host render.

## Next

V44: stage-3 Zoltan super shield (12 HP), the 7-shot laser surge (every 4th surge restores the shield) and full-finale acceptance. Physical DS acceptance is pending.

Recovery: releases/v42/ftl-ds.nds, checkpoints/v42-flagship.zip. Current ROM: releases/v43/ftl-ds.nds. Snapshot and hashes: checkpoints/v43-flagship-drones.zip and build/v43-manifest.json.
