# V39 boarding AI checkpoint

## Completed

Enemy boarding now follows the original rules, recovered from FTL 1.6.14 and the XFTL notes:

- Sabotage ships send half their crew, rounded down, and keep the rest home.
- After two completed sends they stop teleporting.
- Boarders land in a uniformly random room instead of always the weapons room.
- Healthy fighters who are least busy board first. The pilot goes last, and Engi or Zoltan crew who are manning a station stay home.
- Sabotage boarders retreat below 25% health. All boarders are recalled when the enemy escapes with less than 15 s left, when 3 or more of its systems are broken, or when home crew fall below the stay-behind count.
- Intruders follow the first boarder's room choice unless another room is strictly better.

See [V39_BOARDING_AI.md](V39_BOARDING_AI.md).

Behavior change: a three-crew Rebel boarder now sends one crewmember, where V37 sent two. Larger crews send at most two at a time.

Save layout is unchanged (save24). The new boarding state is encounter-local, and existing saves and installed FTL files are untouched.

## Build and evidence

- Full suite passed: 125 host checks (new `enemy_boarding_v39_smoke`) and 45 Python tests, including all twelve eight-sector campaigns (build/v39-check.log).
- Five focused AI/boarding checks also pass with strict warnings and undefined-behavior traps (build/v39-ai-focused.log).
- ROM: 40,930,304 bytes; SHA-256 `b7dec3f653c47b6f3e7a3bee5f5cfdc490148165c39840961f9611fd778706f9`. The build shows only the existing SDK executable-stack warning.
- All 17 audio files verified byte-exact. Static ARM9 union 915,980 bytes. Game state is 18,008 bytes and Campaign 22,156.
- Previews regenerated. The new `enemy-boarders-v39.png` shows an unscripted encounter in which one boarder landed off the weapons room. It is a host render, not an emulator capture.

## Decisions

- Invasion mode (3/4 of the crew) is wired for `ENEMY_FLAGSHIP` but unused until the authentic flagship lands (V41+).
- The DS keeps its two-person teleporter pad, so larger requests take several sends.

## Next

V40: calm-ship (out-of-combat) healing and station behavior. Physical DS/emulator acceptance remains pending.

Recovery: releases/v38/ftl-ds.nds, checkpoints/v38-weapon-ai.zip. Current ROM: releases/v39/ftl-ds.nds. Snapshot and hashes: checkpoints/v39-boarding-ai.zip and build/v39-manifest.json.
