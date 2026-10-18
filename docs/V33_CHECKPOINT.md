# V33 original enemy hulls, main menu and touch response

## Delivered

The rectangular Interceptor, Armored Drone and interim Rebel Command use original Auto-Scout, Auto-Assault and Rebel Rigger hulls respectively. Source rooms, doors, miniature impacts, beam positions and touch targets match the artwork. Their authored strengths and automated behavior remain. Geometry and connectivity can affect boarding and beam coverage; the Command ship remains an authored finale using a Rigger hull, not the authentic Flagship. See V33_SHIP_ART.md.

The ROM boots into a main menu with Continue, New Run, Controls and Sound. It validates primary/backup saves without starting simulation. A missing/invalid checkpoint disables Continue. New Run confirmation defaults to Cancel and does not write or delete saves. Map Start opens Resume Run; B resumes; combat Start retains Actions. Results return to the main menu. The menu holds gameplay and is silent. Touch and buttons both work; main options use larger text. See V33_MAIN_MENU.md.

VBlank input sampling preserves distinct quick contacts that happen while the main loop renders. The bounded eight-entry queue retains each contact's coordinates; holds never repeat. New views, events and confirmation dialogs discard old-view presses, while ordinary selection, power and feedback changes do not add a cooldown. Actions-page/cargo-tab/target-space and zoom-camera changes are guarded. Events older than 30 samples expire instead of triggering late. Input receives immediate presentation without changing the simulation tick policy.

Automatic diagnostic SD writes are deferred during gameplay; explicit Save, diagnostics access and entering the menu flush the bounded log. Recent log entries can be lost on a crash and older unflushed entries can be dropped. Campaign persistence is unchanged. This removes a known blocking I/O path and addresses missed-edge polling; it does not prove measured real-DS latency. See INPUT_RESPONSE.md.

## Evidence

All 116 host checks and 45 Python tests pass (42 event compiler + 3 enemy importer), recorded in build/v33-check.log. The main-menu test was rerun after the final text-size change: build/v33-menu-final.log. No production changes followed except documentation.

New focused checks cover all eight enemy kinds and all Command phases, exact source rooms/doors, retained authored strengths, visible original art, every room touch target and framebuffer guards. Menu tests cover disabled Continue, corrupt-primary/valid-backup recovery, unavailable storage, cancel/confirm, no implicit save replacement, stable repeated frames and result-menu requests. Input tests cover rapid contacts during delayed consumption, held suppression, queue overflow, stale expiry, wraparound, screen barriers and unchanged-view responsiveness. Menu, hull and input checks also passed undefined-behavior trap builds. Logging tests verify deferred polling and explicit writes while retaining the old host default.

The full suite retains 12 earned policy5 eight-sector campaigns with 783 jumps and 783 cold loads, accepted/refused surrender and escape coverage. Prior enemy-loadout and older-policy campaign drivers also pass. These are scripted policies, not human difficulty calibration. One room-damage fixture now chooses an actual room without a system rather than a removed hardcoded room; terminal UI fixtures now verify a menu request and unchanged ended campaign rather than implicit restart.

Actual renderer previews were inspected for the three replaced hulls in battle/aiming and main-menu states (Continue/New Run/confirmation/controls). Repeated menu top frames are byte-identical across states; an image-view omission of repeated regions was checked against the pixels. These are host framebuffers, not emulator screenshots.

The installed SDK disassembly was checked before moving sampling into VBlank: scanKeys and touchRead use bounded cached data/register operations without storage or hardware waits. Queue sharing is volatile-qualified and consumed under short critical sections. No renderer, filesystem or game logic runs inside that interrupt. No previous project VBlank handler was replaced.

ROM build succeeds with the existing SDK executable-stack metadata warning. Static ARM9 union 898,660 bytes; high-water 898,696. Game 17,876 / Campaign 22,024 bytes. Runtime stack/heap, frame pacing and hardware input/audio/storage acceptance remain pending.

## Release and recovery

Current ROM: G:/demake/ftl-ds.nds. Preserved release: releases/v33/ftl-ds.nds. Size 615,424 bytes. SHA-256: `a41185303e9c4c21c27b711296b8d1ef55a114026a27aa92f4478697388ac0eb`.

Recovery: checkpoints/v33-menu-ships-input.zip, SHA-256 sidecar and build/v33-manifest.json. V32 release/checkpoint remain intact. Installed FTL and existing user saves were preserved; original extracted assets remain local. Save format 24 and historical encounter-policy rules are unchanged.

## Next steps

1. On real DS, compare quick taps and successive power/crew orders, try held touch, cross menu/confirmation boundaries and test Continue/New Run/save/backup recovery. Check speaker volume and native-screen readability too. These are pending until observed.
2. Record real frame-time and SD timings if response remains slow. Very short contacts between two input samples can still be missed, and slow rendering can still delay visible response.
3. Continue controller-only management, broader balance/content coverage and the authentic Flagship/base-defense finale from FULL_GAME_CHECKLIST.md. The complete base-game objective remains unfinished.
