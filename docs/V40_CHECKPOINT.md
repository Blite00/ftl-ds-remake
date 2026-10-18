# V40 calm-ship crew AI checkpoint

## Completed

Enemy crews now follow two more original CrewAI rules.

**Calm-ship healing.** FTL 1.6.14's `CrewAI::CheckForProblems` (0x628900) sets `bCalmShip`, then clears it for any fire (0x6289a2), hull breach (0x6289bc), intruder (0x628eb6) or damaged system (0x6290d4).
- A calm ship sends crew to the medbay below 99% health. Otherwise the 25% threshold applies (`CheckForHealing`, XFTL; Hyperspace's `CustomCrew.cpp` comment agrees).
- Calm is a state aboard the ship, not "out of combat". In the DS port, enemy crews only exist during encounters, so the rule takes effect during combat lulls. `game_enemy_calm` is exposed for tests and UI.
- Hurt crew still make the AI power the medbay first, so a calm patient can briefly cost a weapons bar. This matches ShipAI::CheckPowerLevels' `bHurtCrew` ordering.

**Manning tasks.** Stations are no longer fixed at spawn. They use `PrioritizeTask` manning values: piloting 64, shields 65, weapons 66, engines 67, sensors 69, doors 70.
- An unmanned console goes to the crewmember with the least important current task, if that task is less important than the console (`AttemptToAssign`, XFTL).
- A three-crew ship now staffs pilot, shields and weapons, not pilot, weapons and engines. That costs the V37 engines-manned evasion bonus.
- When the pilot dies, the weapons crewmember takes the helm.
- A patient's console is covered while they heal, and stations stay stable once filled.

Save layout is unchanged (save24). Existing saves and installed FTL files are untouched.

## Approximations

- The scheduler runs once per simulated second. Tasks are evaluated in priority order; the original task list is unsorted and built in CheckForProblems order.
- Hard-difficulty manning values (shields 67, engines 65) are not used.
- Oxygen, medbay and teleporter manning values (68/72/71) only rank crew for boarding (V39), since those consoles do not exist on these hulls.
- Automated-ship repair timing remains authored.

## Build and evidence

- Full suite passed: 126 host checks (new `enemy_calm_ai_smoke`) and 45 Python tests, including all twelve eight-sector campaigns (build/v40-check.log).
- Seven focused AI/crew/medbay checks also pass with strict warnings and undefined-behavior traps (build/v40-ai-focused.log).
- ROM: 40,931,328 bytes; SHA-256 `df481917ee70b11e0b285ad3d6c2e1ad2662854e05152440ab32b5235e3db4b4`. The build shows only the existing SDK executable-stack warning.
- All 17 audio files verified byte-exact. Static ARM9 union 917,292 bytes. Game state is 18,008 bytes and Campaign 22,156.
- Previews regenerated. The new `enemy-calm-heal-v40.png` is an unscripted Rebel Fighter whose 90% crewmember heads for the medbay. It is a host render, not an emulator capture.

## Next

V41+: authentic final-sector Federation base defense and the three-phase flagship. Physical DS/emulator acceptance remains pending.

Recovery: releases/v39/ftl-ds.nds, checkpoints/v39-boarding-ai.zip. Current ROM: releases/v40/ftl-ds.nds. Snapshot and hashes: checkpoints/v40-calm-crew.zip and build/v40-manifest.json.
