# V64 tutorial

## Completed
- **Tutorial from the main menu.** The menu now has six entries in the original order: Continue, New Run, Tutorial, Finale, Controls, Sound. Spacing is 23 px, and the Controls and Sound touch rows keep their old positions.
- **26 guided Kestrel steps** use the original tutorial text (`data/text_tutorial.xml`, imported by `tools/import_tutorial.py` into `generated/tutorial_text.h`). DS adaptations are listed in the importer: taps and buttons instead of mouse and iPad gestures, DS screen locations, and text trimmed to fit.
  - **Topics:** ship and systems intro, power (engines), system damage, selecting and sending crew, repair and repower, manning, fire and airlocks, the helm, jumping and the star map, pause, shields, weapon power, targeting a laser volley (blocked by shields), the Artemis missile hitting the enemy shields, enemy systems, and destroying the scout.
  - **Progress:** steps advance when the action is actually done, and read-only steps use NEXT.
  - **Prompts on both screens:** an instruction panel on the top screen with the step counter, topic, text and live goal line; the real game controls on the bottom screen, with a blinking marker on the expected control.
  - **Controls:** buttons and touch both work.
  - **Skipping:** SELECT or the SKIP button leaves the tutorial at any step.
- **Separate from the campaign.** The tutorial has its own Game and state and never calls save or load. Verified in melonDS: the campaign save on the SD card is byte-identical after a tutorial session.
- **New files:** `source/tutorial.{h,c}`, `source/tutorial_view.inc`, `tests/tutorial_smoke.c` (walks all 26 steps and the skip paths, and checks no save is written), `tools/import_tutorial.py`. The module was drafted by a helper agent and integrated here.
- **Not covered:** the original inventory and upgrade lessons (Halberd swap, upgrade screen) and tooltip hovering are left out.

## Evidence
- **Results:** 130 quick host checks passed, including `tutorial_smoke`. The finale menu test was updated for the new menu order.
- **melonDS 1.1** (DLDI, SD sync): Tutorial opens from the menu; steps 1–2 shown; SKIP returns to the menu; the campaign save is unchanged.
- **ROM:** 43,721,728 bytes. SHA-256 `da98ebaea9a9466e131daf881365a48034993dbdc9de258f7637bc587bfe2408`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,230,300 bytes, including the tutorial's own Game.

Recovery: releases/v63/ftl-ds.nds. Current ROM: releases/v64/ftl-ds.nds.
