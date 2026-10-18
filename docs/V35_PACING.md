# V35 combat timing

## Identified cause

V34 advanced the 30 Hz simulation once every other main-loop iteration. Each iteration waits for VBlank, but synchronous software drawing and framebuffer copies can span additional display frames. Those frames were ignored by the simulation clock. Consequently slow rendering can slow weapon charging, movement and animations, rather than only reducing the displayed frame rate. This is a source-code finding; its contribution to a particular physical DS session has not been measured.

## Scheduler

`source/pacing.c` converts elapsed VBlank samples to the existing `GAME_HZ` of 30, using a nominal 60 Hz display clock, matching the previous intended ratio. Fractional steps carry across calls. A main-loop batch is limited to four updates; time beyond eight display frames is discarded so a long stall cannot cause seconds of catch-up. Reset at menu/load and frozen-time transitions discards elapsed time and fractional carry. Simulation rules, weapon cooldown values and save formats are unchanged.

`ds_input_vblank_count()` reads the existing aligned volatile IRQ sample counter. The IRQ still performs only its bounded input work; simulation remains on the main thread.

## Focused validation

`tests/pacing_smoke.c` passes strict C warnings and undefined-behavior trap instrumentation. Ten nominal seconds produce the same 300 game updates when polled every one, two, three or four display frames. Tests also cover fractional carry, repeated polling, stall cap, absence of later backlog, reset and unsigned counter wrap. The existing input-capture test passes the same instrumentation after exposing the sample count.

These are portable scheduler tests. They do not establish actual DS frame rate, input latency or audible pacing. Physical DS checks remain pending: weapon-charge duration, animation smoothness, pause/overlay resume without catch-up, and responsiveness while rendering a crowded fight.

## Integration and rendering

The main loop now samples elapsed VBlanks, advances UI/simulation/audio per returned step and presents once per batch. Menu/setup and scene changes reset timing; outcome or campaign-mode changes stop the batch. Only actual simulation-context changes discard the current batch; switching aiming, weapon or crew-page views keeps elapsed combat time.

Bitmap rendering now clips once per image and writes nontransparent pixels directly. Seven normal/target/destruction framebuffer previews remain byte-identical to V34; an independent clipping reference matches 144 edge/offscreen/transparent cases with buffer guards and undefined-behavior traps. This reduces repeated drawing work without establishing a measured hardware speedup.
