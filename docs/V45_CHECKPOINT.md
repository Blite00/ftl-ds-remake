# V45 final build: finale difficulty choice, host measurements and DS checklist

## Completed

Blite chose a menu option for the flagship difficulty question raised in V44.

- **Main menu:** a new **FINALE** row (between New Run and Controls) toggles the setting for the next new run:
  - **TUNED** is the default: the V44 DS balance.
  - **ORIGINAL** uses the FTL 1.6.14 normal flagship numbers.
- **Feedback:** the hint line shows which setting new runs will get. Touch and Up/Down + A both work, and the setting survives reopening the menu during the session.
- **Applies at launch:** the choice takes effect when you launch from crew setup. It does nothing to a run that is already in progress or saved.
- **Per-run persistence:** every flagship stage uses the run's own choice. The choice is applied when each encounter is staged.
- **Save format 27:** appends one word (0 or 1), and values outside that range are rejected. Saves in formats 2–26 load as TUNED, so existing runs, including runs already in the final sector, keep the V44 behavior. Existing save files are only rewritten when you save explicitly. Installed FTL files are untouched.
- **Not stored:** the menu preference itself, like Sound, is not written to storage, so it starts as TUNED at boot.

What each setting changes is in [V44_CHECKPOINT.md](V44_CHECKPOINT.md): shield layers, artillery cooldown, stage-3 engines, super shield points and surge drones. The mechanics are identical.

## Host measurements and DS checklist

This is the final build of this work. Further fidelity work is skipped by request.

- [HARDWARE.md](HARDWARE.md) (V45 section) has the static memory and stack figures and the host cost probe. In brief:
  - About 1.65 MB of the 4 MiB main RAM is used statically or by resident effects.
  - The worst known stack path is 3.5 KB of the 16 KiB DTCM, and the save/FAT path is unknown.
  - A worst main-loop iteration costs about 0.6 ms on this PC. The DS could be roughly 55 times slower before missing the 33 ms update budget, and about 150 times slower before audio underruns, so frame pacing is likely but unproven.
  - A save to flashcart SD is the one expected music-gap risk.
- [DS_TEST_CHECKLIST.md](DS_TEST_CHECKLIST.md) is the short physical test pass: 18 items covering boot and storage, input, timing, audio, the finale and stability.
- Reproduce with `python tools/stack_report.py` and `tools/host_cost_probe.c`; the build line is in the probe's header. Results are in build/v45-stack.txt and build/v45-host-cost.txt.

## Build and evidence

- **Test suite:** 131 host checks passed, including the new `finale_choice_smoke`. `main_menu_smoke`, 22 save tests and 6 campaign tests were updated for the five-item layout and the 4-byte save tail. 45 Python tests passed (build/v45-check.log).
- **`finale_choice_smoke`:** covers:
  - the menu toggle and its persistence across reinitialization;
  - campaign flagship fights picking up each setting (ORIGINAL has more shield bars);
  - the save27 round trip, rejection of out-of-range values, and save26 migration to TUNED.
- **ROM:** 40,990,720 bytes. SHA-256 `42dcbc33aa9ea6a7ed81be0a7df6003f92a0f226e804c96f04925c6c8974bb47`. The build shows only the existing SDK warning.
- **Memory:** static ARM9 union 984,356 bytes. Game is 19,456 bytes and Campaign 23,748 bytes.
- **Audio:** all 17 audio files verified byte-exact in NitroFS.
- **Previews:** `main-menu-continue-v45.png` and `main-menu-finale-original-v45.png` were inspected. These are host renders.
- **Tools:** `tools/flagship_balance_probe.c` reproduces the V44 balance table.

## Not yet verified

- Physical DS acceptance.
- The map does not show the run's finale setting.

## Next

Run the physical DS test pass in [DS_TEST_CHECKLIST.md](DS_TEST_CHECKLIST.md).

Recovery: releases/v44/ftl-ds.nds. Current ROM: releases/v45/ftl-ds.nds. Snapshot and hashes: checkpoints/v45-final.zip and build/v45-manifest.json (the earlier checkpoints/v45-finale-choice.zip has the same ROM).
