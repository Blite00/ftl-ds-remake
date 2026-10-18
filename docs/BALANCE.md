# V31 later-sector loadout evidence

The V30 opening below is retained. V31 introduces supported multi-weapon profiles from sector 2 for policy4 generated routes. In controlled 32-seed fixtures, passive Basic+Heavy fights caused mean 7.31 hull loss and 10 defeats; Basic+enemy Artemis caused 17.94 hull loss and 29 defeats. Prompt Weapons targeting won all 32 for each pair, with mean hull losses 0.72 and 1.75. These are action-policy fixtures, not human win rates or original difficulty calibration.

Three ordinary-action campaign pilots completed eight sectors (195 jumps), encountering nine new profiles including two missile pairs. Final hull30 includes purchased repairs. They consumed 88 player missiles and one enemy missile; effective Weapons suppression often stopped the missile slot. The limited exposure does not establish balanced difficulty across seeds. Detailed methods, source boundaries and results: [ENEMY_LOADOUTS.md](ENEMY_LOADOUTS.md).

# Early campaign balance probe

## Finding and change

The original opening was safe even without player input. The Scout has one laser shot every 10 seconds, no active shields, and 6 hull. The starting Kestrel has one shield bubble that recharges in 2 seconds. A lone shot cannot get through that bubble. Every authored first-sector fight previously used this Scout, so changing target rooms rarely affected survival.

The guaranteed first fight at node 1 remains this gentle introduction. Later authored first-sector fights in generated campaigns with event policy 4 now choose equally between the existing Interceptor and Armored Drone profiles:

| Profile | First-sector hull | Shield bubbles | Weapon volley | Base cooldown |
| --- | ---: | ---: | --- | ---: |
| Tutorial Scout | 6 | 0 | One 1-damage shot | 10 seconds |
| Interceptor | 9 | 0 | Two 1-damage shots | 11 seconds |
| Armored Drone | 12 | 1 | Two 1-damage shots | 13 seconds |

Two-shot volleys can penetrate the starting bubble. Weapon targeting can interrupt that offense; shield targeting on the Armored Drone can speed up its destruction. This is an intentional DS encounter mix, **not a verified reconstruction of original FTL difficulty**. Imported ship data, weapon damage, repair timing, starting resources and reactor allocation were not changed.

## Measurements

Reproduce with `tests/balance_probe.c`. CSV output is `build/balance_probe.csv`. Strict C99 compilation with warnings treated as errors passed. The probe runs 2,688 isolated encounters: six policies across 128 real first-contact seeds, six policies across 128 later-opening pool seeds, and six policies across 32 seeds for each of six sector-two profiles.

- **First contact:** actual fresh `campaign_init_seed` and `campaign_jump(1)`.
- **Later-opening pool:** an explicit topology fixture connects an existing node 4 and marks it as a fight, then calls the real campaign jump and encounter selector. This isolates the new pool with the untouched starting Kestrel. It does not represent a naturally earned route to that node.
- **Sector-two profiles:** explicit encounter-selection fixtures with the untouched starting Kestrel, without purchases or upgrades. These are comparative matchups, not claims about their frequency on a route.
- Seeds are 1 through 128 or 1 through 32. Each run ends on victory/defeat or after 120 simulated seconds.
- Ready-fire policies pause to issue valid orders whenever weapons are ready. Delayed policies start checking after 8 seconds and then only every 5 seconds. No charge, damage, healing, ammunition or victories are injected.
- Weapons retain their real independent cooldowns. Neither policy synchronizes volleys or repairs/heals/moves crew. A missing shield system falls back to the weapon room; empty-room targeting excludes installed main systems, piloting and drone control where possible.
- No-input trials are stress comparisons, not an expected way to win.

### Opening results (128 seeds per row)

| Encounter / policy | Wins | Defeats | Timeouts | Mean hull lost | Mean missiles | Mean seconds |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Tutorial, ready weapons | 128 | 0 | 0 | 0.00 | 2.00 | 21.78 |
| Tutorial, delayed empty room | 128 | 0 | 0 | 0.00 | 2.00 | 30.45 |
| Tutorial, no input | 0 | 0 | 128 | 0.00 | 0.00 | 120.00 |
| Later mix, ready weapons | 128 | 0 | 0 | 0.39 | 3.11 | 34.61 |
| Later mix, ready shields | 128 | 0 | 0 | 1.05 | 2.92 | 32.17 |
| Later mix, ready empty room | 127 | 1 | 0 | 1.91 | 3.32 | 36.53 |
| Later mix, delayed weapons | 126 | 2 | 0 | 0.92 | 3.41 | 50.21 |
| Later mix, delayed empty room | 120 | 8 | 0 | 3.09 | 3.54 | 51.88 |
| Later mix, no input | 0 | 82 | 46 | 7.20 | 0.00 | 97.95 |

All measured defeats in the later mix were crew losses. Hull loss alone understates the danger from unattended fires, oxygen damage and crew injuries. These fixed pilots do not respond to those hazards, so their win rates are **not measured human win rates**. The useful result is that target choice and delays now change the outcome, while the first encounter remains forgiving.

### Other exposed weaknesses

In sector-two fixtures, Scout, Auto-Assault and Rebel Fighter still caused zero hull damage in all 32 no-input trials over 120 seconds. Their supported offense is a single laser or a single slow combat drone. The existing shield model blocks it. This is a content/loadout limitation to address as varied original enemy equipment is implemented, not evidence that shields should be weakened globally.

Rebel Boarder trials lost one crew member and stalled across all policies. These pilots make no crew orders, and are therefore unsuitable for judging boarding balance. The separate boarding and ordinary-action campaign pilots cover active defense; a future balance probe should include a simple visible-threat response policy.

## Compatibility

Only `sector_enemy` changes. Legacy maps and event policies 0–3 retain their Scout opening pool and consume no extra campaign RNG draw. Policy-4 later first-sector fights consume one additional bounded RNG draw to choose between the two profiles. A resumed policy-4 save therefore remains valid but its future encounters and later random outcomes may differ from the previous build. No save format or stored state layout changes.

The probe asserts both new profiles occur, old policies keep the old pool without extra RNG consumption, and legacy maps remain unchanged.

## Validation limits and next work

The existing six complete eight-sector drivers still pass, including their boss, resource and coldload assertions. They deliberately favor stores and avoid fights, and use immediate weapon actions and extensive repair/healing routines. Their full-hull finishes demonstrate campaign viability and persistence, **not human balance acceptance**. Three complete retreat runs and the three legacy campaigns also pass. Map and historical original-event tests pass.

Physical DS feedback prompted this change; this probe is host simulation. Physical validation of the revised encounter mix remains pending. Next checks should record whether a player notices shield absorption, identifies a damaged system, responds to fire/boarding and finishes a later opening fight without instructions. Further difficulty adjustments should follow those observations and additional supported loadouts rather than changing imported damage values speculatively.

## Reproduction

From the project root in PowerShell:

```powershell
$env:PATH = "$PWD/.tools/msys64/ucrt64/bin;$env:PATH"
$probeSources = @('campaign','rewards','outcome','game','fire','boarding','breach','subsystems','species','beam','drones') | ForEach-Object { "source/$_.c" }
& .tools/msys64/ucrt64/bin/gcc.exe -std=c99 -Wall -Wextra -Werror -Isource -Igenerated tests/balance_probe.c @probeSources -o build/balance_probe.exe
& ./build/balance_probe.exe | Set-Content build/balance_probe.csv
```
