# V42 authentic Rebel flagship (stage hulls, artillery, cloaking, crew)

## Sources

- `ftl.dat` (local, read-only):
  - `data/bosses.xml` BOSS_1/2/3_NORMAL, using layouts `boss_N_easy`, as the base game does on normal.
  - ARTILLERY_BOSS_1..4 from `data/blueprints.xml`.
  - Hull art from `img/ships_glow/boss_N_base.png`.
  - The new `tools/import_flagship.py` (run by `build.ps1 -FtlArchive`) writes `generated/flagship_data.h` and `flagship_art.h`.
- FTL 1.6.14 `FTLGame.exe`, read-only disassembly:
  - `BossShip::StartStage` (0x406980): stage-1 crew seeding; per-room crew carried to later stages, with rooms past the new hull dropped; stage 3 powers the super shield.
  - `BossShip::OnLoop` (0x407d40): `crewCounts` refreshed every live frame; surge timer; stage-2 drone surge and stage-3 barrage, with every 4th surge going to the super shield.
  - `ArtillerySystem::OnLoop` (0x634f20): cooldown multiplier `1.5 - 0.25 × effective power`; aiming via `GetRandomTargettingPoint`.
- Cross-checks: the FTL wiki "The Rebel Flagship" and "Cloaking" pages (fetched 2026-10-05).
- Not compared with controlled play sessions.

## Implemented (verified rules)

| Rule | DS |
|---|---|
| Stage hulls 20/22/20; reactor 42/44/31; shields 8 bars (4 layers); stage layouts with 19/15/11 rooms and imported doors | `flagship_setup` from the imported tables, plus the imported hull art |
| Artillery rooms: laser (3×1, 20 s), missile (3×1, 5 pierce, 23 s, no ammo), beam (2 damage, 100 px, 26 s), ion (3 × 1 ion, 28 s). Charge rate scales with effective power: ×0.5 at 4 bars, ×0.75 at 3 | `Artillery` system; damage, ion, crew repair, boarder sabotage and AI repair all apply |
| Artillery aims at random points: distinct uniform rooms; beams from a random room in a random direction | 16 discrete beam directions |
| Stage 1 cloaking (2 bars): +60% evasion, 5 s per bar, then 4 ion bars (20 s lockout). The player's weapons stop charging and cannot target a cloaked ship; drones hold fire. The AI cloaks whenever possible and randomly chooses whether to fire while cloaked | `EnemyCloak`; artillery honours the random fire-while-cloaked choice |
| Stage 1 crew: 11 humans, one in each of rooms 0–6, 9, 10, 14 and 18. The weapon-room gunners are isolated: they repair but can't man | Enemy encounter slots raised from 8 to 12 (`GAME_MAX_ENEMY_CREW`). Carried-intruder disk slots are unchanged (mapped on save/load) |
| Later stages keep survivors per room. Rooms past the smaller hull lose their crew (the ion gunner after stage 1, the beam gunner after stage 2). Stage 1 always reseeds | `Campaign.boss_crew`, refreshed every live tick, as in OnLoop |
| Killing the whole crew does not win: the AI takes control, systems count as manned, and damaged systems repair over time except in rooms with fire or a breach | Automated mode with the existing 12 s/bar repair rate (authored timing), extended to artillery, cloaking, doors and piloting |
| Stage 3 has the teleporter (invasion boarding) | V39 boarding AI in invasion mode (3/4 of the crew) |

Doors (3 bars) are an enemy subsystem. Stages 1 and 2 earn a stage reward. The original is a HIGH scrap reward at sector-1 value; the DS gives an authored 30–40 scrap.

## Stage 2 and 3 specials

Stage-2 drones and the drone surge arrived in V43; the stage-3 super shield and laser surge arrived in V44 ([V43_CHECKPOINT.md](V43_CHECKPOINT.md), [V44_CHECKPOINT.md](V44_CHECKPOINT.md)). Mind control is AE-only and excluded.

## DS approximations

- Room-level targeting: one target per room, not per slot.
- Beam directions are quantized to 16.
- Burst spacing is 0.2 s.
- AI repair rate is the authored 12 s per bar.
- Stage scrap is approximated.
- The sensors cap at level 2 is not modeled.
- Legacy six-sector runs and save-24 runs already in the final sector keep the interim authored platforms (`interim_flagship`).

## Difficulty

V42 shipped authentic normal numbers. With all three stages complete (V44), the authentic flagship proved unwinnable for DS builds: the strong build wins 10/0/0 stages out of 10, and the original Easy blueprints do not fix it. V44 therefore makes a DS tuning the default: 4 shield bars, artillery cooldown ×1.5, stage-3 engines 3, super shield 8 and 4 surge drones. Authentic numbers remain behind `Game.flagship_authentic`. See [V44_CHECKPOINT.md](V44_CHECKPOINT.md) for the measurements. Since V45 the player picks TUNED or ORIGINAL per run from the main menu ([V45_CHECKPOINT.md](V45_CHECKPOINT.md)).

## Save format 26

Appends `boss_crew_valid` and 19 per-room counts (80 bytes). Saves 2–25 migrate. Counts are only valid in the final sector.
