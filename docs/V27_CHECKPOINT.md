# V27 checkpoint - warning scouts and enemy escape

Date: October 3, 2026. Full base-game goal remains active.

## Implemented

- Complete original AUTO_WARNING closure with nine introductions, exact two-ship pool, escape warning, departure/pursuit and destruction callbacks.
- Explicit Engage and frozen warning with Continue. Bounded FTL charge stops for pause, disabled engines/piloting and ions.
- Separate escape outcome with no hull explosion or salvage; pursuit advances once. Destruction retains LOW-standard salvage.
- Crew left aboard are lost once. Home boarders and hazards remain active. Death outcomes take precedence over departure.
- Remote shots after departure reject without spending ammunition or charge. Home-target bombs and autofire continue through the shared combat simulation.
- Save20 preserves the existing payload layout and old content policies, adds the ninth-root policy/mask and clears transient escape state on load.

## Evidence

- tools/check.ps1: 80 C checks and 28 Python compiler tests passed. An outdated save-version assertion was corrected; the full suite was rerun after the final combat fix. Final UI wording was checked separately afterward.
- Focused core tests cover threshold/pause/engine/pilot/ion behavior, actual last-tick damage, death priority, lost identities, boarder melee and home-bomb impacts/autofire after departure.
- Event tests cover all nine introductions, eight full 40-second escapes with starting resources and live enemy weapons, frozen warning, once-only pursuit, actual beacon capture and separately labeled forced edge cases.
- Six earned eight-sector campaigns: 389 jumps and 389 disk cold loads, 70 original decisions, two trades and 11 original battles (10 hull wins, one escape). All three event ship descriptors and four enemy types covered. The second-branch pilot withholds fire against warning scouts to exercise pursuit; no resources or victories are injected. Crew-only event wins remain focused-fixture coverage.
- Actual archived V26 writer migration: 3716 to 3716 bytes, identical payload and exact backup; old policy2 and seen bits129 retained. See build/v27-release-migration.json.
- Actual renderer frames inspected: introduction, warning, charging, stopped charge, gotaway, departure, stale aim/boarding, home-bomb targeting and fleet map. Removed stale enemy art/targeting and clarified departed-target instructions.
- ROM build passed. The existing SDK executable-stack metadata warning persists.
- Static ARM9 union 747300 bytes; high-water offset 747328. Game 13480 / Campaign 17520 bytes. These are not runtime memory or performance measurements.

## Release identity

ROM: releases/v27/ftl-ds-v27.nds, 535552 bytes.
SHA-256: `b5524bd23b4ed102e7fc48202881489819b75270853bcee0ee47d64be35c1495`.
Archive: checkpoints/v27-enemy-escape.zip with a verified manifest and SHA-256 sidecar.

## Fidelity and limits

Source text, enemy pools, raw escape attributes and pursuit amount are imported. Hull-trigger/countdown interpretation, fixed loadouts, reward math and clearing in-flight attacks at departure are documented DS approximations. Unsupported warning effects, chance/load forms and negative pursuit effects fail closed. Original extracted content remains local; installed FTL and user saves remain untouched.

Physical DS, emulator input/audio/DLDI, runtime stack/heap/frame pacing, human usability/balance and interrupted-SD recovery acceptance remain pending. Scripted campaigns and static sizes do not establish these gates.

## Exact next steps

1. Player FTL charging/retreat: destinations, fuel/fleet costs, engine/pilot disruption, hostile boarders and explicit away-crew handling.
2. Surrender, original conditions and quests with complete closures and once-only consequences.
3. Beacon-owned hazards persisting through battles, management and saves; shared reactor/visibility/pursuit rules.
4. Remaining equipment, augments, drone roles, playable hulls, authentic Federation base/flagship, hangar progression and DS release gates in FULL_GAME_CHECKLIST.md.
