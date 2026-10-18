# V25 checkpoint: compiled original event choices

October 3, 2026. Software integration verified. Full base-game content, human balance, observed emulator input/audio/storage and physical DS acceptance remain unfinished.

## Implemented

- Six complete original event closures are compiled from the owner's local archive: refuel/repair stations, missile/drone-part sales, fuel-for-drone trades and the damaged refugee fuel encounter. All source choices and nested/random alternatives are retained. The catalog contains 41 nodes and 30 choices.
- Runtime handles fixed and ranged resource changes, hull repair, four-choice decisions, random event/text alternatives and living aboard-ship Engi conditions. Resource costs and worst-case ranges are shown before confirmation. Costs/rewards commit once; affordability queries consume no RNG. Outcome text shows actual changes.
- Optional supply/repair beacons can draw reviewed economy roots; initial fuel and pre-exit repair anchors remain authored services. Imported unique flags prevent campaign-wide repeats, including after loading. Revisits and captured-beacon patrols take precedence over ordinary event content.
- WAIT while out of fuel opens the refugee contact when the fleet has not captured the beacon. Its original branches include purchase, Engi negotiation, refusal and an attack/threat that leads directly to surrender. There is no fabricated combat branch. This automatic placement is authored DS policy.
- Up to four wrapped choices fit the lower screen with focus, blue conditions, unavailable-state feedback and cost hints. Narrative paging uses L/R or touch. Current imported roots fit one page; separate synthetic fixtures verify complete three-page text. ASCII punctuation glyphs preserve source wording.
- Map-only save18 appends event policy and the encountered-root mask (eight bytes). Formats 2-17 migrate with original event policy disabled and existing map/route state preserved. Start a new run to receive these original events. Active encounters remain outside the save contract.
- The compiler rejects unsupported effects/attributes/references, malformed ranges, cycles, excessive depth/nodes and unrenderable labels. If any branch is unsupported, its entire root stays disabled. A local register accounts for 351 named definitions across twelve base-source containers.

## Evidence

All **73 registered C host checks and 17 Python compiler tests passed** (build/v25-checks.txt). Focused runtime checks cover exact trades, repair clamping, bounded/random reproducibility, atomic rejection, pure queries, living/aboard Engi requirements, nested refusal/negotiation, WAIT/fleet priority, unique/revisit guards and legacy policy. Real imported purchases preserve uniqueness through a disk cold load; a following random exchange reproduces its text and outcomes.

Six earned eight-sector campaigns used the existing seed/branch combinations: branch0 seeds1/42/8675309 made 68/65/64 jumps; branch1 made 65/66/65. They completed **393 jumps and 393 disk cold loads**, 41 imported decisions and two real resource exchanges, encountering all five ordinary economy roots (mask0x1f). The fuel-only refugee root is covered by focused fixtures, not these earned campaigns. All six ended at 30 hull. Human balance and other tactical builds remain pending.

The old pilot initially lost a finale after accumulating insufficient equipment. The revised pilot buys weapons/capacity/reactor incrementally, sells replaced cargo and reallocates engine power when necessary. No game balance, seeds, encounter placement, resources, damage or victories were altered to obtain these passes. See build/v25-campaign-final.txt.

Actual archived V24 writer migration passed: 3708 to 3716 bytes, the complete original payload prefix and backup byte-identical. Route history/offers, map, stock, RNG, resources and Defense I ownership remain unchanged. Old/backup SHA-256: `e2e19b8e85041090d53fcff5ac403fc9906574a5bcb6dc32ee04ac3cd47fa3c7`. Fixture funding is compatibility evidence only. See build/v25-release-migration.json.

Inspected actual renderer previews include original-event-four, original-event-blue, original-event-disabled, original-event-repair, original-event-repair-outcome, original-event-trade-range and event-synthetic-page1/2/3, all with -v25.png suffix. Final choice spacing was reduced to eight pixels per row to leave margins around three-line buttons. Host previews are distinct from emulator screenshots or physical input acceptance.

ROM build passed with the existing SDK executable-stack metadata warning. Static ARM9 union 724836 bytes / high-water offset 724864; Game 13460 / Campaign 17492 bytes. Narrative storage is now bounded at 2048 bytes; runtime events allocate no heap. Stack high-water, runtime memory/frame pacing and physical input/audio/SD remain unmeasured.

Release: `releases/v25/ftl-ds-v25.nds`, 513024 bytes, SHA-256 `307ebaef8a6c7058356889ad522f6e699dc98c3e4564a77a87488c5016fb67eb`.
Recoverable archive: `checkpoints/v25-original-events.zip` with per-file manifest and hash sidecar. V24 baseline hash was verified before editing; old checkpoints and installed FTL/user saves are preserved.

## Imported facts and DS adaptations

[V25_EVENT_DESIGN.md](V25_EVENT_DESIGN.md) records source IDs/hashes and exact compiler semantics. Original text, choice structure, resource ranges, repair values and Engi condition are imported. Source file names alone do not prove historical pre-AE eligibility of all definitions; only six reviewed roots are enabled.

Uniform range/list sampling, disabled-visible choices, worst-case debit checks, campaign-wide unique suppression, authored beacon placement and automatic out-of-fuel contact are explicit adaptations. The source describes MED fuel rewards as scrap plus fuel but does not provide numeric engine distributions; this one reward is authored as 3-6 fuel plus 15-25 scrap. It is not verified original reward math.

## Exact next steps

1. Add supported original combat-event closures with correct result-dependent outcomes, then system/equipment conditions, quest targets and persistent consequences. A six-root compiler foundation does not complete original event coverage.
2. Implement active hazards and their pause, sensor, power and save interactions. Extend sector pools and reviewed sector types with those dependencies.
3. Finish remaining drone roles/lifecycle, cloaking, augments, artillery/special shields, overflow/shared inventory and original ship coverage.
4. Complete Federation-base/flagship finale, hangar/unlocks/difficulty/history, animations/tutorial/audio/lid handling and release validation.

No original quest, system/equipment blue-option root, active environmental hazard or authentic flagship mechanic is declared complete by this checkpoint. Full objective remains active.
