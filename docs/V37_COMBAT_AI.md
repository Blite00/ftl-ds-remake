# V37 combat AI fidelity pass

## Reference and scope

Primary reference: [XFTL crew-AI reverse-engineering notes](https://gitlab.com/znixian/xftl/-/blob/master/doc/crew-ai), reviewed 2026-10-04. This is reverse-engineering evidence, not an official specification or verification against the running PC game.

The reference describes healing below 25% health during combat, retaining patients until 99%, shield and critical oxygen repair priorities, room danger checks and separate defense/repair tasks. It also describes boarders preferring reachable useful rooms, penalizing fire and low oxygen, and retaining equally good orders. V37 adopts those broad rules for the supported DS systems and species.

## Changes and DS choices

- Species-relative medical thresholds replace fixed human-health values. Patients reserve available beds; inaccessible or dangerous medbays are excluded.
- Defense no longer overwrites every home crewmember's healing or repair order. Room task assignments are bounded by room size and demand.
- Shields and critically low oxygen receive urgent repairs; other work uses system priorities. Teleporter gathering is subordinate to emergency work.
- Reactor allocation includes drones, teleporters and racial bonus power. Power loss immediately caps shield bubbles. The allocation order is authored for this port.
- Boarding launch supports parties capped at two even with a larger roster. Home orders remain under one scheduler. Thresholds use species maximum health.
- Boarders can move from a destroyed system to another useful room; hazards and distance affect selection. Equal targets retain their previous destination. Initial teleport target remains weapons; recall threshold/cooldown and room-level movement remain DS choices.

This remains a bounded, once-per-second scheduler rather than a copy of FTL's complete task assignment system. No new save fields or imported assets are needed. Original music from V36 remains included.

## Remaining fidelity work

1. Recover and validate normal-difficulty projectile target selection, volley targeting and beam placement. Current enemy weapon selection/aiming remains authored; this pass does not claim weapon-AI parity.
2. Compare boarding recall timing, landing-room choice and crew specialization against controlled original-game encounters.
3. Match calm-ship healing and station assignment more fully; current scheduler operates during combat.
4. Broaden supported systems/equipment only after their behavior and event requirements are implemented. Unsupported content remains excluded.
5. Replace the interim Rebel Command finale with the authentic flagship/base-defense campaign flow.
6. Validate balance, responsiveness and V36 streamed audio on physical DS hardware. Automated tests and host framebuffers do not establish real-hardware acceptance.
