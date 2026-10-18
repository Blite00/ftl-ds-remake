# V38 enemy weapon AI checkpoint

## Completed

Enemy weapons now use the original FTL normal-difficulty targeting. These rules were recovered from the installed 1.6.14 executable and cross-checked against XFTL and Hyperspace notes:

- Each shot has a 33% chance to pick from shields, engines, weapons, oxygen and drones when the player has that system. Otherwise it hits a uniform random room.
- Every shot in a volley goes to a different room.
- Each weapon fires on its own as soon as it is charged; nothing synchronizes volleys.
- Beams aim from one chosen room centre toward a second and keep their full length.

Previously every shot hit a system room. See [V38_WEAPON_AI.md](V38_WEAPON_AI.md) for addresses, verified rules and the remaining DS approximations.

Save layout is unchanged (save24). Volley plans are transient combat state. Existing saves, backups and installed FTL files are untouched.

## Build and evidence

- Full suite passed: 124 host checks (new `enemy_weapon_ai_smoke`) and 45 Python tests, including all twelve eight-sector scripted campaigns with 783 cold loads (build/v38-check.log).
- Focused AI/loadout/boarding checks also pass with strict warnings and undefined-behavior traps (build/v38-ai-focused.log).
- ROM: 40,928,256 bytes; SHA-256 `81334b5ef48bb8be08534517c6604794e8713a83ecc28df14b338102699086db`. The build shows only the existing SDK executable-stack warning (build/v38-build.log).
- All 17 embedded audio files verified byte-exact (build/v38-rom-audio.log).
- Static ARM9 union 914,092 bytes (+1,808); high-water 914,120. Game state is 17,996 bytes and Campaign 22,144 (+120 each, for the per-weapon volley plans).
- Host previews regenerated. `enemy-volley-v38.png` and `enemy-beam-v38.png` were added. They are controlled scenarios drawn by the real renderer, not emulator captures.

## Decisions and test change

- Normal difficulty only. Easy (20%) and hard (priority list) are documented but not exposed; the port has no difficulty selector yet.
- The scripted campaign pilot now handles fires and breaches in any room as a group with medbay retreats. Before, it repaired only system rooms; seed 42 lost its crew to spreading fire in corridors once those rooms could be hit. These are ordinary player orders. Game rules were not relaxed.
- Balance changes: fewer shots land on systems, and more land in empty rooms (hull damage still applies, plus fire and breach spread). Difficulty still needs a human or DS playtest.

## Limitations and next steps

Enemy reactor/weapon power order, the manned charge bonus, burst spacing, enemy drone targeting and automated repair remain authored. Full parity is not claimed, and physical DS/emulator acceptance is pending. Next: V39 boarding landing room, recall timing and crew specialization.

Recovery: releases/v37/ftl-ds.nds and checkpoints/v37-combat-ai.zip. Current ROM: releases/v38/ftl-ds.nds. Snapshot and hashes: checkpoints/v38-weapon-ai.zip and build/v38-manifest.json (made by the new `tools/release.py`).
