# V44 flagship final stage checkpoint

## Completed

Stage 3 of the flagship now has its original Zoltan super shield and laser power surge, completing the three-stage finale.

- **Super shield:** absorbs every non-friendly hit point for point before normal shields: lasers, missiles, ion, bombs and beams. Nothing reaches the hull, rooms or crew while it holds. Once it is gone, normal shields and hull take hits as usual. The HUD shows `SUPER n`.
- **Teleporter blocked:** while the super shield is up, your teleporter cannot send crew over or recall them.
- **Laser surge:** fires every 20–30 s, with the same 5 s `SURGE` warning as stage 2. Each surge fires 7 lasers at random rooms. Every 4th surge fires no lasers and restores the super shield to full.
- **Artillery level:** stages 1 and 2 cap artillery at level 3, matching observed original play. The blueprint's 4-bar artillery only fires at full strength in stage 3.

Sources: `BossShip::OnLoop`, which shows the stage-3 surge counter and the shield restore on the 4th surge, and `data/bosses.xml` (BOSS_3_NORMAL). The FTL wiki confirms the 12-point super shield on normal and 7 surge lasers.

## Difficulty decision: DS tuning by default

The authentic normal flagship is effectively unwinnable for DS builds. I kept the authentic mechanics and made a DS-tuned set of numbers the default. The authentic numbers stay one flag away (`Game.flagship_authentic`).

| | Authentic normal | DS tuning (default) |
|---|---|---|
| Shield bars, stages 1–3 | blueprint (up to 4 layers) | 4 bars (2 layers) |
| Artillery | level cap 3 in stages 1–2 | level cap 3, cooldown ×1.5 |
| Stage-3 engines | blueprint | 3 bars |
| Super shield | 12 | 8 |
| Stage-2 surge drones | 6 | 4 |

Seeded stage-by-stage wins out of 10 (`tools/flagship_balance_probe.c`, output in build/v44-balance.txt):

| Build | Authentic normal | DS tuning |
|---|---|---|
| Strong (reactor 18, 3 shield layers, Artemis + 2× Burst II + Heavy Laser, 6 crew) | 10 / 0 / 0 | 10 / 10 / 8 |
| Typical (V43 default kit) | 0 / 0 / 0 | 9 / 1 / 4 |

Even the easy blueprints from the original data could not make stages 2–3 winnable for the strong DS build, so switching to Easy alone does not fix this. The 18 scripted campaign fixtures now reach the flagship and sometimes win stages:
- Large campaigns: 10 engagements, 5 stage wins, 5 defeats.
- Surrender runs: 14 engagements, 2 stage wins, 12 defeats.

This trades original parity for winnability. V45 makes it a player choice: the main menu's FINALE option picks TUNED (default) or ORIGINAL for each new run; see [V45_CHECKPOINT.md](V45_CHECKPOINT.md).

## Build and evidence

- **Test suite:** 130 host checks (new `flagship_final_smoke`) and 45 Python tests passed (build/v44-check.log). Focused undefined-behavior traps passed (build/v44-focused.log).
- **ROM:** 40,990,720 bytes. SHA-256 `c0a646cfe4e1248314ee8f8f1403afdf087bba5a2a9c47ae18d17cc8b3bd4283`. The build shows only the existing SDK warning.
- **Audio and memory:** all 17 audio files verified. The static ARM9 union is 984,148 bytes.
- **Previews:** `flagship-super-shield-v44.png` and `flagship-laser-surge-v44.png` were inspected. These are host renders.
- **Saves:** the save format is unchanged (26). Existing saves and installed FTL files are untouched.

## Not yet verified

- Physical DS acceptance is pending.
- Surge laser spread is authored, at roughly 2–7 s.
- Mind control is AE-only and excluded.

## Next

V45: host pacing/balance and audio/memory measurements, plus a concise physical-DS test checklist.

Recovery: releases/v43/ftl-ds.nds. Current ROM: releases/v44/ftl-ds.nds. Snapshot and hashes: checkpoints/v44-flagship-final.zip and build/v44-manifest.json.
