# V32 surrender, escape and readable sector map

## Delivered

New runs use event policy 5. The complete original generic REBEL encounter is available in eligible optional fight pools from sector 2. Its ten introductions, 23 weighted surrender text entries, two source hulls, two destruction alternatives and all nine crew-defeat reward alternatives remain intact. Unsupported quest roots and equipment stay excluded.

Surrender chance and threshold are sampled once per encounter. An offer holds simulation for explicit touch or button acceptance/refusal. Acceptance exchanges boarding parties safely, ceases fire and commits its supplies once; no destruction reward follows. Refusal resumes the same enemy, queued attacks and escape progress. Terminal combat outcomes take priority over offers. When both triggers become eligible in the same tick, surrender precedes a new escape warning; refusal can then show that warning.

Escape chance is independent, also sampled once. The original generic Rebel data omits a timer, so this DS version uses 40 seconds of operational engine/pilot time. Disruption stops charging. The absent gotaway callback grants no salvage or invented pursuit penalty; the existing warning scout retains its explicit fleet consequence.

The bottom touch screen defaults to a zoomed current-sector view; the top shows the entire sector and outlines the viewport. Both axes use irregular, repeatable beacon positions. Minimum overview spacing is 26 pixels. L/R selects and moves the camera; Up or ALL/ZOOM toggles the bottom view; B or CENTER returns to the ship. Touch uses the same coordinates and never jumps until confirmed. Red rings indicate actual capture and gold dotted rings indicate next-turn capture. The visual layout changes no campaign links, RNG, travel costs or fleet timing. See V32_MAP.md.

## Source boundaries and compatibility

Source XML establishes probabilities, threshold bounds, text, branch weights and eligible hulls. Reward quantities, default escape time, sampling/priority and automatic negotiated crew exchange are documented DS interpretations, not verified original executable behavior. Offer hints show possible resource ranges, clamped to storage capacity; actual reward rolls commit on acceptance. See V32_EVENT_SOURCE.md and the prior surrender-core checkpoint.

Safe map checkpoints now write format 24 with the same payload layout as 23. Historical saves retain their original policy and eligible roots; loading a V31 save does not silently add surrender encounters. Start a new run for policy 5. Encounter thresholds, warnings, queues and camera state are transient. Pending negotiations cannot be saved.

## Validation

All 112 host checks, 42 event compiler tests and three enemy importer tests pass. Final suite evidence is in build/v32-check.log and build/v32-evidence.json. Focused surrender tests cover 256 seeded chance combinations, resource caps, once-only rewards, boarding-exchange preflight, rejection, overlapping warnings and death priority. Map tests cover 64 generated plus 64 legacy fixtures, deterministic irregular layouts, minimum spacing, both zoom modes, every beacon selection, actual threat colors, pure rendering and framebuffer guards. Core surrender, save policy and map checks also pass undefined-behavior trap builds.

Twelve ordinary-action policy 5 campaigns complete 783 jumps and 783 disk coldloads. The driver uses three seeds, both sector-route choices and accept/refuse policies. It deliberately reduces firing against weakened crewed ships to allow negotiations. There are two earned offers: one accepted and one refused. Across 21 original battles it records 16 hull wins, four escapes and one accepted surrender, covering all five descriptors and four supported hull types. No resources, health, charge, damage, geometry or victories are injected in these pilots. Separate synthetic boundary/UI fixtures are labeled explicitly.

An exploratory expansion to seeds 1..12 found crew loss for seed 5 on branch 1. That failed pilot trace is preserved in build/v32-surrender-expanded-pilot-failure.txt. The selected regression matrix is not a universal win-rate or original-difficulty calibration, and direct crew-defeat branches use fixtures rather than earned kills in this matrix.

Existing regression assertions were updated for the current writer version and default policy. Prior policy 4 campaign pilots retain their original encounter/result expectations explicitly. Historical migration fixtures remain; malformed modern map, route and intruder files use format 24 so rejection checks their corrupt fields rather than merely an incompatible policy/header.

Actual historical-writer compatibility was tested from the verified V31 checkpoint: its archived writer creates format 23; the V32 reader preserves nondefault resources, skills, cargo and policy 4, refuses the new root 11, then writes 24. Both files are 3,724 bytes; the payload and retained backup match byte for byte. See build/v32-release-migration.json.

Actual host-rendered frames were inspected for starting/middle/exit/overview/pursuit maps, surrender selection and acceptance, plus generic Rebel escape warning/charge. These are framebuffer previews, not emulator screenshots. ROM build succeeds with the existing SDK executable-stack metadata warning.

ARM9 static union: 868,012 bytes; high-water: 868,040. Game 17,876 / Campaign 22,024 bytes. Runtime stack, heap, frame pacing, audio, input and flashcart storage remain unmeasured on this build.

## Release and recovery

ROM: G:/demake/ftl-ds.nds. Preserved copy: releases/v32/ftl-ds.nds. Size 585,728 bytes. SHA-256: `c8959536b47117b0961188bb3c72ab10b96972c62b616e0dfb12fb9e0d93006b`.

Recovery: checkpoints/v32-surrender-map.zip, accompanying SHA-256 sidecar and build/v32-manifest.json. V31 release/checkpoint remain intact. Installed FTL and existing user saves were preserved; original extracted data stays local.

## Next steps

1. Human DS playtest of bottom-screen zoom, beacon selection, capture warnings, negotiation choices, louder V30 audio and controls. Verify repeated save/resume on the flashcart. Report these pending until actually performed.
2. Broader economy/difficulty testing across routes, ships and player strategies. Investigate the retained crew-loss pilot and excessively safe weapon-suppression cases before tuning. Increase earned generic Rebel surrender/escape/boarding coverage.
3. Finish remaining button-only equipment, upgrades, naming and confirmations; add more supported original hazards, equipment and enemies.
4. Continue authentic moving Flagship/base defense and progression work from FULL_GAME_CHECKLIST.md. The complete base game remains unfinished.
